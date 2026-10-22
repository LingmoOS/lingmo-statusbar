// SPDX-FileCopyrightText: 2026 Lingmo OS Team
//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// KWin helper script: pushes the active window to lingmo-statusbar.
//
// Confirmed against this repository's KWin (lingmo-kwin):
//   workspace.activeClient            src/scripting/workspace_wrapper.h:37
//   workspace.clientActivated signal  same Q_PROPERTY, NOTIFY clientActivated
//   window.caption                    src/window.h:469
//   window.resourceClass              src/window.h:162
//   window.desktopFileName            src/window.h:586
//   callDBus(...)                     src/scripting/scripting.cpp:302,
//                                     src/scripting/documentation-global.xml:128
//
// The receiving side is lingmo-statusbar's org.lingmo.StatusBar interface,
// see dbus/org.lingmo.StatusBar.xml.

var serviceName = "org.lingmo.StatusBar";
var objectPath = "/org/lingmo/StatusBar";
var interfaceName = "org.lingmo.StatusBar";

var trackedClient = null;

function pushActiveWindow(client) {
    if (!client) {
        callDBus(serviceName, objectPath, interfaceName, "ClearActiveWindow");
        return;
    }

    callDBus(serviceName, objectPath, interfaceName, "SetActiveWindow",
             client.caption, client.resourceClass, client.desktopFileName);
}

function onCaptionChanged() {
    pushActiveWindow(trackedClient);
}

function onClientActivated(client) {
    if (trackedClient) {
        try {
            trackedClient.captionChanged.disconnect(onCaptionChanged);
        } catch (e) {
            // The previous client may already be gone; nothing to disconnect.
        }
    }

    trackedClient = client ? client : null;
    if (trackedClient) {
        trackedClient.captionChanged.connect(onCaptionChanged);
    }

    pushActiveWindow(trackedClient);
}

onClientActivated(workspace.activeClient);
workspace.clientActivated.connect(onClientActivated);
