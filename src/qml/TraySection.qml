// SPDX-FileCopyrightText: 2026 Lingmo OS Team
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import org.deepin.dtk 1.0 as D

import Lingmo.StatusBar 1.0

// Tray placeholders (volume / network / battery).
//
// The icons come from TrayBridge, which returns a fixed list in phase 1; see
// src/traybridge.h for the StatusNotifierWatcher work planned for phase 2.
//
// Only the icon group lives here: in the macOS layout the clock sits between the
// tray and the system menu button, so the button has its own file,
// SystemMenuButton.qml, instead of being the tail of this row.
Row {
    id: root

    property bool darkTheme: false

    spacing: 2

    Repeater {
        model: TrayBridge.items

        delegate: Item {
            id: trayItem

            width: 22
            height: 22

            readonly property bool hovered: trayMouseArea.containsMouse

            Rectangle {
                anchors.fill: parent
                radius: 5
                color: root.darkTheme ? Qt.rgba(1, 1, 1, 0.12) : Qt.rgba(0, 0, 0, 0.07)
                visible: trayItem.hovered
            }

            D.DciIcon {
                anchors.centerIn: parent
                name: modelData.iconName
                fallbackToQIcon: true
                sourceSize: Qt.size(16, 16)
                theme: D.DTK.themeType
                mode: D.DTK.NormalState
            }

            MouseArea {
                id: trayMouseArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: TrayBridge.activate(modelData.id)
            }
        }
    }
}
