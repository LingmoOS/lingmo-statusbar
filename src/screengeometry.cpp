// SPDX-FileCopyrightText: 2026 Lingmo OS Team
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "screengeometry.h"

#include <QGuiApplication>
#include <QScreen>

ScreenGeometry::ScreenGeometry(int barHeight, QObject *parent)
    : QObject(parent)
    , m_barHeight(barHeight)
{
    // Every signal that can move the bar ends up in rebindPrimaryScreen().
    connect(qGuiApp, &QGuiApplication::primaryScreenChanged, this, [this](QScreen *) {
        rebindPrimaryScreen();
    });
    connect(qGuiApp, &QGuiApplication::screenAdded, this, [this](QScreen *) {
        rebindPrimaryScreen();
    });
    connect(qGuiApp, &QGuiApplication::screenRemoved, this, [this](QScreen *) {
        rebindPrimaryScreen();
    });

    rebindPrimaryScreen();
}

int ScreenGeometry::barX() const
{
    return m_rect.x();
}

int ScreenGeometry::barY() const
{
    return m_rect.y();
}

int ScreenGeometry::barWidth() const
{
    return m_rect.width();
}

int ScreenGeometry::barHeight() const
{
    return m_rect.height();
}

void ScreenGeometry::rebindPrimaryScreen()
{
    QScreen *screen = QGuiApplication::primaryScreen();
    if (m_screen != screen) {
        if (m_screen) {
            disconnect(m_screen, nullptr, this, nullptr);
        }
        m_screen = screen;
        if (m_screen) {
            connect(m_screen, &QScreen::geometryChanged, this, &ScreenGeometry::updateGeometry);
            connect(m_screen, &QScreen::availableGeometryChanged, this, &ScreenGeometry::updateGeometry);
        }
    }
    updateGeometry();
}

void ScreenGeometry::updateGeometry()
{
    const QRect screenRect = m_screen ? m_screen->geometry() : QRect();

    // TODO(phase 2): multi-screen. Either create one bar per QScreen, or move the
    // single bar to the screen that holds the active window. The bar also does not
    // reserve its area yet (_NET_WM_STRUT_PARTIAL on X11, wlr-layer-shell on
    // Wayland), so maximized windows will overlap it.
    const QRect rect(screenRect.x(), screenRect.y(), screenRect.width(), m_barHeight);
    if (rect == m_rect) {
        return;
    }

    m_rect = rect;
    Q_EMIT geometryChanged();
}
