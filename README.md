# lingmo-statusbar

macOS-style status bar for Lingmo OS: one full-width window at the top edge of the
screen, written with Qt 6 and DTK 6 (Qt Quick + `org.deepin.dtk`).

- Repository: <https://github.com/LingmoOS/lingmo-statusbar>
- License: `GPL-3.0-or-later`, full text in `LICENSE` and `LICENSES/GPL-3.0-or-later.txt`
- Status: phase 1 skeleton. The bar starts, draws and reacts to clicks; the global
  menu and the real system tray are not implemented yet.

```text
[ Lingmo ][ application name + reserved global menu ] ... [ tray ][ clock ][ menu ]
```

## Layout

| Path | Contents |
| --- | --- |
| `CMakeLists.txt` | Top-level build script: Qt 6 / Dtk 6 lookup, D-Bus adaptor, QML resources, install rules |
| `src/main.cpp` | `DApplication` setup, single instance, `org.lingmo.StatusBar` export, QML engine |
| `src/screengeometry.*` | Window rectangle of the bar (primary screen, full width, 36 px) |
| `src/activewindowwatcher.*` | Active window data received over D-Bus and exposed to QML |
| `src/traybridge.*` | Placeholder tray items; the real status notifier host is phase 2 |
| `src/systemmenucontroller.*` | Settings / log out / shut down entries; they only log |
| `src/kwinscriptloader.*` | `--enable-kwin-script`: asks KWin to load the bundled script |
| `src/qml/` | Bar window and its sections, compiled into `qrc:/qml/` |
| `dbus/org.lingmo.StatusBar.xml` | The interface the KWin helper script pushes to |
| `data/kwin/lingmo-statusbar-activewindow/` | KWin script package: `workspace.activeClient` → D-Bus |
| `data/lingmo-statusbar.desktop` | Autostart entry (`/etc/xdg/autostart`) |
| `translations/` | Empty in phase 1, see `translations/README.md` |
| `debian/` | Debian packaging |

## What phase 1 does

- One frameless, always-on-top window spanning the top edge of the primary screen,
  36 px tall (`STATUSBAR_BAR_HEIGHT` in `CMakeLists.txt`, used by
  `src/screengeometry.cpp`). Window flags are
  `Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool`
  (`src/qml/StatusBar.qml`), so the bar stays above normal windows and does not
  appear in the task bar or in the window switcher.
- Left, from the left edge: the Lingmo menu button, then the application name of
  the active window, pushed by the KWin script described below, then that window's
  menu titles. When nothing is pushed, the configurable fallback name is shown
  (default `Lingmo OS`, override with `$LINGMO_STATUSBAR_FALLBACK_APP`). The Lingmo
  menu is a `D.Menu` with About This PC, Settings, Log Out and Shut Down and only
  logs, like the system menu. The menu titles come from `com.canonical.dbusmenu`,
  see "Global menu" below.
- Middle: empty by design. The bar follows the macOS arrangement, not the GNOME
  one: the clock is not centred, and an `Item { Layout.fillWidth: true }` keeps
  the two groups at their edges.
- Right, from the right edge inwards: the system menu button, the clock, and the
  three placeholder tray icons (volume, network, battery). The clock is updated
  every second from a QML `Timer` and opens a month calendar popup on click; the
  calendar is display only. The tray icons are plain icon theme names looked up by
  `D.DciIcon`; clicking one writes a line to the log. The system menu opens a
  `D.Menu` with Settings, Log Out and Shut Down; each entry calls
  `SystemMenuController::trigger()`, which logs the action and executes nothing.

## What is still missing

- **Global menu on Wayland**: the menu is read from X11 window properties, so a
  Wayland session shows no menu titles. See "Global menu" below.
- **System tray**: no `org.kde.StatusNotifierWatcher`
  (legacy `com.canonical.StatusNotifierWatcher`) service, no
  `org.kde.StatusNotifierItem` hosting, no `com.canonical.dbusmenu` menus for tray
  items. `src/traybridge.h` documents the interfaces to implement in phase 2.
- **System menu actions**: Settings, Log Out and Shut Down only log.
- **Multi-screen**: the bar follows `QGuiApplication::primaryScreen()` only. There
  is one bar and it does not move to the screen that holds the active window; see
  the `TODO(phase 2)` in `ScreenGeometry::updateGeometry()`.
