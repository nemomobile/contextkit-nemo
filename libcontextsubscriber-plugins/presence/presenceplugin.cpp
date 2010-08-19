/*
 * Copyright (C) 2010 Nokia Corporation.
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


#include "presenceplugin.h"
#include "logging.h"
#include "sconnect.h"

IProviderPlugin* pluginFactory(const QString& /*constructionString*/)
{
    return new ContextSubscriberPresence::PresenceStatePlugin();
}

namespace ContextSubscriberPresence {

PresenceStatePlugin::PresenceStatePlugin(): presenceStateKey("Presence.State")
{
    // Connect to the global account change

    //TODO replace that line with right signal  
   // sconnect(&globalaccount, SIGNAL(globalAvailabilityChanged()), this, SLOT(emitValueChanged()));

}

void PresenceStatePlugin::subscribe(QSet<QString> keys)
{

    // Check for invalid keys
    foreach (const QString& key, keys) {
        if (key != presenceStateKey) {
            emit subscribeFailed(key, "Invalid key");
        }
    }

    if (keys.contains(presenceStateKey)) {

        // Now the value is there; signal that the subscription is done.
        emit subscribeFinished(presenceStateKey);
    }
}


void PresenceStatePlugin::unsubscribe(QSet<QString> keys)
{
}

/// Check the current status of the Session.State property and emit
/// the valueChanged signal.
void PresenceStatePlugin::emitValueChanged()
{
    // TODO
    // Is it as simple as reading the globalAvailabilityChanged value and forwarded to context fw ?
    emit valueChanged(presenceStateKey, "Available");
}


}  // end namespace
