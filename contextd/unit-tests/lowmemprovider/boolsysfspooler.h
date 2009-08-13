/*
 * Copyright (C) 2008 Nokia Corporation.
 *
 * Contact: Marius Vollmer <marius.vollmer@nokia.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#ifndef BOOLSYSFSPOOLER_H
#define BOOLSYSFSPOOLER_H

#include <QVariant>
#include <QObject>

class BoolSysFsPooler : public QObject
{
    Q_OBJECT
    
public:
    enum TriState { TriStateTrue, TriStateFalse, TriStateUnknown };
    BoolSysFsPooler(const QString &fname);
    TriState getState();
    void setState(TriState s);

private:
    TriState state;

signals:
    void stateChanged(TriState newState);
};

#endif // BOOLSYSFSPOOLER_H
