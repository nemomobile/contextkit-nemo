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

#include "bluezplugin.h"
#include "sconnect.h"

#include "logging.h"

/// The factory method for constructing the IPropertyProvider instance.
IProviderPlugin* pluginFactory(const QString& /*constructionString*/)
{
    // Note: it's the caller's responsibility to delete the plugin if
    // needed.
    return new ContextSubscriberBluez::BluezPlugin();
}

namespace ContextSubscriberBluez {

/// Constructor. Construct the BluezInterface which handles the D-Bus
/// connection, and tell it to try to connect to Bluez right away.
BluezPlugin::BluezPlugin()
{
    // Connect signals from the Bluez interface. The events we listen to are:
    // - interface getting connected to Bluez
    // - interface failing to connect to Bluez
    // - interface losing connection to Bluez
    // - Bluez property changing
    sconnect(&bluezInterface, SIGNAL(ready()),
             this, SIGNAL(ready()));
    sconnect(&bluezInterface, SIGNAL(failed(QString)),
             this, SIGNAL(failed(QString)));
    sconnect(&bluezInterface, SIGNAL(propertyChanged(QString, QVariant)),
             this, SLOT(onPropertyChanged(QString, QVariant)));
    bluezInterface.connectToBluez();

    // Create a mapping from Bluez properties to Context Properties
    properties["Powered"] = "Bluetooth.Enabled";
    properties["Discoverable"] = "Bluetooth.Visible";
}

/// Implementation of the IPropertyProvider::subscribe. We don't need
/// any extra work for subscribing to keys, thus subscribe is finished
/// right away.
void BluezPlugin::subscribe(QSet<QString> keys)
{
    contextDebug() << keys;
    foreach(const QString& key, keys) {
        emit subscribeFinished(key);
    }
}

/// Implementation of the IPropertyProvider::unsubscribe. We're not
/// keeping track on subscriptions, so we don't need to do anything.
void BluezPlugin::unsubscribe(QSet<QString> keys)
{
}

/// Called when a Bluez property changes. Check if the change is
/// relevant, and if so, signal the value change of the corresponding
/// context property.
void BluezPlugin::onPropertyChanged(QString key, QVariant value)
{
    contextDebug() << key << value.toString();
    if (properties.contains(key)) {
        contextDebug() << "Prop changed:" << properties[key] << value.toString();
        emit valueChanged(properties[key], value);
        // Note: the upper layer is responsible for checking if the
        // value was a different one.
    }
}

} // end namespace

