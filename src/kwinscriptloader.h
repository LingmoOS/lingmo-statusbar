// SPDX-FileCopyrightText: 2026 Lingmo OS Team
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>

// Asks a running KWin to load the bundled active window tracker script.
//
// The script is a plain KWin JavaScript file installed at
//   /usr/share/kwin/scripts/lingmo-statusbar-activewindow/contents/code/main.js
// and can also be enabled permanently through the KWin script KCM.
//
// Confirmed D-Bus API (lingmo-kwin):
//   * org.kde.KWin /Scripting org.kde.kwin.Scripting.loadScript(filePath, pluginName)
//     returns the script id, or -1 when the plugin name is already loaded
//     (lingmo-kwin/src/scripting/scripting.cpp:886-899, Q_CLASSINFO at
//     scripting.h:320),
//   * the script object is exported on "/<id>" with interface org.kde.kwin.Script
//     and method run() (lingmo-kwin/src/scripting/scripting.cpp:106 plus
//     scripting/org.kde.kwin.Script.xml).
class KWinScriptLoader
{
public:
    // Path of the installed tracker script, empty when it is not installed.
    static QString trackerScriptPath();

    // Loads and runs the tracker script; returns false and fills errorMessage on
    // failure (KWin not running, script not installed, script already loaded).
    static bool loadTrackerScript(QString *errorMessage = nullptr);
};