- **Reserved screen space**: the bar does not set `_NET_WM_STRUT_PARTIAL` (X11) and
  does not use `wlr-layer-shell` (Wayland), so maximized windows are covered by
  the bar.
- **Wayland**: `Qt.Tool` and `Qt.WindowStaysOnTopHint` are X11 window manager
  hints. On a Wayland session there is no xdg-shell equivalent, so KWin treats the
  bar as an ordinary toplevel window; the layer-shell integration
  (`LayerShellQt`) is a phase-2 item. On X11 the flags behave as described above.
- **Localisation**: no `.ts` files; the clock uses `Qt.formatDateTime()` with a
  fixed format string, which is not locale aware.

## Global menu

The menu titles of the active window are shown next to its name; clicking a title
opens its submenu and clicking an entry sends the click to the application.

A window does not publish its menu to the window manager. The Qt platform theme
exports the menu as a `com.canonical.dbusmenu` object and advertises the service
name and the object path in two X11 properties on the window:

| Piece | Evidence |
| --- | --- |
| `_KDE_NET_WM_APPMENU_SERVICE_NAME` / `_KDE_NET_WM_APPMENU_OBJECT_PATH` | set by `lingmo-qt-plugins/platformtheme/platformtheme.cpp:27-28`, read by `lingmo-kwin/src/atoms.cpp:64-65` and `lingmo-kwin/src/x11window.h:378` |
| `com.canonical.AppMenu.Registrar` (`RegisterWindow`) | `lingmo-qt-plugins/platformtheme/qdbusmenubar.cpp:130` |
| `org.kde.kappmenu` (`showMenu`, `showRequest`) | `lingmo-kwin/src/appmenu.cpp:28-30,98`, `lingmo-workspace/appmenu/org.kde.kappmenu.xml` |
| dbusmenu layout `(ia{sv}av)` | `lingmo-workspace/appmenu/com.canonical.AppMenu.Registrar.xml`; the demarshalling order follows the retired status bar's `src/libdbusmenuqt/dbusmenutypes_p.cpp:78-95` |

Data flow:

```text
active window (X11)
  _KDE_NET_WM_APPMENU_SERVICE_NAME / _KDE_NET_WM_APPMENU_OBJECT_PATH
    -> AppMenuTracker   (src/appmenu/appmenutracker.cpp, libxcb)
      -> DbusMenuClient (src/appmenu/dbusmenuclient.cpp)
           AboutToShow(0), GetLayout(0, -1, <properties>)
         -> QML: AppMenu.topLevelItems (src/qml/AppMenuBar.qml)
            click -> Event(id, "clicked") back to the application
```

Limits of this first phase-2 step: X11 only; one level of nested submenu; the
pre-click state is refreshed by `LayoutUpdated` and after every click, but
`ItemsPropertiesUpdated` is not consumed yet.

The client has a headless test that needs no display, no X11 and no DTK:

```bash
./test/dbusmenu/run.sh
```

It starts `test/dbusmenu/fake_menu.py` (a fake dbusmenu service) on the session bus
and checks both the parsed tree and the `Event` call the client sends back. It needs
`python3-dbus`, `python3-gi` and `moc` from `qt6-base-dev-tools`.

## Active window data

KWin has no D-Bus getter for the active window. `org.kde.KWin` on `/KWin` exposes
`getWindowInfo(uuid)` (needs a window id) and `queryWindowInfo()` (interactive,
the user has to click a window), see `lingmo-kwin/src/org.kde.KWin.xml` and
`lingmo-kwin/src/dbusinterface.cpp:220,243`. The active window is only available
through the KWin scripting API:

| API | Evidence |
| --- | --- |
| `workspace.activeClient` | `lingmo-kwin/src/scripting/workspace_wrapper.h:37` (`Q_PROPERTY`, `NOTIFY clientActivated`) |
| `workspace.clientActivated(client)` | same signal declaration |
| `window.caption` | `lingmo-kwin/src/window.h:469` |
| `window.resourceClass` | `lingmo-kwin/src/window.h:162` |
| `window.desktopFileName` | `lingmo-kwin/src/window.h:586` |
| `callDBus(service, path, iface, method, ...)` | `lingmo-kwin/src/scripting/scripting.cpp:302`, `documentation-global.xml:128` |

The data flow is:

