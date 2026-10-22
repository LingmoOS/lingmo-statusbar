// SPDX-FileCopyrightText: 2026 Lingmo OS Team
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import org.deepin.dtk 1.0 as D

// Rightmost button of the bar: opens the system menu.
//
// In the macOS layout the clock sits between the tray icons and this button, so
// the button is its own item here instead of the tail of TraySection.
Item {
    id: root

    property color textColor: "#1b1b1b"
    property bool darkTheme: false

    implicitWidth: 22
    implicitHeight: 22

    Rectangle {
        anchors.fill: parent
        radius: 5
        color: root.darkTheme ? Qt.rgba(1, 1, 1, 0.12) : Qt.rgba(0, 0, 0, 0.07)
        visible: mouseArea.containsMouse || systemMenu.opened
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
        id: mouseArea

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

    SystemMenu {
        id: systemMenu

        parent: root
        // The button is the outermost item of the bar, so the menu is right
        // aligned with it: a centred menu would stick out of the screen.
        x: root.width - width
        y: root.height + 6
        darkTheme: root.darkTheme
        textColor: root.textColor
    }
}
