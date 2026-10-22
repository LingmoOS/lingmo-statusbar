// SPDX-FileCopyrightText: 2026 Lingmo OS Team
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Window
import QtQuick.Controls
import org.deepin.dtk 1.0 as D

import Lingmo.StatusBar 1.0

// Root window of the status bar.
//
// One frameless, always-on-top window spanning the top edge of the primary
// screen. It is a Qt.Tool window, so it never shows up in the task bar or in the
// window switcher. Three sections:
//
//   [ application name + reserved global menu ] ... [ clock ] ... [ tray + menu ]
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

    AppMenuSection {
        anchors {
            left: parent.left
            leftMargin: 12
            verticalCenter: parent.verticalCenter
        }
        textColor: statusBar.barTextColor
    }

    ClockSection {
        anchors {
            horizontalCenter: parent.horizontalCenter
            verticalCenter: parent.verticalCenter
        }
        darkTheme: statusBar.darkTheme
        textColor: statusBar.barTextColor
    }

    TraySection {
        anchors {
            right: parent.right
            rightMargin: 8
            verticalCenter: parent.verticalCenter
        }
        darkTheme: statusBar.darkTheme
        textColor: statusBar.barTextColor
    }
}
