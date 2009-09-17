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

const QString BluezPlugin::serviceName = "org.bluez";
const QString BluezPlugin::managerPath = "/";
const QString BluezPlugin::managerInterface = "org.bluez.Manager";
const QString BluezPlugin::adapterInterface = "org.bluez.Adapter";
QDBusConnection BluezPlugin::busConnection = QDBusConnection::systemBus();

/// Constructor. Try to connect to Bluez right away.
BluezPlugin::BluezPlugin() : manager(0), adapter(0)
{
    busConnection.connect("org.freedesktop.DBus", "/org/freedesktop/DBus",
                          "org.freedesktop.DBus", "NameOwnerChanged",
                          this, SLOT(onNameOwnerChanged(QString, QString, QString)));
    // Create a mapping from Bluez properties to Context Properties
    properties["Powered"] = "Bluetooth.Enabled";
    properties["Discoverable"] = "Bluetooth.Visible";

    connectToBluez();
}

/// Called when the nameOwnerChanged signal is received over D-Bus.
void BluezPlugin::onNameOwnerChanged(QString name, QString /*oldOwner*/, QString newOwner)
{
    if (name == serviceName) {
        if (newOwner != "") {
            // BlueZ appeared -> connect to it. If successful, ready()
            // will be emitted when the connection is established.
            connectToBluez();
        }
        else {
            // BlueZ disappeared
            emit failed("BlueZ left D-Bus");
        }
    }
}

/// Try to establish the connection to BlueZ.
void BluezPlugin::connectToBluez()
{
    if (adapter) {
        busConnection.disconnect(serviceName,
                                 adapterPath,
                                 adapterInterface,
                                 "PropertyChanged",
                                 this, SLOT(onPropertyChanged(QString, QDBusVariant)));
        delete adapter;
        adapter = 0;
    }
    if (manager) {
        delete manager;
        manager = 0;
    }

    manager = new QDBusInterface(serviceName, managerPath, managerInterface, busConnection, this);
    manager->callWithCallback("DefaultAdapter", QList<QVariant>(), this,
                              SLOT(replyDefaultAdapter(QDBusObjectPath)),
                              SLOT(replyDBusError(QDBusError)));
}

/// Called when a D-Bus error occurs when processing our
/// callWithCallback.
void BluezPlugin::replyDBusError(QDBusError err)
{
    contextWarning() << "DBus error occured:" << err.message();
    emit failed("Cannot connect to BlueZ:" + err.message());
}

/// Called when the DefaultAdapter D-Bus call is done.
void BluezPlugin::replyDefaultAdapter(QDBusObjectPath path)
{
    contextDebug();
    adapterPath = path.path();
    adapter = new QDBusInterface(serviceName, adapterPath, adapterInterface, busConnection, this);
    busConnection.connect(serviceName,
                          path.path(),
                          adapterInterface,
                          "PropertyChanged",
                          this, SLOT(onPropertyChanged(QString, QDBusVariant)));
    adapter->callWithCallback("GetProperties", QList<QVariant>(), this,
                              SLOT(replyGetProperties(QMap<QString, QVariant>)),
                              SLOT(replyDBusError(QDBusError)));
}

/// Connected to the D-Bus signal PropertyChanged from BlueZ /
/// adaptor. Check if the change is relevant, and if so, signal the
/// value change of the corresponding context property.
void BluezPlugin::onPropertyChanged(QString key, QDBusVariant value)
{
    contextDebug() << key << value.variant().toString();
    if (properties.contains(key)) {
        contextDebug() << "Prop changed:" << properties[key];
        emit valueChanged(properties[key], value.variant());
    }
}

/// Called when the GetProperties D-Bus call is done.
void BluezPlugin::replyGetProperties(QMap<QString, QVariant> map)
{
    contextDebug();
    emit ready();
    foreach(const QString& key, map.keys()) {
        if (properties.contains(key)) {
            contextDebug() << "Prop changed:" << properties[key];
            emit valueChanged(properties[key], map[key]);
            // Note: the upper layer is responsible for checking if the
            // value was a different one.
        }
    }
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

} // end namespace

