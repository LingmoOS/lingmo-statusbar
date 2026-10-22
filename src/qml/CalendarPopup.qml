// SPDX-FileCopyrightText: 2026 Lingmo OS Team
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import org.deepin.dtk 1.0 as D

// Simple month calendar opened from the clock section.
//
// Display only: phase 1 has no calendar/agenda backend, so clicking a day does
// nothing.
Popup {
    id: root

    property bool darkTheme: false

    // First day of the displayed month.
    property date shownMonth: {
        const today = new Date();
        return new Date(today.getFullYear(), today.getMonth(), 1);
    }

    // 42 cells: six weeks, starting on the Monday on or before the 1st.
    property var cells: []

    popupType: Popup.Window
    width: 280
    height: 266
    padding: 0
    modal: false
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    readonly property color panelColor: darkTheme ? Qt.rgba(0.16, 0.16, 0.17, 0.98)
                                                 : Qt.rgba(1, 1, 1, 0.98)
    readonly property color labelColor: darkTheme ? "#e6e6e6" : "#1b1b1b"
    readonly property color mutedColor: darkTheme ? Qt.rgba(1, 1, 1, 0.40) : Qt.rgba(0, 0, 0, 0.32)
    readonly property color accentColor: darkTheme ? "#2f86d8" : "#0071e3"

    function isSameDay(a, b) {
        return a.getFullYear() === b.getFullYear()
            && a.getMonth() === b.getMonth()
            && a.getDate() === b.getDate();
    }

    function rebuildCells() {
        const first = new Date(root.shownMonth.getFullYear(), root.shownMonth.getMonth(), 1);
        // Monday-first grid: getDay() is 0 for Sunday.
        const offset = (first.getDay() + 6) % 7;
        const today = new Date();

        const list = [];
        for (let i = 0; i < 42; ++i) {
            const day = new Date(first.getFullYear(), first.getMonth(), 1 - offset + i);
            list.push({
                "day": day.getDate(),
                "inMonth": day.getMonth() === root.shownMonth.getMonth(),
                "isToday": root.isSameDay(day, today)
            });
        }
        root.cells = list;
    }

    onShownMonthChanged: rebuildCells()
    Component.onCompleted: rebuildCells()

    background: Rectangle {
        radius: 10
        color: root.panelColor
        border.width: 1
        border.color: root.darkTheme ? Qt.rgba(1, 1, 1, 0.12) : Qt.rgba(0, 0, 0, 0.08)
    }

    contentItem: Item {
        Column {
            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
                margins: 12
            }
            spacing: 8

            // Month switcher.
            Row {
                width: parent.width
                height: 26
                spacing: 4

                Item {
                    id: previousMonth
                    width: 24
                    height: 24

                    D.DciIcon {
                        anchors.centerIn: parent
                        name: "arrow_ordinary_left"
                        fallbackToQIcon: true
                        sourceSize: Qt.size(12, 12)
                        theme: D.DTK.themeType
                        mode: D.DTK.NormalState
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            root.shownMonth = new Date(root.shownMonth.getFullYear(),
                                                       root.shownMonth.getMonth() - 1, 1);
                        }
                    }
                }

                Text {
                    width: parent.width - 56
                    height: parent.height
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    text: Qt.formatDateTime(root.shownMonth, "MMMM yyyy")
                    color: root.labelColor
                    font.pixelSize: 13
                    font.weight: Font.DemiBold
                }

                Item {
                    id: nextMonth
                    width: 24
                    height: 24

                    D.DciIcon {
                        anchors.centerIn: parent
                        name: "arrow_ordinary_right"
                        fallbackToQIcon: true
                        sourceSize: Qt.size(12, 12)
                        theme: D.DTK.themeType
                        mode: D.DTK.NormalState
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            root.shownMonth = new Date(root.shownMonth.getFullYear(),
                                                       root.shownMonth.getMonth() + 1, 1);
                        }
                    }
                }
            }

            // Weekday header. 2024-01-01 was a Monday, which keeps the labels in the
            // same order as the Monday-first grid above without hard-coding names.
            Row {
                width: parent.width
                spacing: 0

                Repeater {
                    model: 7

                    delegate: Text {
                        width: 36
                        height: 16
                        horizontalAlignment: Text.AlignHCenter
                        text: Qt.formatDateTime(new Date(2024, 0, 1 + index), "ddd")
                        color: root.mutedColor
                        font.pixelSize: 11
                    }
                }
            }

            Grid {
                columns: 7
                spacing: 0

                Repeater {
                    model: root.cells

                    delegate: Item {
                        width: 36
                        height: 26

                        Rectangle {
                            anchors.centerIn: parent
                            width: 26
                            height: 22
                            radius: 5
                            color: modelData.isToday ? root.accentColor : "transparent"
                        }

                        Text {
                            anchors.centerIn: parent
                            text: modelData.day
                            color: modelData.isToday ? "#ffffff"
                                                     : (modelData.inMonth ? root.labelColor : root.mutedColor)
                            font.pixelSize: 12
                            font.weight: modelData.isToday ? Font.DemiBold : Font.Normal
                        }
                    }
                }
            }
        }
    }
}
