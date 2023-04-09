/*
 * Copyright (C) 2021 CuteOS Team.
 *
 * Author:     Reion Wong <aj@cuteos.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtQuick.Window 2.12

import Cute.System 1.0 as System
import Cute.StatusBar 1.0
import Cute.NetworkManagement 1.0 as NM
import CuteUI 1.0 as CuteUI

Item {
    id: rootItem

    property int iconSize: 16

    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true

    property bool darkMode: false
    property color textColor: rootItem.darkMode ? "#FFFFFF" : "#000000";
    property var fontSize: rootItem.height ? rootItem.height / 3 : 1

    System.Wallpaper {
        id: sysWallpaper

        function reload() {
            if (sysWallpaper.type === 0)
                bgHelper.setBackgound(sysWallpaper.path)
            else
                bgHelper.setColor(sysWallpaper.color)
        }

        Component.onCompleted: sysWallpaper.reload()

        onTypeChanged: sysWallpaper.reload()
        onColorChanged: sysWallpaper.reload()
        onPathChanged: sysWallpaper.reload()
    }

    BackgroundHelper {
        id: bgHelper

        onNewColor: {
            background.color = color
            rootItem.darkMode = darkMode
        }
    }

    Rectangle {
        id: background
        anchors.fill: parent
        opacity: 0.3

       color: CuteUI.Theme.darkMode ? "#4D4D4D" : "#FFFFFF"
       opacity: windowHelper.compositing ? CuteUI.Theme.darkMode ? 0.5 : 0.7 : 1.0

       Behavior on color {
           ColorAnimation {
               duration: 100
               easing.type: Easing.Linear
           }
       }
    }

    CuteUI.WindowHelper {
        id: windowHelper
    }

    CuteUI.PopupTips {
        id: popupTips
    }

    CuteUI.DesktopMenu {
        id: acticityMenu

        MenuItem {
            text: qsTr("Close")
            onTriggered: acticity.close()
        }
    }

    // Main layout
    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: CuteUI.Units.smallSpacing
        anchors.rightMargin: CuteUI.Units.smallSpacing
        spacing: CuteUI.Units.smallSpacing / 2

        // App name
        

        // App menu
        
        StandardItem {
            id: lyricsItem
            visible: lyricsHelper.lyricsVisible
            animationEnabled: true
            Layout.fillHeight: true
            Layout.preferredWidth: _lyricsLayout.implicitWidth + CuteUI.Units.smallSpacing

            RowLayout {
                id: _lyricsLayout
                anchors.fill: parent

                Label {
                    id: lyricsLabel
                    Layout.alignment: Qt.AlignCenter
                    font.pointSize: rootItem.fontSize
                    color: rootItem.textColor
                    text: lyricsHelper.lyrics
                    }
                }
         }
        // System tray(Right)
        SystemTray {}

        StandardItem {
            id: controler

            checked: controlCenter.item.visible
            animationEnabled: true
            Layout.fillHeight: true
            Layout.preferredWidth: _controlerLayout.implicitWidth + CuteUI.Units.largeSpacing

            onClicked: {
                toggleDialog()
            }

            function toggleDialog() {
                if (controlCenter.item.visible)
                    controlCenter.item.close()
                else {
                    // 先初始化，用户可能会通过Alt鼠标左键移动位置
                    controlCenter.item.position = Qt.point(0, 0)
                    controlCenter.item.position = mapToGlobal(0, 0)
                    controlCenter.item.open()
                }
            }

            RowLayout {
                id: _controlerLayout
                anchors.fill: parent
                anchors.leftMargin: CuteUI.Units.smallSpacing
                anchors.rightMargin: CuteUI.Units.smallSpacing

                spacing: CuteUI.Units.largeSpacing

                Image {
                    id: volumeIcon
                    visible: controlCenter.item.defaultSink
                    source: "qrc:/images/" + (rootItem.darkMode ? "dark/" : "light/") + controlCenter.item.volumeIconName + ".svg"
                    width: rootItem.iconSize
                    height: width
                    sourceSize: Qt.size(width, height)
                    asynchronous: true
                    Layout.alignment: Qt.AlignCenter
                    antialiasing: true
                    smooth: false
                }

                Image {
                    id: wirelessIcon
                    width: rootItem.iconSize
                    height: width
                    sourceSize: Qt.size(width, height)
                    source: activeConnection.wirelessIcon ? "qrc:/images/" + (rootItem.darkMode ? "dark/" : "light/") + activeConnection.wirelessIcon + ".svg" : ""
                    asynchronous: true
                    Layout.alignment: Qt.AlignCenter
                    visible: enabledConnections.wirelessHwEnabled &&
                             enabledConnections.wirelessEnabled &&
                             activeConnection.wirelessName &&
                             wirelessIcon.status === Image.Ready
                    antialiasing: true
                    smooth: false
                }

                // Battery Item
                RowLayout {
                    visible: battery.available

                    Image {
                        id: batteryIcon
                        height: rootItem.iconSize
                        width: height + 6
                        sourceSize: Qt.size(width, height)
                        source: "qrc:/images/" + (rootItem.darkMode ? "dark/" : "light/") + battery.iconSource
                        Layout.alignment: Qt.AlignCenter
                        antialiasing: true
                        smooth: false
                    }

                    Label {
                        text: battery.chargePercent + "%"
                        font.pointSize: rootItem.fontSize
                        color: rootItem.textColor
                        visible: battery.showPercentage
                    }
                }
            }
        }

        StandardItem {
            id: shutdownItem

            animationEnabled: true
            Layout.fillHeight: true
            Layout.preferredWidth: shutdownIcon.implicitWidth + CuteUI.Units.smallSpacing
            checked: shutdownDialog.item.visible

            onClicked: {
                shutdownDialog.item.position = Qt.point(0, 0)
                shutdownDialog.item.position = mapToGlobal(0, 0)
                shutdownDialog.item.open()
            }

            Image {
                id: shutdownIcon
                anchors.centerIn: parent
                width: rootItem.iconSize
                height: width
                sourceSize: Qt.size(width, height)
                source: "qrc:/images/" + (rootItem.darkMode ? "dark/" : "light/") + "system-shutdown-symbolic.svg"
                asynchronous: true
                antialiasing: true
                smooth: false
            }
        }

        // 弹出通知中心和日历
        
    }

    MouseArea {
        id: _sliding
        anchors.fill: parent
        z: -1

        property int startY: -1
        property bool activated: false

        onActivatedChanged: {
            // TODO
            // if (activated)
            //     acticity.move()
        }

        onPressed: {
            startY = mouse.y
        }

        onReleased: {
            startY = -1
        }

        onDoubleClicked: {
            acticity.toggleMaximize()
        }

        onMouseYChanged: {
            if (startY === parseInt(mouse.y)) {
                activated = false
                return
            }

            // Up
            if (startY > parseInt(mouse.y)) {
                activated = false
                return
            }

            if (mouse.y > rootItem.height)
                activated = true
            else
                activated = false
        }
    }

    // Components
    Loader {
        id: controlCenter
        sourceComponent: ControlCenter {}
        asynchronous: true
    }

    Loader {
        id: shutdownDialog
        sourceComponent: ShutdownDialog {}
        asynchronous: true
    }

    NM.ActiveConnection {
        id: activeConnection
    }

    NM.EnabledConnections {
        id: enabledConnections
    }

    NM.Handler {
        id: nmHandler
    }
}
