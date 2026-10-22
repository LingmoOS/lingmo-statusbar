// SPDX-FileCopyrightText: 2026 Lingmo OS Team
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QString>

// Actions of the right-hand system menu.
//
// Phase 1 only logs: nothing is executed. Log out and shut down are deliberately
// not wired to anything yet (they need a session manager / logind policy check,
// which is a phase-2 item).
class SystemMenuController : public QObject
{
    Q_OBJECT

public:
    explicit SystemMenuController(QObject *parent = nullptr);

    // action is one of "settings", "logout", "shutdown".
    Q_INVOKABLE void trigger(const QString &action);

Q_SIGNALS:
    void triggered(const QString &action);
};
