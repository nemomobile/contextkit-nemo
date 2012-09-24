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
#include <contextkit_props/presence.hpp>

IProviderPlugin* pluginFactory(const QString& /*constructionString*/)
{
    return new ContextSubscriberPresence::PresenceStatePlugin();
}

namespace ContextSubscriberPresence {

PresenceStatePlugin::PresenceStatePlugin(): presenceStateKey(presence_state)
{
    // Emitting signals inside ctor doesn't make sense.  Thus, queue it.
    QMetaObject::invokeMethod(this, "ready", Qt::QueuedConnection);
}

QString PresenceStatePlugin::mapPresence(GlobalPresenceIndicator::GLOBAL_PRESENCE presence)
{
    switch (presence) {
    case GlobalPresenceIndicator::GLOBAL_PRESENCE_OFFLINE:
        return CONTEXT_PRESENCE_OFFLINE;
    case GlobalPresenceIndicator::GLOBAL_PRESENCE_CONNECTING:
        return CONTEXT_PRESENCE_CONNECTING;
    case GlobalPresenceIndicator::GLOBAL_PRESENCE_ONLINE:
        return CONTEXT_PRESENCE_ONLINE;
    case GlobalPresenceIndicator::GLOBAL_PRESENCE_BUSY:
        return CONTEXT_PRESENCE_BUSY;
    default:
        return QString();
    }

}

void PresenceStatePlugin::subscribe(QSet<QString> keys)
{
    // Check for invalid keys
    foreach (const QString& key, keys) {
        if (key != presenceStateKey) {
            Q_EMIT subscribeFailed(key, "Invalid key");

        }
    }

    if (keys.contains(presenceStateKey)) {
        // Connect to the global account change
        sconnect(GlobalPresenceIndicator::instance(),
                 SIGNAL(globalPresenceChanged(GlobalPresenceIndicator::GLOBAL_PRESENCE)),
                 this,
                 SLOT(emitValueChanged(GlobalPresenceIndicator::GLOBAL_PRESENCE)));

        QString presence = mapPresence(GlobalPresenceIndicator::instance()->globalPresence());
        // Now the value is there; signal that the subscription is done.
        Q_EMIT subscribeFinished(presenceStateKey, QVariant(presence));
    }
}

void PresenceStatePlugin::unsubscribe(QSet<QString> keys)
{
    GlobalPresenceIndicator::killInstance();
}

void PresenceStatePlugin::blockUntilReady()
{
    // This plugin is immediately ready
    Q_EMIT ready();
}

void PresenceStatePlugin::blockUntilSubscribed(const QString& key)
{
    // subscribe() has already set the value, no need to do anything.
}

/// Check the current status of the Session.State property and emit
/// the valueChanged signal.
void PresenceStatePlugin::emitValueChanged(GlobalPresenceIndicator::GLOBAL_PRESENCE presence)
{
    Q_EMIT valueChanged(presenceStateKey, mapPresence(presence));
}
}  // end namespace
