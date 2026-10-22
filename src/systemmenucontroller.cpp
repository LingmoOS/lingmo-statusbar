// SPDX-FileCopyrightText: 2026 Lingmo OS Team
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "systemmenucontroller.h"

#include <QDebug>

SystemMenuController::SystemMenuController(QObject *parent)
    : QObject(parent)
{
}

void SystemMenuController::trigger(const QString &action)
{
    // TODO(phase 2): "settings" opens the control center, "logout"/"shutdown" go
    // through the session manager instead of being logged.
    qInfo().noquote() << "[system-menu]" << action << "- phase 1: logged only, nothing is executed";

    Q_EMIT triggered(action);
}
