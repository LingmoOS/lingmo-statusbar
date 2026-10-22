// SPDX-FileCopyrightText: 2026 Lingmo OS Team
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Window
import org.deepin.dtk 1.0 as D

import Lingmo.StatusBar 1.0

// Root window of the status bar.
//
// One frameless, always-on-top window spanning the top edge of the primary
// screen. It is a Qt.Tool window, so it never shows up in the task bar or in the
// window switcher.
//
// The items are grouped at the two ends of the bar and the middle stays empty,
// which is how macOS arranges its menu bar (this bar is not GNOME: the clock is
// not in the middle):
//
//   [ Lingmo ][ app name + reserved global menu ] ... [ tray ][ clock ][ menu ]
//   |<------------------- left ------------------->|     |<---- right ---->|
//
// Read from the outside in, the right group is the system menu button, then the
// clock, then the tray icons. The Item with Layout.fillWidth is the only thing in
// the middle: it absorbs the free width and keeps the two groups apart.
//
// ScreenGeometry owns the window rectangle from C++ (src/screengeometry.cpp) so
// screen hotplug and resolution changes do not need Screen.* bindings here.
D.ApplicationWindow {
    id: statusBar

    x: ScreenGeometry.barX
    y: ScreenGeometry.barY
    width: ScreenGeometry.barWidth
    height: ScreenGeometry.barHeight

    // Frameless, kept above normal windows, and out of the task bar.
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool

    color: "transparent"
    visible: true
    title: qsTr("Lingmo Status Bar")

    readonly property bool darkTheme: D.DTK.themeType === D.ApplicationHelper.DarkType
    readonly property color barColor: darkTheme ? Qt.rgba(0.11, 0.11, 0.12, 0.94)
                                                : Qt.rgba(0.98, 0.98, 0.98, 0.94)
    readonly property color barTextColor: darkTheme ? "#e6e6e6" : "#1b1b1b"
    readonly property color hairlineColor: darkTheme ? Qt.rgba(1, 1, 1, 0.10)
                                                     : Qt.rgba(0, 0, 0, 0.08)

    background: Rectangle {
        color: statusBar.barColor

        Rectangle {
            anchors {
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }
            height: 1
            color: statusBar.hairlineColor
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // Left, from the left edge: the Lingmo menu first (the Apple menu of the
        // macOS layout), then the application name and the reserved global menu.
        LingmoMenu {
            Layout.leftMargin: 12
            Layout.alignment: Qt.AlignVCenter
            darkTheme: statusBar.darkTheme
            textColor: statusBar.barTextColor
        }

        AppMenuSection {
            Layout.leftMargin: 10
            Layout.alignment: Qt.AlignVCenter
            textColor: statusBar.barTextColor
        }

        // Middle: deliberately empty. The spacer takes all the free width, so the
        // left group stays at the left edge and the right group at the right edge.
        Item {
            Layout.fillWidth: true
        }

        // Right, from left to right: tray icons, clock, system menu button. The
        // system menu is the outermost item and the clock sits between it and the
        // tray, which is the order macOS uses.
        TraySection {
            Layout.alignment: Qt.AlignVCenter
            darkTheme: statusBar.darkTheme
        }

        ClockSection {
            Layout.leftMargin: 6
            Layout.alignment: Qt.AlignVCenter
            darkTheme: statusBar.darkTheme
            textColor: statusBar.barTextColor
        }

        SystemMenuButton {
            Layout.leftMargin: 6
            Layout.rightMargin: 8
            Layout.alignment: Qt.AlignVCenter
            darkTheme: statusBar.darkTheme
            textColor: statusBar.barTextColor
        }
    }
}
