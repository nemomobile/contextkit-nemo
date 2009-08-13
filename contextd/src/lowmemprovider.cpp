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

#include <ContextProvider>

#include "lowmemprovider.h"
#include "logging.h"
#include "loggingfeatures.h"
#include "sconnect.h"

namespace ContextD {

/*!
    \class LowMemProvider

    \brief Provider the System.MemoryPressure key.

    The LowMemProvider implements the System.MemoryPressure context property that has three possible
    values: normal, high, and critical.  It works by poll(2)ing the (Maemo specific)
    /sys/kernel/{low,high}_watermark sysfs attributes. The following table describes the correspondence
    between the attributes and the value of the context property:

    \code
    low_watermark | high_watermark | System.MemoryPressure
    --------------+----------------+-------------------------
          0       |       0        |       normal (0)
          1       |       0        |       high (1)
          1       |       1        |       critical (2)
          x       |       x        |       unspecified (null)
    \endcode

    As a side note, in Maemo terms:
    * low_watermark means `background killing' is active
    * high_watermark is `low memory condition' (expect the oom killer soon)
    
    Note that these are advisory, and there are cases when the OOM killer strikes without
    high_watermark being 1.
*/

#define LOW_WATERMARK   "/sys/kernel/low_watermark"
#define HIGH_WATERMARK  "/sys/kernel/high_watermark"

/// Constructor.
LowMemProvider::LowMemProvider() : memoryPressure("System.MemoryPressure")
{
    contextDebug() << F_LOWMEM << "Initializing lowmem plugin";

    sconnect(&memoryPressure, SIGNAL(firstSubscriberAppeared(QString)),
             this, SLOT(onFirstSubscriberAppeared()));
    sconnect(&memoryPressure, SIGNAL(lastSubscriberDisappeared(QString)),
             this, SLOT(onLastSubscriberDisappeared()));
}

/// This is called when the first subscriber for System.MemoryPressure appears.
/// Here we create the watchers on the pool files and start observing.
void LowMemProvider::onFirstSubscriberAppeared()
{
    lowWM = new BoolSysFsPooler(LOW_WATERMARK);
    highWM = new BoolSysFsPooler(HIGH_WATERMARK);

    sconnect(lowWM, SIGNAL(stateChanged(TriState)), 
             this, SLOT(onWatermarkStateChanged()));
    sconnect(highWM, SIGNAL(stateChanged(TriState)), 
             this, SLOT(onWatermarkStateChanged()));

    onWatermarkStateChanged();
}

/// This is called when the last subscriber for System.MemoryPressure disappears.
/// We stop observing the pool files and free the resources.
void LowMemProvider::onLastSubscriberDisappeared()
{
    delete lowWM;
    lowWM = NULL;
    delete highWM;
    highWM = NULL;
}

/// Slot called by the pools when one of them changes state. We get 
/// the current state of the pools and, based on that, we compute 
/// the current value for the property (normal, high, critical).
void LowMemProvider::onWatermarkStateChanged()
{
    contextDebug() << F_LOWMEM << "Watermarks state changed";

    BoolSysFsPooler::TriState lowWMState = lowWM->getState();
    BoolSysFsPooler::TriState highWMState = highWM->getState();

    if (lowWMState == BoolSysFsPooler::TriStateFalse && 
        highWMState == BoolSysFsPooler::TriStateFalse) {
        // normal
        memoryPressure.setValue(0);
    } else if (lowWMState == BoolSysFsPooler::TriStateTrue && 
               highWMState == BoolSysFsPooler::TriStateFalse) {
        // high
        memoryPressure.setValue(1);
    } else if (lowWMState == BoolSysFsPooler::TriStateTrue && 
               highWMState == BoolSysFsPooler::TriStateTrue) {
        // critical
        memoryPressure.setValue(2);
    } else if (lowWMState == BoolSysFsPooler::TriStateFalse && 
               highWMState == BoolSysFsPooler::TriStateTrue) {
        // ignore rogue state
        return;
    } else {
        // undefined */
        memoryPressure.unsetValue();
        return;
    }
}

}
