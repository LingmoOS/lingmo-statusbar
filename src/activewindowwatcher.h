// SPDX-FileCopyrightText: 2026 Lingmo OS Team
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QString>

// Active window of the session, as far as the status bar can see it.
//
// KWin has no D-Bus getter for the active window (org.kde.KWin only offers
// getWindowInfo(uuid) and the interactive queryWindowInfo()). The scripting API
// does expose it:
//
//   workspace.activeClient            lingmo-kwin/src/scripting/workspace_wrapper.h:37
//   notification clientActivated      same Q_PROPERTY, NOTIFY clientActivated
//   W::caption                        lingmo-kwin/src/window.h:469
//   W::resourceClass                  lingmo-kwin/src/window.h:162
//   W::desktopFileName                lingmo-kwin/src/window.h:586
//
// The bundled KWin script (data/kwin/lingmo-statusbar-activewindow) pushes those
// three values to SetActiveWindow() below, which is exported on the session bus
// as org.lingmo.StatusBar (see dbus/org.lingmo.StatusBar.xml). If nothing pushes
// data, appName() returns the configurable fallback name ("Lingmo OS").
class ActiveWindowWatcher : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString appName READ appName NOTIFY changed)
    Q_PROPERTY(QString caption READ caption NOTIFY changed)
    Q_PROPERTY(QString resourceClass READ resourceClass NOTIFY changed)
    Q_PROPERTY(QString desktopEntry READ desktopEntry NOTIFY changed)
    Q_PROPERTY(QString fallbackAppName READ fallbackAppName WRITE setFallbackAppName NOTIFY changed)

public:
    explicit ActiveWindowWatcher(QObject *parent = nullptr);

    // Application name for the left section: desktop file stem, else the resource
    // class, else fallbackAppName().
    QString appName() const;
    QString caption() const;
    QString resourceClass() const;
    QString desktopEntry() const;

    // Shown when no active window information is available. Defaults to the value
    // of $LINGMO_STATUSBAR_FALLBACK_APP, or "Lingmo OS".
    QString fallbackAppName() const;
    void setFallbackAppName(const QString &fallbackAppName);

public Q_SLOTS:
    // Exported over D-Bus and called by the KWin helper script.
    void SetActiveWindow(const QString &caption, const QString &resourceClass, const QString &desktopEntry);
    void ClearActiveWindow();

Q_SIGNALS:
    void changed();
    void ActiveWindowChanged(const QString &caption, const QString &resourceClass, const QString &desktopEntry);

private:
    QString m_caption;
    QString m_resourceClass;
    QString m_desktopEntry;
    QString m_fallbackAppName;
};
