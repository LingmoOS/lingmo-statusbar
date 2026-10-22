// SPDX-FileCopyrightText: 2026 Lingmo OS Team
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls

import Lingmo.StatusBar 1.0

// Fly-out of one top level entry of the active window's menu bar.
//
// The bar window is 36 px tall, so the popup is its own window. It shows the
// children of `menuId` in the first column and, when an entry has a submenu, its
// children in a second column next to it. Deeper nesting drills into the second
// column (there is no third column yet).
//
// Data: AppMenu (DbusMenuClient) requests GetLayout/AboutToShow from the window's
// com.canonical.dbusmenu object, see src/appmenu/dbusmenuclient.cpp.
Popup {
    id: root

    property int menuId: 0
    property color textColor: "#1b1b1b"
    property bool darkTheme: false

    property var entries: []
    property var submenuEntries: []
    property int submenuId: 0
    property int submenuParentId: 0

    readonly property int columnWidth: 208
    readonly property bool flyoutVisible: root.submenuId !== 0

    popupType: Popup.Window
    modal: false
    padding: 0
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    width: root.flyoutVisible ? root.columnWidth * 2 + 12 : root.columnWidth + 12
    height: Math.max(firstColumn.implicitHeight, flyoutColumn.implicitHeight) + 12

    onOpened: root.reload()
    onClosed: {
        root.submenuId = 0;
        root.submenuParentId = 0;
        root.submenuEntries = [];
    }

    function reload() {
        AppMenu.aboutToShow(root.menuId);
        root.entries = AppMenu.childrenOf(root.menuId);
        root.submenuId = 0;
        root.submenuParentId = 0;
        root.submenuEntries = [];
    }

    function openSubmenu(id) {
        if (id === root.submenuId) {
            return;
        }
        AppMenu.aboutToShow(id);
        root.submenuId = id;
        root.submenuEntries = AppMenu.childrenOf(id);
    }

    background: Rectangle {
        radius: 8
        color: root.darkTheme ? Qt.rgba(0.16, 0.16, 0.17, 0.98) : Qt.rgba(1, 1, 1, 0.98)
        border.width: 1
        border.color: root.darkTheme ? Qt.rgba(1, 1, 1, 0.12) : Qt.rgba(0, 0, 0, 0.08)
    }

    contentItem: Row {
        anchors.fill: parent
        anchors.margins: 6
        spacing: 0

        Column {
            id: firstColumn

            width: root.columnWidth

            Repeater {
                model: root.entries

                delegate: AppMenuItemRow {
                    width: firstColumn.width
                    text: modelData.text
                    separator: modelData.separator
                    itemEnabled: modelData.enabled
                    hasSubmenu: modelData.hasSubmenu
                    checked: modelData.checked
                    highlighted: root.flyoutVisible && root.submenuParentId === modelData.id
                    textColor: root.textColor
                    darkTheme: root.darkTheme

                    onActivated: {
                        AppMenu.activate(modelData.id);
                        root.close();
                    }
                    onSubmenuRequested: {
                        root.submenuParentId = modelData.id;
                        root.openSubmenu(modelData.id);
                    }
                }
            }
        }

        Column {
            id: flyoutColumn

            width: root.columnWidth
            visible: root.flyoutVisible

            Repeater {
                model: root.submenuEntries

                delegate: AppMenuItemRow {
                    width: flyoutColumn.width
                    text: modelData.text
                    separator: modelData.separator
                    itemEnabled: modelData.enabled
                    hasSubmenu: modelData.hasSubmenu
                    checked: modelData.checked
                    textColor: root.textColor
                    darkTheme: root.darkTheme

                    onActivated: {
                        AppMenu.activate(modelData.id);
                        root.close();
                    }
                    onSubmenuRequested: {
                        // TODO(phase 2): a third column for deeper menus.
                        root.openSubmenu(modelData.id);
                    }
                }
            }
        }
    }
}
