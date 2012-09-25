/*
 * Copyright (C) 2008, 2009 Nokia Corporation.
 *
 * Contact: Marius Vollmer <marius.vollmer@nokia.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#ifndef SCONNECT_H
#define SCONNECT_H

#include <QObject>
#include <QDebug>

inline void sconnect(const QObject *from, const char* fromSignal,
                     const QObject *to, const char* toSignal, Qt::ConnectionType type = Qt::AutoConnection)
{
    if (!QObject::connect(from, fromSignal, to, toSignal, type))
        qFatal("    *****************\n"
               "Connect returned false, aborting, enable core dumping (ulimit -c unlimited), \n"
               "enable debug (qmake CONFIG+=debug), recompile, rerun and then use the\n"
               "core file with gdb's backtrace to see the location.\n"
               "    *****************\n");
}

#endif
