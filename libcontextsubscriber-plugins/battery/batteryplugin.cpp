/*
 * Copyright (C) 2009 Nokia Corporation.
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

#include "batteryplugin.h"
#include "sconnect.h"

#include "logging.h"


/// The factory method for constructing the IPropertyProvider instance.
IProviderPlugin* pluginFactory(const QString& /*constructionString*/)
{
    // Note: it's the caller's responsibility to delete the plugin if
    // needed.
    return new ContextSubscriberBattery::BatteryPlugin();
}

namespace ContextSubscriberBattery {


/// Constructor. Try to connect to Battery right away.
BatteryPlugin::BatteryPlugin()
{


}

/// Destructor.
BatteryPlugin::~BatteryPlugin()
{


}


/// Implementation of the IPropertyProvider::subscribe. We don't need
/// any extra work for subscribing to keys, thus subscribe is finished
/// right away.
void BatteryPlugin::subscribe(QSet<QString> keys)
{
}

/// Implementation of the IPropertyProvider::unsubscribe. We're not
/// keeping track on subscriptions, so we don't need to do anything.
void BatteryPlugin::unsubscribe(QSet<QString> keys)
{
}

/// A BMEEvent is received
void BatteryPlugin::onBMEEvent()
{
}

/// Check the current status of the Session.State property and emit
/// the valueChanged signal.
void BatteryPlugin::emitValueChanged()
{
}

} // end namespace