```text
KWin script (data/kwin/lingmo-statusbar-activewindow/contents/code/main.js)
  workspace.activeClient / captionChanged
    → callDBus("org.lingmo.StatusBar", "/org/lingmo/StatusBar",
               "org.lingmo.StatusBar", "SetActiveWindow", caption,
               resourceClass, desktopFileName)
      → ActiveWindowWatcher::SetActiveWindow()   (src/activewindowwatcher.cpp)
        → QML: ActiveWindowWatcher.appName       (src/qml/AppMenuSection.qml)
```

`appName` is the desktop entry stem, falling back to `resourceClass`, then to the
fallback name.

The script is a normal KWin script package, installed to
`/usr/share/kwin/scripts/lingmo-statusbar-activewindow/` and disabled by default
(`EnabledByDefault: false` in its `metadata.json`). It can be started in two ways:

```bash
# 1. let the bar ask KWin to load it (runtime only, not persisted)
lingmo-statusbar --enable-kwin-script

# 2. or enable it permanently in KWin: System Settings → Window → KWin Scripts
```

The bar works without the script; the left section then shows the fallback name.

To check the receiving end without KWin, call the exported method directly and
watch the bar (the KWin script's argument order is caption, resource class,
desktop entry):

```bash
gdbus call --session \
  --dest org.lingmo.StatusBar \
  --object-path /org/lingmo/StatusBar \
  --method org.lingmo.StatusBar.SetActiveWindow \
  "Dolphin — /home" "dolphin" "org.kde.dolphin.desktop"
```

## Dependencies

`Build-Depends` in `debian/control`:

- `cmake` (top-level script requires 3.18), `debhelper-compat (= 13)`
- Qt 6: `qt6-base-dev` (Core, Gui, Widgets, DBus), `qt6-declarative-dev`
  (Quick, Qml)
- DTK 6: `libdtk6core-dev`, `libdtk6gui-dev`, `libdtk6widget-dev`; the QML types
  (`D.ApplicationWindow`, `D.DciIcon`, `D.Menu`) need `libdtk6declarative` at
  runtime

The CMake dependency lookup is:

```cmake
# The DTK6 umbrella package resolves component "X" to the separate package
# Dtk6X (dtkcommon/misc/config.cmake.in), so the widget module is "Widget"
# (target Dtk6::Widget), not "Widgets".
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Quick Qml DBus)
find_package(Dtk6 REQUIRED COMPONENTS Core Gui Widget)
```

## Build and install

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j"$(nproc)"
sudo cmake --install build
```

The install prefix defaults to `/usr` (`CMakeLists.txt`), and can be changed with
`-DCMAKE_INSTALL_PREFIX=...` or `DESTDIR`. Installed files:

- `/usr/bin/lingmo-statusbar`
- `/usr/share/kwin/scripts/lingmo-statusbar-activewindow/`
- `/etc/xdg/autostart/lingmo-statusbar.desktop`

## Run

```bash
lingmo-statusbar                # in a session with KWin running
lingmo-statusbar --enable-kwin-script
```

The process is a session singleton: a second instance exits immediately
(`DGuiApplicationHelper::setSingleInstance()` in `src/main.cpp`). Log output goes
to the console (`DLogManager::registerConsoleAppender()`), so the system menu and
tray placeholder clicks are visible when the bar is started from a terminal.

## Packaging

```bash
dpkg-buildpackage -b -uc -us
```

`debian/rules` must keep its executable bit; it is committed as mode `100755`.

## Phase 2

- Global menu: own `com.canonical.dbusmenu` client code, watch `org.kde.kappmenu`
  (`/MenuBar` on the session bus), render the active window's menu in the reserved
  slot of the left section, forward `Event`/`AboutToShow`.
- Tray: implement `org.kde.StatusNotifierWatcher` (legacy
  `com.canonical.StatusNotifierWatcher`) plus `org.kde.StatusNotifierItem` hosting
  and item menus, replacing the fixed list in `src/traybridge.cpp`.
- System menu: open the control center, log out and shut down through the session
  manager instead of logging.
- Multi-screen: one bar per screen, or follow the screen of the active window;
  reserve bar space with `_NET_WM_STRUT_PARTIAL` / `wlr-layer-shell`.
- Localisation: generate `.ts` files for the `qsTr()` strings and use a
  locale-aware clock format.
