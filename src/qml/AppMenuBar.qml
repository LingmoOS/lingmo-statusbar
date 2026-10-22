// SPDX-FileCopyrightText: 2026 Lingmo OS Team
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick

import Lingmo.StatusBar 1.0

// Top level titles of the active window's menu bar.
//
// AppMenu is fed by AppMenuTracker, which reads the two X11 properties a window
// uses to advertise its com.canonical.dbusmenu object
// (_KDE_NET_WM_APPMENU_SERVICE_NAME / _KDE_NET_WM_APPMENU_OBJECT_PATH). A window
// without a menu, a Wayland session or a missing X11 connection leaves
// AppMenu.topLevelItems empty, and this row renders nothing at all.
Row {
    id: root

    property color textColor: "#1b1b1b"
    property bool darkTheme: false

    spacing: 2

    Repeater {
        model: AppMenu.topLevelItems

        delegate: Item {
            id: entry

            width: label.implicitWidth + 16
            height: 22

            Rectangle {
                anchors.fill: parent
                radius: 5
                visible: mouseArea.containsMouse || menuPopup.opened
                color: root.darkTheme ? Qt.rgba(1, 1, 1, 0.12) : Qt.rgba(0, 0, 0, 0.07)
            }

            Text {
                id: label

                anchors.centerIn: parent
                text: modelData.text
                color: root.textColor
                opacity: modelData.enabled ? 0.9 : 0.4
                font.pixelSize: 12
            }

            MouseArea {
                id: mouseArea

                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    if (menuPopup.opened) {
                        menuPopup.close();
                    } else {
                        menuPopup.open();
                    }
                }
            }

            AppMenuPopup {
                id: menuPopup

                parent: entry
                x: 0
                y: entry.height + 6
                menuId: modelData.id
                textColor: root.textColor
                darkTheme: root.darkTheme
            }
        }
    }
}
