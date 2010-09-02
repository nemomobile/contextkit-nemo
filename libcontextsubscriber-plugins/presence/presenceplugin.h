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

#ifndef PRESENCEPLUGIN_H
#define PRESENCEPLUGIN_H

#include <iproviderplugin.h> // For IProviderPlugin definition
#include "presence-ui/globalpresenceindicator.h"

#define CONTEXT_PRESENCE_OFFLINE "offline"
#define CONTEXT_PRESENCE_BUSY "away"
#define CONTEXT_PRESENCE_ONLINE "available"

using ContextSubscriber::IProviderPlugin;

extern "C" {
    IProviderPlugin* pluginFactory(const QString& constructionString);
}

namespace ContextSubscriberPresence
{

/*!
  \class PresencePlugin

  \brief A libcontextsubscriber plugin for communicating with libpresence.
  Provides context property Presence.Name

 */

class PresenceStatePlugin : public IProviderPlugin
{
    Q_OBJECT

public:
    explicit PresenceStatePlugin();
    virtual void subscribe(QSet<QString> keys);
    virtual void unsubscribe(QSet<QString> keys);

private Q_SLOTS:
    void emitValueChanged(GlobalPresenceIndicator::GLOBAL_PRESENCE presence);

private:
    QString presenceStateKey;
    QString mapPresence(GlobalPresenceIndicator::GLOBAL_PRESENCE presenceState);
};
}
#endif

