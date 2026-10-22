// SPDX-FileCopyrightText: 2026 Lingmo OS Team
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Window

// Time and date, at the right end of the bar next to the system menu button.
// Clicking it opens the calendar popup.
Item {
    id: root

    property color textColor: "#1b1b1b"
    property bool darkTheme: false

    implicitWidth: content.implicitWidth + 16
    implicitHeight: 24

    property date now: new Date()

    Timer {
        interval: 1000
        repeat: true
        running: true
        triggeredOnStart: true
        onTriggered: root.now = new Date()
    }

    Rectangle {
        anchors.fill: parent
        radius: 6
        color: root.darkTheme ? Qt.rgba(1, 1, 1, 0.10) : Qt.rgba(0, 0, 0, 0.06)
        visible: mouseArea.containsMouse
    }

    Row {
        id: content

        anchors.centerIn: parent
        height: parent.height
        spacing: 6

        Text {
            // Localised clock format is a phase-2 item; Qt.formatDateTime() uses the
            // C locale for names, which is what the bar shows for now.
            height: parent.height
            verticalAlignment: Text.AlignVCenter
            text: Qt.formatDateTime(root.now, "HH:mm")
            color: root.textColor
            font.pixelSize: 13
            font.weight: Font.DemiBold
        }

        Text {
            height: parent.height
            verticalAlignment: Text.AlignVCenter
            text: Qt.formatDateTime(root.now, "ddd d MMM")
            color: root.textColor
            opacity: 0.7
            font.pixelSize: 12
        }
    }

    MouseArea {
        id: mouseArea

        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: {
            if (calendarPopup.opened) {
                calendarPopup.close();
            } else {
                calendarPopup.open();
            }
        }
    }

    CalendarPopup {
        id: calendarPopup

        // The bar window is only as tall as the bar itself, so the popup is a
        // separate window (popupType: Popup.Window) anchored under the clock. x/y
        // stay relative to this item, so the popup follows the clock to its place
        // at the right end of the bar without any fixed screen coordinate.
        //
        // It is right aligned with the clock instead of centred on it: the clock
        // now sits close to the right screen edge, where a centred 280 px popup
        // would hang over that edge.
        parent: root
        x: root.width - width
        y: root.height + 4
        darkTheme: root.darkTheme
    }
}
