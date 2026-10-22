// SPDX-FileCopyrightText: 2026 Lingmo OS Team
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbusmenuclient.h"

#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QDBusPendingCallWatcher>
#include <QDBusVariant>
#include <QDebug>
#include <QStringList>

namespace {

constexpr auto MenuInterface = "com.canonical.dbusmenu";
constexpr auto MenuSignalLayoutUpdated = "LayoutUpdated";
constexpr int LayoutInfiniteDepth = -1;

// dbusmenu labels mark mnemonics with '_' and a literal underscore with "__".
QString cleanLabel(const QVariant &value)
{
    QString text = value.toString();
    if (text.isEmpty()) {
        return text;
    }

    const QChar escape(0x01);
    text.replace(QLatin1String("__"), QString(escape));
    text.remove(QLatin1Char('_'));
    text.replace(escape, QLatin1Char('_'));
    return text;
}

// a(ia{sv}av): one entry of the layout tree, children are variants of the very
// same structure. Demarshalling order mirrors libdbusmenuqt's
// dbusmenutypes_p.cpp:78-95.
void parseLayoutItem(const QDBusArgument &argument, AppMenuItem &item, QHash<int, AppMenuItem> &flat)
{
    argument.beginStructure();

    int id = 0;
    QVariantMap properties;
    argument >> id >> properties;

    item.id = id;
    item.text = cleanLabel(properties.value(QStringLiteral("label")));
    item.type = properties.value(QStringLiteral("type"), QStringLiteral("standard")).toString();
    item.enabled = properties.value(QStringLiteral("enabled"), true).toBool();
    item.visible = properties.value(QStringLiteral("visible"), true).toBool();
    item.checkable = !properties.value(QStringLiteral("toggle-type")).toString().isEmpty();
    item.checked = properties.value(QStringLiteral("toggle-state"), 0).toInt() == 1;
    item.hasSubmenu = properties.value(QStringLiteral("children-display")).toString() == QLatin1String("submenu");

    argument.beginArray();
    while (!argument.atEnd()) {
        QDBusVariant childVariant;
        argument >> childVariant;

        const QDBusArgument childArgument = childVariant.variant().value<QDBusArgument>();
        AppMenuItem child;
        parseLayoutItem(childArgument, child, flat);
        item.children.append(child.id);
    }
    argument.endArray();

    argument.endStructure();

    flat.insert(item.id, item);
}

QVariantMap entryMap(const AppMenuItem &item)
{
    return {
        { QStringLiteral("id"), item.id },
        { QStringLiteral("text"), item.type == QLatin1String("separator") ? QString() : item.text },
        { QStringLiteral("enabled"), item.enabled },
        { QStringLiteral("separator"), item.type == QLatin1String("separator") },
        { QStringLiteral("hasSubmenu"), item.hasSubmenu },
        { QStringLiteral("checked"), item.checked },
    };
}

} // namespace

DbusMenuClient::DbusMenuClient(QObject *parent)
    : QObject(parent)
    , m_serviceWatcher(this)
{
    m_serviceWatcher.setConnection(QDBusConnection::sessionBus());
    m_serviceWatcher.setWatchMode(QDBusServiceWatcher::WatchForUnregistration);
    connect(&m_serviceWatcher, &QDBusServiceWatcher::serviceUnregistered, this, [this](const QString &service) {
        if (service == m_service) {
            // The application went away; drop the stale menu.
            clear();
        }
    });
}

bool DbusMenuClient::available() const
{
    return m_available;
}

QString DbusMenuClient::service() const
{
    return m_service;
}

QString DbusMenuClient::objectPath() const
{
    return m_objectPath;
}

QVariantList DbusMenuClient::topLevelItems() const
{
    return entriesFor(m_rootChildren);
}

QVariantList DbusMenuClient::childrenOf(int id) const
{
    const auto it = m_items.constFind(id);
    if (it == m_items.constEnd()) {
        return {};
    }
    return entriesFor(it->children);
}

QVariantList DbusMenuClient::entriesFor(const QList<int> &ids) const
{
    QVariantList entries;
    entries.reserve(ids.size());
    for (int id : ids) {
        const auto it = m_items.constFind(id);
        if (it == m_items.constEnd() || !it->visible) {
            continue;
        }
        entries.append(entryMap(*it));
    }
    return entries;
}

