// SPDX-FileCopyrightText: 2026 Lingmo OS Team
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "kwinscriptloader.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QFileInfo>
#include <QStandardPaths>
#include <QStringList>

namespace {

constexpr auto KWinService = "org.kde.KWin";
constexpr auto KWinScriptingPath = "/Scripting";
constexpr auto KWinScriptingInterface = "org.kde.kwin.Scripting";
constexpr auto KWinScriptInterface = "org.kde.kwin.Script";
constexpr auto TrackerPluginName = "lingmo-statusbar-activewindow";
constexpr int DBusTimeoutMs = 5000;

QString callFailure(const QDBusMessage &reply)
{
    const QString name = reply.errorName();
    const QString message = reply.errorMessage();
    return name.isEmpty() ? message : name + QLatin1String(": ") + message;
}

} // namespace

QString KWinScriptLoader::trackerScriptPath()
{
    const QString relativePath = QStringLiteral("kwin/scripts/")
        + QLatin1String(TrackerPluginName)
        + QStringLiteral("/contents/code/main.js");

    const QStringList roots = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    for (const QString &root : roots) {
        const QString candidate = root + QLatin1Char('/') + relativePath;
        if (QFileInfo::exists(candidate)) {
            return candidate;
        }
    }

    return QString();
}

bool KWinScriptLoader::loadTrackerScript(QString *errorMessage)
{
    const auto fail = [errorMessage](const QString &message) {
        if (errorMessage) {
            *errorMessage = message;
        }
        return false;
    };

    const QString scriptPath = trackerScriptPath();
    if (scriptPath.isEmpty()) {
        return fail(QStringLiteral("helper script %1 is not installed")
                        .arg(QLatin1String(TrackerPluginName)));
    }

    QDBusMessage loadRequest = QDBusMessage::createMethodCall(QLatin1String(KWinService),
                                                              QLatin1String(KWinScriptingPath),
                                                              QLatin1String(KWinScriptingInterface),
                                                              QStringLiteral("loadScript"));
    loadRequest << scriptPath << QLatin1String(TrackerPluginName);

    const QDBusMessage loadReply
        = QDBusConnection::sessionBus().call(loadRequest, QDBus::Block, DBusTimeoutMs);
    if (loadReply.type() != QDBusMessage::ReplyMessage) {
        return fail(callFailure(loadReply));
    }

    const int scriptId = loadReply.arguments().value(0).toInt();
    if (scriptId < 0) {
        return fail(QStringLiteral("%1 is already loaded by KWin").arg(QLatin1String(TrackerPluginName)));
    }

    // KWin exports each script on "/<id>", see lingmo-kwin/src/scripting/scripting.cpp:106.
    const QString scriptObjectPath = QLatin1Char('/') + QString::number(scriptId);
    QDBusMessage runRequest = QDBusMessage::createMethodCall(QLatin1String(KWinService),
                                                             scriptObjectPath,
                                                             QLatin1String(KWinScriptInterface),
                                                             QStringLiteral("run"));

    const QDBusMessage runReply
        = QDBusConnection::sessionBus().call(runRequest, QDBus::Block, DBusTimeoutMs);
    if (runReply.type() != QDBusMessage::ReplyMessage) {
        return fail(callFailure(runReply));
    }

    return true;
}
