// SPDX-FileCopyrightText: 2026 Lingmo OS Team
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick

// One row of an application menu popup: a regular entry, a checkable entry or a
// separator. Purely presentational; the popup decides what a click does.
Item {
    id: root

    property string text: ""
    property bool separator: false
    property bool itemEnabled: true
    property bool hasSubmenu: false
    property bool checked: false
    property bool highlighted: false
    property color textColor: "#1b1b1b"
    property bool darkTheme: false

    signal activated()
    signal submenuRequested()

    implicitWidth: 208
    implicitHeight: root.separator ? 9 : 26

    Rectangle {
        visible: root.separator
        anchors.verticalCenter: parent.verticalCenter
        x: 8
        width: parent.width - 16
        height: 1
        color: root.darkTheme ? Qt.rgba(1, 1, 1, 0.12) : Qt.rgba(0, 0, 0, 0.10)
    }

    Rectangle {
        anchors.fill: parent
        anchors.margins: 2
        radius: 4
        visible: !root.separator && (root.highlighted || mouseArea.containsMouse)
        color: root.darkTheme ? Qt.rgba(1, 1, 1, 0.12) : Qt.rgba(0, 0, 0, 0.07)
    }

    Text {
        id: checkMark

        visible: !root.separator && root.checked
        anchors {
            left: parent.left
            leftMargin: 8
            verticalCenter: parent.verticalCenter
        }
        text: qsTr("✓")
        color: root.textColor
        font.pixelSize: 11
    }

    Text {
        id: label

        visible: !root.separator
        anchors {
            left: parent.left
            leftMargin: root.checked ? 24 : 8
            right: arrow.visible ? arrow.left : parent.right
            rightMargin: 8
            verticalCenter: parent.verticalCenter
        }
        elide: Text.ElideRight
        text: root.text
        color: root.textColor
        opacity: root.itemEnabled ? 0.95 : 0.4
        font.pixelSize: 12
    }

    Text {
        id: arrow

        visible: !root.separator && root.hasSubmenu
        anchors {
            right: parent.right
            rightMargin: 8
            verticalCenter: parent.verticalCenter
        }
        text: qsTr("›")
        color: root.textColor
        opacity: 0.55
        font.pixelSize: 12
    }

    MouseArea {
        id: mouseArea

        anchors.fill: parent
        hoverEnabled: !root.separator && root.itemEnabled
        enabled: !root.separator && root.itemEnabled
        cursorShape: Qt.PointingHandCursor

        onContainsMouseChanged: {
            if (containsMouse && root.hasSubmenu) {
                root.submenuRequested();
            }
        }
        onClicked: {
            if (root.hasSubmenu) {
                root.submenuRequested();
            } else {
                root.activated();
            }
        }
    }
}
