// SPDX-FileCopyrightText: 2026 Lingmo OS Team
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QVariantList>

// Placeholder tray for phase 1.
//
// The three items returned by items() are static: they only draw an icon from the
// icon theme and log a click. They are NOT backed by a status notifier host and
// they cannot show the real tray icons of running applications.
//
// TODO(phase 2): implement the tray for real.
//   * own the org.kde.StatusNotifierWatcher service (legacy name
//     com.canonical.StatusNotifierWatcher) and implement
//     RegisterStatusNotifierItem()/RegisterStatusNotifierHost(),
//   * host org.kde.StatusNotifierItem objects (legacy com.canonical.StatusNotifierItem):
//     Id, Title, IconName/IconPixmap, ToolTip, Activate, SecondaryActivate,
//     ContextMenu,
//   * render each item's menu through com.canonical.dbusmenu.
class TrayBridge : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList items READ items CONSTANT)

public:
    explicit TrayBridge(QObject *parent = nullptr);

    // Fixed list of { id, iconName, tooltip } maps: volume, network, battery.
    QVariantList items() const;

    // Called when a placeholder icon is clicked; logs only in phase 1.
    Q_INVOKABLE void activate(const QString &id);
};
