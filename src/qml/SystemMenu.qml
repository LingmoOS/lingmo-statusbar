// SPDX-FileCopyrightText: 2026 Lingmo OS Team
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import org.deepin.dtk 1.0 as D

import Lingmo.StatusBar 1.0

// System menu of the right-hand button.
//
// The entries only log in phase 1: SystemMenuController::trigger() writes a line
// and performs no action, see src/systemmenucontroller.cpp.
D.Menu {
    id: root

    property color textColor: "#1b1b1b"
    property bool darkTheme: false

    // The bar window itself is 36 px tall, so the menu is its own window.
    popupType: Popup.Window
    modal: false

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
