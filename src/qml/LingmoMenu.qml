// SPDX-FileCopyrightText: 2026 Lingmo OS Team
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import org.deepin.dtk 1.0 as D

import Lingmo.StatusBar 1.0

// Leftmost item of the bar: the Lingmo menu, the Apple menu of the macOS layout.
//
// The button shows the Lingmo word mark. This repository ships no icon resource
// (src/qml/qml.qrc holds QML files only) and the bar must not depend on one
// particular icon theme, so a text label is used instead of an icon. The Lingmo
// icon themes do carry a mark, should the button ever need a glyph: the icon name
// is "lingmoui" (real file: /usr/share/icons/Crule/apps/scalable/lingmoui.svg,
// package lingmo-systemicons), usable here as
//
//   D.DciIcon {
//       name: "lingmoui"
//       fallbackToQIcon: true
//   }
//
// The entries only log in phase 1, exactly like SystemMenu.qml: see
// SystemMenuController::trigger() in src/systemmenucontroller.cpp.
Item {
    id: root

    property color textColor: "#1b1b1b"
    property bool darkTheme: false

    implicitWidth: label.implicitWidth + 14
    implicitHeight: 24

    Rectangle {
        anchors.fill: parent
        radius: 6
        color: root.darkTheme ? Qt.rgba(1, 1, 1, 0.10) : Qt.rgba(0, 0, 0, 0.06)
        visible: mouseArea.containsMouse || lingmoMenu.opened
    }

    Text {
        id: label

        anchors.centerIn: parent
        text: qsTr("Lingmo")
        color: root.textColor
        font.pixelSize: 13
        font.weight: Font.DemiBold
    }

    MouseArea {
        id: mouseArea

        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: {
            if (lingmoMenu.opened) {
                lingmoMenu.close();
            } else {
                lingmoMenu.open();
            }
        }
    }

    // Same DTK menu as SystemMenu.qml. The bar window is only as tall as the bar
    // itself, so the menu is its own window (popupType: Popup.Window); x/y are
    // relative to the button, so the menu sits under it wherever the button moves.
    D.Menu {
        id: lingmoMenu

        parent: root
        popupType: Popup.Window
        modal: false
        x: 0
        y: root.height + 6

        // TODO(phase 2): "about" opens the control center (dde-control-center),
        // "settings" opens it as well, "logout"/"shutdown" go through the session
        // manager. Phase 1 logs only, the same as the system menu.
        D.MenuItem {
            text: qsTr("About This PC")
            onTriggered: SystemMenuController.trigger("about")
        }

        D.MenuItem {
            text: qsTr("Settings")
            onTriggered: SystemMenuController.trigger("settings")
        }

        D.MenuSeparator {}

        D.MenuItem {
            text: qsTr("Log Out")
            onTriggered: SystemMenuController.trigger("logout")
        }

        D.MenuItem {
            text: qsTr("Shut Down")
            onTriggered: SystemMenuController.trigger("shutdown")
        }
    }
}
