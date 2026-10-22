// SPDX-FileCopyrightText: 2026 Lingmo OS Team
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "traybridge.h"

#include <QDebug>

namespace {

QVariantMap makeItem(const QString &id, const QString &iconName, const QString &tooltip)
{
    return {
        { QStringLiteral("id"), id },
        { QStringLiteral("iconName"), iconName },
        { QStringLiteral("tooltip"), tooltip },
    };
}

} // namespace

TrayBridge::TrayBridge(QObject *parent)
    : QObject(parent)
{
}

QVariantList TrayBridge::items() const
{
    // Icon theme names (freedesktop naming). D.DciIcon looks these up with
    // fallbackToQIcon, so the bar shows them with the current icon theme.
    return {
        makeItem(QStringLiteral("audio"), QStringLiteral("audio-volume-medium"), QStringLiteral("Volume")),
        makeItem(QStringLiteral("network"), QStringLiteral("network-wireless"), QStringLiteral("Network")),
        makeItem(QStringLiteral("battery"), QStringLiteral("battery-good"), QStringLiteral("Battery")),
    };
}

void TrayBridge::activate(const QString &id)
{
    // TODO(phase 2): forward to the matching org.kde.StatusNotifierItem, which
    // replaces this placeholder list entirely.
    qInfo().noquote() << "[tray]" << id << "- placeholder item, no tray host in phase 1";
}
