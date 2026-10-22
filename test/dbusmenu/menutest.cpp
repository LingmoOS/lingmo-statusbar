// SPDX-FileCopyrightText: 2026 Lingmo OS Team
//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Headless check of DbusMenuClient against test/dbusmenu/fake_menu.py.
//
// The test needs no window, no DTK and no display: it talks to a fake
// com.canonical.dbusmenu service on the session bus and prints what the client
// parsed, so run.sh can compare it with the tree the fake exports.
//
// Build: see the command line in test/dbusmenu/run.sh.

#include "appmenu/dbusmenuclient.h"

#include <QCoreApplication>
#include <QDBusConnection>
#include <QEventLoop>
#include <QTextStream>
#include <QTimer>

namespace {

constexpr auto FakeService = "org.lingmo.TestMenu";
constexpr auto FakePath = "/MenuBar";
constexpr int TimeoutMs = 5000;

QTextStream &out()
{
    static QTextStream stream(stdout);
    return stream;
}

void dumpEntries(const QString &prefix, const QVariantList &entries)
{
    for (const QVariant &entry : entries) {
        const QVariantMap map = entry.toMap();
        out() << prefix << map.value(QStringLiteral("id")).toInt() << '|'
              << map.value(QStringLiteral("text")).toString() << '|'
              << (map.value(QStringLiteral("separator")).toBool() ? QStringLiteral("separator")
                                                                  : QStringLiteral("-")) << '|'
              << (map.value(QStringLiteral("hasSubmenu")).toBool() ? QStringLiteral("submenu")
                                                                   : QStringLiteral("-"))
              << '\n';
    }
    out().flush();
}

// Waits until the client has a non-empty top level or the timeout expires.
bool waitForMenu(DbusMenuClient &client)
{
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    timer.setInterval(TimeoutMs);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    QObject::connect(&client, &DbusMenuClient::changed, &loop, [&loop, &client]() {
        if (!client.topLevelItems().isEmpty()) {
            loop.quit();
        }
    });
    timer.start();
    loop.exec();
    return !client.topLevelItems().isEmpty();
}

} // namespace

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    if (!QDBusConnection::sessionBus().isConnected()) {
        out() << "NO_SESSION_BUS\n";
        return 2;
    }

    DbusMenuClient client;
    bool signalled = false;
    QObject::connect(&client, &DbusMenuClient::changed, &client, [&signalled]() {
        signalled = true;
    });

    out() << "SERVICE " << FakeService << '\n';
    client.setMenu(QString::fromLatin1(FakeService), QString::fromLatin1(FakePath));

    if (!waitForMenu(client)) {
        out() << "TIMEOUT waiting for the layout\n";
        return 1;
    }

    out() << "AVAILABLE " << (client.available() ? "yes" : "no") << '\n';
    dumpEntries(QStringLiteral("TOP "), client.topLevelItems());
    dumpEntries(QStringLiteral("CHILD-OF-1 "), client.childrenOf(1));

    // "New" (id 10) must reach the fake service as Event(10, "clicked").
    client.activate(10);

    QEventLoop drain;
    QTimer::singleShot(300, &drain, &QEventLoop::quit);
    drain.exec();

    client.clear();
    out() << "CLEARED available=" << (client.available() ? "yes" : "no")
          << " top=" << client.topLevelItems().size() << '\n';
    out() << "DONE changed-signal=" << (signalled ? "yes" : "no") << '\n';
    out().flush();
    return 0;
}
