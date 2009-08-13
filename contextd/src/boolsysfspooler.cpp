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

#include "sconnect.h"
#include "boolsysfspooler.h"
#include "logging.h"
#include "loggingfeatures.h"

namespace ContextD {

/*!
    \class BoolSysFsPooler

    \brief Watches for changes in content of a sysfs attribute.
    
    The values of the sysfs attribute are {"0", "1"}. The BoolSysfsPoller emits the 
    stateChanged signal whenever the attribute value changes.

*/

/// Contructor. Will watch the specified \a fname.
BoolSysFsPooler::BoolSysFsPooler(const QString &fname) :
    input(fname), state(TriStateUnknown)
{
    contextDebug() << F_LOWMEM << "Opening" << fname << "in read mode.";

    if (! input.open(QIODevice::ReadOnly | QIODevice::Unbuffered)) {
        contextWarning() << F_LOWMEM << "Failed to open" << fname << "for reading.";
        return;
    }
 
    watcher.addPath(fname);
    sconnect(&watcher, SIGNAL(fileChanged(QString)), this, SLOT(onFileChanged()));
    readState();
}

/// Gets called when the watcher notices a file change.
void BoolSysFsPooler::onFileChanged()
{
    contextDebug() << F_LOWMEM << "New data on" << input.fileName();
    readState();
}

/// Reads the current state from the observed file (0, 1 or Unknown). Emits 
/// the stateChanged if the new state is different then the previous one.
void BoolSysFsPooler::readState()
{
    contextDebug() << F_LOWMEM << "Reading" << input.fileName() << "state";
    char data[32];
    qint64 bytesRead = input.read(data, 32);
    TriState newState;

    if (bytesRead > 0 && data[0] == '0')
        newState = TriStateFalse;
    else if (bytesRead > 0 && data[0] == '1')
        newState = TriStateTrue;
    else
        newState = TriStateUnknown;

    if (newState != state) {
        state = newState;
        emit stateChanged(newState);
    }

    input.reset();
}

/// Returns the current state.
BoolSysFsPooler::TriState BoolSysFsPooler::getState()
{
    return state;
}

}
