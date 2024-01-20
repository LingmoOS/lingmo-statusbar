/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 * Author:     lingmoos <lingmo@lingmo.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CONTROLCENTERDIALOG_H
#define CONTROLCENTERDIALOG_H

#include <QQuickWindow>
#include <QTimer>

class ControlCenterDialog : public QQuickWindow
{
    Q_OBJECT

public:
    ControlCenterDialog(QQuickWindow *view = nullptr);

    Q_INVOKABLE void open();

protected:
    bool eventFilter(QObject *object, QEvent *event);
};

#endif // CONTROLCENTERDIALOG_H