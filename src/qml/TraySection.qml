// SPDX-FileCopyrightText: 2026 Lingmo OS Team
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import org.deepin.dtk 1.0 as D

import Lingmo.StatusBar 1.0

// Right section: tray placeholders (volume / network / battery) and the system
// menu button.
//
// The icons come from TrayBridge, which returns a fixed list in phase 1; see
// src/traybridge.h for the StatusNotifierWatcher work planned for phase 2.
Row {
    id: root

    property color textColor: "#1b1b1b"
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

    Item {
        id: menuButton

        width: 22
        height: 22

        readonly property bool hovered: menuMouseArea.containsMouse

        Rectangle {
            anchors.fill: parent
            radius: 5
            color: root.darkTheme ? Qt.rgba(1, 1, 1, 0.12) : Qt.rgba(0, 0, 0, 0.07)
            visible: menuButton.hovered || systemMenu.opened
        }

        D.DciIcon {
            anchors.centerIn: parent
            name: "preferences-system"
            fallbackToQIcon: true
            sourceSize: Qt.size(16, 16)
            theme: D.DTK.themeType
            mode: D.DTK.NormalState
        }

        MouseArea {
            id: menuMouseArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                if (systemMenu.opened) {
                    systemMenu.close();
                } else {
                    systemMenu.open();
                }
            }
        }
    }

    SystemMenu {
        id: systemMenu

        parent: menuButton
        x: Math.round((menuButton.width - width) / 2)
        y: menuButton.height + 6
        darkTheme: root.darkTheme
        textColor: root.textColor
    }
}
