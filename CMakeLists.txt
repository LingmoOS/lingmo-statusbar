# SPDX-FileCopyrightText: 2026 Lingmo OS Team
#
# SPDX-License-Identifier: CC0-1.0

cmake_minimum_required(VERSION 3.18)

project(lingmo-statusbar
    VERSION "0.2.0"
    DESCRIPTION "macOS-style top status bar for Lingmo OS"
    HOMEPAGE_URL "https://github.com/LingmoOS/lingmo-statusbar"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(QT_NS Qt6)
set(DTK_NS Dtk6)

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()
include(GNUInstallDirs)

find_package(${QT_NS} REQUIRED COMPONENTS Core Gui Widgets Quick Qml DBus)

# The DTK6 umbrella package resolves every component "X" to the separate package
# Dtk6X (dtkcommon/misc/config.cmake.in), so the widget module is "Widget"
# (libdtk6widget-dev, target Dtk6::Widget), not "Widgets".
find_package(${DTK_NS} REQUIRED COMPONENTS Core Gui Widget)

# The global menu reads the active window's X11 properties with libxcb directly,
# the same two atoms lingmo-kwin reads (lingmo-kwin/src/atoms.cpp:64-65).
find_package(PkgConfig REQUIRED)
pkg_check_modules(XCB REQUIRED xcb)

# The bar is 36 px tall; the window is as wide as the screen it is pinned to.
set(STATUSBAR_BAR_HEIGHT 36)
set(STATUSBAR_DBUS_SERVICE "org.lingmo.StatusBar")
set(STATUSBAR_DBUS_PATH "/org/lingmo/StatusBar")

# Exported D-Bus interface of the bar. The KWin helper script pushes the active
# window over this interface; see dbus/org.lingmo.StatusBar.xml.
qt_add_dbus_adaptor(STATUSBAR_DBUS_ADAPTOR
    dbus/org.lingmo.StatusBar.xml
    src/activewindowwatcher.h
    ActiveWindowWatcher
)

qt_add_resources(STATUSBAR_QML_RESOURCES src/qml/qml.qrc)

set(STATUSBAR_SOURCES
    src/main.cpp
    src/activewindowwatcher.h
    src/activewindowwatcher.cpp
    src/appmenu/appmenutracker.h
    src/appmenu/appmenutracker.cpp
    src/appmenu/dbusmenuclient.h
    src/appmenu/dbusmenuclient.cpp
    src/kwinscriptloader.h
    src/kwinscriptloader.cpp
    src/screengeometry.h
    src/screengeometry.cpp
    src/systemmenucontroller.h
    src/systemmenucontroller.cpp
    src/traybridge.h
    src/traybridge.cpp
    ${STATUSBAR_DBUS_ADAPTOR}
    ${STATUSBAR_QML_RESOURCES}
)

add_executable(${PROJECT_NAME} ${STATUSBAR_SOURCES})

target_compile_definitions(${PROJECT_NAME} PRIVATE
    STATUSBAR_VERSION="${PROJECT_VERSION}"
    STATUSBAR_BAR_HEIGHT=${STATUSBAR_BAR_HEIGHT}
    STATUSBAR_DBUS_SERVICE="${STATUSBAR_DBUS_SERVICE}"
    STATUSBAR_DBUS_PATH="${STATUSBAR_DBUS_PATH}"
)

target_include_directories(${PROJECT_NAME} PRIVATE ${XCB_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${QT_NS}::Core
    ${QT_NS}::Gui
    ${QT_NS}::Widgets
    ${QT_NS}::Quick
    ${QT_NS}::Qml
    ${QT_NS}::DBus
    ${DTK_NS}::Core
    ${DTK_NS}::Gui
    ${DTK_NS}::Widget
    ${XCB_LIBRARIES}
)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})

# KWin helper script: pushes workspace.activeClient data to org.lingmo.StatusBar.
install(DIRECTORY data/kwin/lingmo-statusbar-activewindow
    DESTINATION ${CMAKE_INSTALL_DATADIR}/kwin/scripts
)

# Session autostart entry; a status bar is not something you launch by hand.
# CMAKE_INSTALL_FULL_SYSCONFDIR resolves to /etc for the /usr prefix, which is
# where autostart entries belong (and where debian/install expects them).
install(FILES data/lingmo-statusbar.desktop
    DESTINATION ${CMAKE_INSTALL_FULL_SYSCONFDIR}/xdg/autostart
)