void DbusMenuClient::setMenu(const QString &service, const QString &objectPath)
{
    if (service.isEmpty() || objectPath.isEmpty()) {
        clear();
        return;
    }
    if (service == m_service && objectPath == m_objectPath) {
        return;
    }

    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!m_service.isEmpty()) {
        bus.disconnect(m_service, m_objectPath, QLatin1String(MenuInterface),
                       QLatin1String(MenuSignalLayoutUpdated), this,
                       SLOT(onLayoutUpdated(uint, int)));
        m_serviceWatcher.removeWatchedService(m_service);
    }

    m_service = service;
    m_objectPath = objectPath;
    m_available = true;
    m_layoutRevision = -1;
    resetTree();

    m_serviceWatcher.addWatchedService(m_service);
    bus.connect(m_service, m_objectPath, QLatin1String(MenuInterface),
                QLatin1String(MenuSignalLayoutUpdated), this,
                SLOT(onLayoutUpdated(uint, int)));

    // Let the application build its real menu before asking for the layout.
    aboutToShow(0);
    fetchLayout();

    Q_EMIT changed();
}

void DbusMenuClient::clear()
{
    if (!m_available && m_service.isEmpty()) {
        return;
    }

    if (!m_service.isEmpty()) {
        QDBusConnection::sessionBus().disconnect(m_service, m_objectPath, QLatin1String(MenuInterface),
                                                QLatin1String(MenuSignalLayoutUpdated), this,
                                                SLOT(onLayoutUpdated(uint, int)));
        m_serviceWatcher.removeWatchedService(m_service);
    }

    m_service.clear();
    m_objectPath.clear();
    m_available = false;
    m_layoutRevision = -1;
    resetTree();

    Q_EMIT changed();
}

void DbusMenuClient::resetTree()
{
    m_items.clear();
    m_rootChildren.clear();
}

void DbusMenuClient::aboutToShow(int id)
{
    if (m_service.isEmpty()) {
        return;
    }

    QDBusMessage message = QDBusMessage::createMethodCall(m_service, m_objectPath,
                                                          QLatin1String(MenuInterface),
                                                          QStringLiteral("AboutToShow"));
    message << id;
    QDBusConnection::sessionBus().send(message);
}

void DbusMenuClient::activate(int id)
{
    if (m_service.isEmpty()) {
        return;
    }

    const auto it = m_items.constFind(id);
    if (it != m_items.constEnd() && !it->enabled) {
        return;
    }

    // (id, "clicked", variant data, timestamp). Timestamp 0 is what libdbusmenu's
    // own clients send when they have no X11 event time at hand.
    QDBusMessage message = QDBusMessage::createMethodCall(m_service, m_objectPath,
                                                          QLatin1String(MenuInterface),
                                                          QStringLiteral("Event"));
    message << id << QStringLiteral("clicked")
            << QVariant::fromValue(QDBusVariant(QString())) << uint(0);
    QDBusConnection::sessionBus().send(message);

    // A top level entry being clicked from the bar has no persistent state in the
    // client, refresh anyway so checkable items pick up the new toggle state.
    fetchLayout();
}

void DbusMenuClient::fetchLayout()
{
    if (m_service.isEmpty()) {
        return;
    }

    // Only the properties the bar renders; a non-empty list keeps the reply small.
    const QStringList propertyNames {
        QStringLiteral("label"),
        QStringLiteral("enabled"),
        QStringLiteral("visible"),
        QStringLiteral("type"),
        QStringLiteral("children-display"),
        QStringLiteral("toggle-type"),
        QStringLiteral("toggle-state"),
    };

    QDBusMessage message = QDBusMessage::createMethodCall(m_service, m_objectPath,
                                                          QLatin1String(MenuInterface),
                                                          QStringLiteral("GetLayout"));
    message << 0 << LayoutInfiniteDepth << propertyNames;

    auto *watcher = new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(message), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *call) {
        call->deleteLater();

        const QDBusMessage reply = call->reply();
        if (reply.type() != QDBusMessage::ReplyMessage) {
            qWarning().noquote() << "[appmenu] GetLayout failed:" << reply.errorName()
                                 << reply.errorMessage();
            return;
        }

        onLayoutReply(reply.arguments());
    });
}

void DbusMenuClient::onLayoutReply(const QList<QVariant> &arguments)
{
    // GetLayout returns (revision, layout).
    if (arguments.size() < 2) {
        return;
    }

    const QVariant layoutValue = arguments.at(1);
    if (!layoutValue.canConvert<QDBusArgument>()) {
        qWarning() << "[appmenu] unexpected GetLayout reply type" << layoutValue.typeName();
        return;
    }

    resetTree();

    const QDBusArgument layout = layoutValue.value<QDBusArgument>();
    layout.beginArray();
    while (!layout.atEnd()) {
        AppMenuItem item;
        parseLayoutItem(layout, item, m_items);
        m_rootChildren.append(item.id);
    }
    layout.endArray();

    Q_EMIT changed();
}

void DbusMenuClient::onLayoutUpdated(uint revision, int parentId)
{
    Q_UNUSED(parentId)
    if (static_cast<int>(revision) == m_layoutRevision) {
        return;
    }
    m_layoutRevision = static_cast<int>(revision);
    fetchLayout();
}
