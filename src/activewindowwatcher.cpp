// SPDX-FileCopyrightText: 2026 Lingmo OS Team
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "activewindowwatcher.h"

#include <QDebug>
#include <QFileInfo>

#include "statusbaradaptor.h"

namespace {

constexpr auto DefaultFallbackAppName = "Lingmo OS";
constexpr auto FallbackAppNameEnvVar = "LINGMO_STATUSBAR_FALLBACK_APP";
constexpr auto DesktopEntrySuffix = ".desktop";

} // namespace

ActiveWindowWatcher::ActiveWindowWatcher(QObject *parent)
    : QObject(parent)
    , m_fallbackAppName(QString::fromLocal8Bit(qgetenv(FallbackAppNameEnvVar)))
{
    if (m_fallbackAppName.isEmpty()) {
        m_fallbackAppName = QString::fromLatin1(DefaultFallbackAppName);
    }

    // Exports org.lingmo.StatusBar for this object. main() registers the object
    // itself on /org/lingmo/StatusBar with QDBusConnection::registerObject().
    new StatusBarAdaptor(this);
}

QString ActiveWindowWatcher::appName() const
{
    if (!m_desktopEntry.isEmpty()) {
        // desktopEntry is usually a plain file name ("org.kde.dolphin.desktop"),
        // but it may also be a full path. The stem is the closest thing to an
        // application name that the window manager exposes.
        const QString fileName = QFileInfo(m_desktopEntry).fileName();
        const QString suffix = QString::fromLatin1(DesktopEntrySuffix);
        return fileName.endsWith(suffix) ? fileName.chopped(suffix.size()) : fileName;
    }

    if (!m_resourceClass.isEmpty()) {
        return m_resourceClass;
    }

    return m_fallbackAppName;
}

QString ActiveWindowWatcher::caption() const
{
    return m_caption;
}

QString ActiveWindowWatcher::resourceClass() const
{
    return m_resourceClass;
}

QString ActiveWindowWatcher::desktopEntry() const
{
    return m_desktopEntry;
}

QString ActiveWindowWatcher::fallbackAppName() const
{
    return m_fallbackAppName;
}

void ActiveWindowWatcher::setFallbackAppName(const QString &fallbackAppName)
{
    const QString name = fallbackAppName.isEmpty() ? QString::fromLatin1(DefaultFallbackAppName)
                                                   : fallbackAppName;
    if (name == m_fallbackAppName) {
        return;
    }

    m_fallbackAppName = name;
    Q_EMIT changed();
}

void ActiveWindowWatcher::SetActiveWindow(const QString &caption,
                                          const QString &resourceClass,
                                          const QString &desktopEntry)
{
    if (caption == m_caption && resourceClass == m_resourceClass && desktopEntry == m_desktopEntry) {
        return;
    }

    m_caption = caption;
    m_resourceClass = resourceClass;
    m_desktopEntry = desktopEntry;

    Q_EMIT changed();
    Q_EMIT ActiveWindowChanged(m_caption, m_resourceClass, m_desktopEntry);
}

void ActiveWindowWatcher::ClearActiveWindow()
{
    SetActiveWindow(QString(), QString(), QString());
}
