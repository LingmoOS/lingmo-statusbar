// SPDX-FileCopyrightText: 2026 Lingmo OS Team
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDBusServiceWatcher>
#include <QHash>
#include <QList>
#include <QObject>
#include <QString>
#include <QVariantList>

// One entry of a com.canonical.dbusmenu layout tree.
struct AppMenuItem
{
    int id = 0;
    QString text;
    QString type; // "standard" or "separator"
    bool enabled = true;
    bool visible = true;
    bool checkable = false;
    bool checked = false;
    bool hasSubmenu = false;
    QList<int> children;
};

// Client half of com.canonical.dbusmenu.
//
// Phase 2 of lingmo-statusbar: the bar shows the menu of the active window. The
// window does not publish its menu titles itself; it exports a dbusmenu object and
// advertises the service name plus object path in two X11 properties on its window
// (AppMenuTracker reads those). This class talks to that object:
//
//   bool           AboutToShow(int id)
//   a(ia{sv}av)    GetLayout(int parentId, int recursionDepth, as propertyNames)
//   void           Event(int id, s eventId, v data, u timestamp)
//   signal         LayoutUpdated(uint revision, int parentId)
//
// The signatures were taken from the XML shipped by the ecosystem
// (lingmo-workspace/appmenu/com.canonical.AppMenu.Registrar.xml, and the retired
// lingmo-statusbar's src/libdbusmenuqt/com.canonical.dbusmenu.xml), and the
// layout demarshalling follows the same beginStructure/beginArray order as
// libdbusmenuqt's dbusmenutypes_p.cpp:78-95.
class DbusMenuClient : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool available READ available NOTIFY changed)
    Q_PROPERTY(QString service READ service NOTIFY changed)
    Q_PROPERTY(QString objectPath READ objectPath NOTIFY changed)
    // Top level entries, ready to feed a Repeater: { id, text, enabled, separator,
    // hasSubmenu, checked }.
    Q_PROPERTY(QVariantList topLevelItems READ topLevelItems NOTIFY changed)

public:
    explicit DbusMenuClient(QObject *parent = nullptr);

    bool available() const;
    QString service() const;
    QString objectPath() const;
    QVariantList topLevelItems() const;

    // Menu advertised by the active window; an empty service or path clears it.
    Q_INVOKABLE void setMenu(const QString &service, const QString &objectPath);
    Q_INVOKABLE void clear();

    // Children of an entry, same shape as topLevelItems().
    Q_INVOKABLE QVariantList childrenOf(int id) const;

    // dbusmenu protocol.
    Q_INVOKABLE void aboutToShow(int id);
    Q_INVOKABLE void activate(int id);

Q_SIGNALS:
    void changed();

private:
    void fetchLayout();
    void onLayoutReply(const QList<QVariant> &arguments);
    void resetTree();
    QVariantList entriesFor(const QList<int> &ids) const;

private Q_SLOTS:
    // Connected by name in setMenu(): LayoutUpdated(uint revision, int parentId).
    void onLayoutUpdated(uint revision, int parentId);

private:
    QString m_service;
    QString m_objectPath;
    bool m_available = false;
    int m_layoutRevision = -1;

    // Flat id -> item map, children are referenced by id to keep the recursive
    // layout easy to walk from QML.
    QHash<int, AppMenuItem> m_items;
    QList<int> m_rootChildren;
    QDBusServiceWatcher m_serviceWatcher;
};
