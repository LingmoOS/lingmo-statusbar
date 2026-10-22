// SPDX-FileCopyrightText: 2026 Lingmo OS Team
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick

import Lingmo.StatusBar 1.0

// Left section: application name of the active window, followed by the menu titles
// of that window (phase 2).
Item {
    id: root

    property color textColor: "#1b1b1b"
    property bool darkTheme: false

    implicitWidth: row.implicitWidth
    implicitHeight: 24

    Row {
        id: row

        anchors.verticalCenter: parent.verticalCenter
        height: 24
        spacing: 10

        Text {
            id: applicationName

            height: parent.height
            width: Math.min(implicitWidth, 220)
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
            text: ActiveWindowWatcher.appName
            color: root.textColor
            font.pixelSize: 13
            font.weight: Font.DemiBold
        }

        // Menu titles of the active window. AppMenuBar renders nothing when the
        // window exports no menu (see src/appmenu/ and src/qml/AppMenuBar.qml).
        AppMenuBar {
            height: parent.height
            textColor: root.textColor
            darkTheme: root.darkTheme
        }
    }
}
