// SPDX-FileCopyrightText: 2026 Lingmo OS Team
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QRect>

class QScreen;

// Geometry of the status bar window: full screen width, fixed height, pinned to
// the top edge of the screen.
//
// Phase 1 follows QGuiApplication::primaryScreen() only; multi-screen support is
// a phase-2 item, see the TODO in ScreenGeometry::updateGeometry().
class ScreenGeometry : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int barX READ barX NOTIFY geometryChanged)
    Q_PROPERTY(int barY READ barY NOTIFY geometryChanged)
    Q_PROPERTY(int barWidth READ barWidth NOTIFY geometryChanged)
    Q_PROPERTY(int barHeight READ barHeight NOTIFY geometryChanged)

public:
    explicit ScreenGeometry(int barHeight, QObject *parent = nullptr);

    int barX() const;
    int barY() const;
    int barWidth() const;
    int barHeight() const;

Q_SIGNALS:
    // Emitted whenever the rectangle of the bar changes: screen hotplug,
    // resolution change or a change of the primary screen.
    void geometryChanged();

private:
    void rebindPrimaryScreen();
    void updateGeometry();

    QScreen *m_screen = nullptr;
    QRect m_rect;
    int m_barHeight;
};
