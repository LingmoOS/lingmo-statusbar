// SPDX-FileCopyrightText: 2026 Lingmo OS Team
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "activewindowwatcher.h"
#include "appmenu/appmenutracker.h"
#include "appmenu/dbusmenuclient.h"
#include "kwinscriptloader.h"
#include "screengeometry.h"
#include "systemmenucontroller.h"
#include "traybridge.h"

#include <DApplication>
#include <DGuiApplicationHelper>
#include <DLog>

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDBusConnection>
#include <QDebug>
#include <QQmlApplicationEngine>
#include <QQmlEngine>
#include <QUrl>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE
DCORE_USE_NAMESPACE

namespace {

// One QML module for the objects the bar needs; imported as
// "import Lingmo.StatusBar 1.0" by the QML files in src/qml.
constexpr auto QmlModuleUri = "Lingmo.StatusBar";
constexpr int QmlModuleMajorVersion = 1;
constexpr int QmlModuleMinorVersion = 0;

} // namespace

int main(int argc, char *argv[])
{
    DApplication app(argc, argv);
    app.setOrganizationName(QStringLiteral("Lingmo"));
    app.setApplicationName(QStringLiteral("lingmo-statusbar"));
    app.setApplicationDisplayName(QStringLiteral("Lingmo Status Bar"));
    app.setApplicationVersion(QStringLiteral(STATUSBAR_VERSION));
    // The bar is a panel: closing/hiding its window must not end the session.
    app.setQuitOnLastWindowClosed(false);

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("macOS-style top status bar for Lingmo OS"));
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption kwinScriptOption(
        QStringList { QStringLiteral("k"), QStringLiteral("enable-kwin-script") },
        QStringLiteral("Ask KWin to load the bundled active window tracker script."));
    parser.addOption(kwinScriptOption);
    parser.process(app);

    if (!DGuiApplicationHelper::setSingleInstance(QString::fromLatin1(STATUSBAR_DBUS_SERVICE))) {
        qInfo() << "lingmo-statusbar is already running";
        return 0;
    }

    DLogManager::registerConsoleAppender();

    ActiveWindowWatcher activeWindowWatcher;
    TrayBridge trayBridge;
    SystemMenuController systemMenuController;
    ScreenGeometry screenGeometry(STATUSBAR_BAR_HEIGHT);

    // Global menu of the active window (phase 2). AppMenuTracker reads the X11
    // properties the active window uses to advertise its com.canonical.dbusmenu
    // object; without an X connection it reports itself unsupported and the bar
    // simply shows no menu titles.
    DbusMenuClient appMenuClient;
    AppMenuTracker appMenuTracker(&appMenuClient);
    if (!appMenuTracker.isSupported()) {
        qInfo() << "no usable X11 display: the global menu stays empty";
    }

    // Export org.lingmo.StatusBar so the KWin helper script can push the active
    // window. Losing the name is not fatal: the bar keeps showing the fallback
    // application name.
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.registerService(QString::fromLatin1(STATUSBAR_DBUS_SERVICE))
        || !bus.registerObject(QString::fromLatin1(STATUSBAR_DBUS_PATH), &activeWindowWatcher)) {
        qWarning() << "failed to export" << STATUSBAR_DBUS_SERVICE << "on"
                   << STATUSBAR_DBUS_PATH << "- active window tracking is unavailable";
    }

    qmlRegisterSingletonInstance(QmlModuleUri, QmlModuleMajorVersion, QmlModuleMinorVersion,
                                 "ActiveWindowWatcher", &activeWindowWatcher);
    qmlRegisterSingletonInstance(QmlModuleUri, QmlModuleMajorVersion, QmlModuleMinorVersion,
                                 "TrayBridge", &trayBridge);
    qmlRegisterSingletonInstance(QmlModuleUri, QmlModuleMajorVersion, QmlModuleMinorVersion,
                                 "SystemMenuController", &systemMenuController);
    qmlRegisterSingletonInstance(QmlModuleUri, QmlModuleMajorVersion, QmlModuleMinorVersion,
                                 "ScreenGeometry", &screenGeometry);
    qmlRegisterSingletonInstance(QmlModuleUri, QmlModuleMajorVersion, QmlModuleMinorVersion,
                                 "AppMenu", &appMenuClient);

    QQmlApplicationEngine engine;
#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        []() {
            QCoreApplication::exit(1);
        },
        Qt::QueuedConnection);
#endif
    engine.load(QUrl(QStringLiteral("qrc:/qml/StatusBar.qml")));
    if (engine.rootObjects().isEmpty()) {
        qCritical() << "failed to load qrc:/qml/StatusBar.qml";
        return 1;
    }

    if (parser.isSet(kwinScriptOption)) {
        QString error;
        if (KWinScriptLoader::loadTrackerScript(&error)) {
            qInfo() << "loaded the KWin active window tracker script";
        } else {
            qWarning().noquote() << "could not load the KWin tracker script:" << error;
        }
    }

    return app.exec();
}
