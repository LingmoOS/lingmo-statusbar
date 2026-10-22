// SPDX-FileCopyrightText: 2026 Lingmo OS Team
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

#include <xcb/xcb.h>

class QSocketNotifier;
class DbusMenuClient;

// Feeds DbusMenuClient with the menu the active window advertises.
//
// A window does not expose its menu through KWin: the Qt platform theme sets two
// X11 properties on the window that owns the menu, and the compositor reads them
// back when it needs to show the appmenu button
// (lingmo-kwin/src/x11window.h:378 showRequest,
//  lingmo-kwin/src/atoms.cpp:64-65 _KDE_NET_WM_APPMENU_SERVICE_NAME/_OBJECT_PATH,
//  lingmo-qt-plugins/platformtheme/platformtheme.cpp:27-28 set the same atoms).
//
// The retired Lingmo status bar read exactly these two properties off
// KX11Extras::activeWindow() (its src/appmenu/appmenumodel.cpp:170-171). This
// class does the equivalent through xcb directly, so the bar does not need the
// KF6WindowSystem dependency, and follows _NET_ACTIVE_WINDOW instead of relying on
// a toolkit helper.
//
// X11 only by nature: without an X display (Wayland session without XWayland) the
// tracker reports isSupported() == false and the bar shows no global menu.
class AppMenuTracker : public QObject
{
    Q_OBJECT

public:
    explicit AppMenuTracker(DbusMenuClient *client, QObject *parent = nullptr);
    ~AppMenuTracker() override;

    bool isSupported() const;

private:
    void processX11Events();
    void trackActiveWindow();
    void refreshMenuProperties();
    QByteArray readStringProperty(xcb_window_t window, xcb_atom_t atom) const;

    DbusMenuClient *m_client = nullptr;

    xcb_connection_t *m_connection = nullptr;
    QSocketNotifier *m_notifier = nullptr;
    xcb_window_t m_root = XCB_WINDOW_NONE;
    xcb_atom_t m_activeWindowAtom = XCB_ATOM_NONE;
    xcb_atom_t m_serviceNameAtom = XCB_ATOM_NONE;
    xcb_atom_t m_objectPathAtom = XCB_ATOM_NONE;
    xcb_window_t m_activeWindow = XCB_WINDOW_NONE;
};
