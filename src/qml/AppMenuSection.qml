// SPDX-FileCopyrightText: 2026 Lingmo OS Team
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick

import Lingmo.StatusBar 1.0

// Left section: application name of the active window, plus the reserved slot for
// the global menu.
Item {
    id: root

    property color textColor: "#1b1b1b"

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

        // ------------------------------------------------------------------
        // Global menu (phase 2). com.canonical.dbusmenu is NOT implemented in
        // phase 1: this slot is reserved and stays empty, nothing is faked here.
        //
        // Phase 2 feeds `globalMenuRow.globalMenuModel` from the window's
        // com.canonical.dbusmenu node (Layout signal + AboutToShow), exported by
        // org.kde.kappmenu on the session bus, and the Repeater below renders the
        // menu titles of the active window.
        // ------------------------------------------------------------------
        Row {
            id: globalMenuRow

            height: parent.height
            spacing: 6

            property var globalMenuModel: null

            Repeater {
                model: globalMenuRow.globalMenuModel

                delegate: Text {
                    height: globalMenuRow.height
                    verticalAlignment: Text.AlignVCenter
                    text: modelData && modelData.title ? modelData.title : ""
                    color: root.textColor
                    font.pixelSize: 12
                    opacity: 0.85
                }
            }
        }
    }
}
