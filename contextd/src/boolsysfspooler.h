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
#include <QStringList>
#include <QObject>
#include <QFile>
#include <QFileSystemWatcher>

namespace ContextD {

class BoolSysFsPooler : public QObject
{
    Q_OBJECT

public:
    /// Represents a state of the observed file (1, 0, ?)
    enum TriState { TriStateTrue, TriStateFalse, TriStateUnknown };

    BoolSysFsPooler(const QString &fname);
    TriState getState();

private:
    QFile input; ///< Input file.
    QFileSystemWatcher watcher; ///< Watcher that observes the file for changes.
    TriState state; ///< Current (last read) state.

    void readState();

private slots:
    void onFileChanged();

signals:
    /// Emitted automatically when the state changes. \a newState contains the new state.
    void stateChanged(TriState newState); 
};

}

#endif // BOOLSYSFSPOOLER_H
