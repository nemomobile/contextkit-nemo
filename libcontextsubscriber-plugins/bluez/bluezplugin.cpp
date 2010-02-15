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

#include <asyncdbusinterface.h>

#include <QDBusServiceWatcher>

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

BluezPlugin::BluezPlugin() : manager(0), adapter(0), status(NotConnected), serviceWatcher(0)
{
    // Create a mapping from Bluez properties to Context Properties
    properties["Powered"] = "Bluetooth.Enabled";
    properties["Discoverable"] = "Bluetooth.Visible";

    // We're ready to take in subscriptions right away; we'll connect to bluez
    // when we get subscriptions.
    QMetaObject::invokeMethod(this, "emitReady", Qt::QueuedConnection);
}

void BluezPlugin::disconnectFromBluez()
{
    qDebug() << "disconnecting from bluez";
    status = NotConnected;
    delete adapter;
    adapter = 0;
    delete manager;
    manager = 0;
    delete serviceWatcher;
    serviceWatcher = 0;
}

/// Try to establish the connection to BlueZ.
void BluezPlugin::connectToBluez()
{
    qDebug() << "connecting to bluez";
    disconnectFromBluez();
    status = Connecting;

    // FIXME: this is "too early"; bluez might not have a default adapter yet

    manager = new AsyncDBusInterface(serviceName, managerPath, managerInterface, busConnection, this);
    manager->callWithCallback("DefaultAdapter", QList<QVariant>(), this,
                              SLOT(replyDefaultAdapter(QDBusObjectPath)),
                              SLOT(replyDBusError(QDBusError)));
    serviceWatcher = new QDBusServiceWatcher(serviceName, busConnection);
    // When Bluez disappears from D-Bus, we emit failed to signal that we're
    // not able to take in subscriptions. And when Bluez reappears, we emit
    // "ready". Then the upper layer will renew its subscriptions (and we
    // reconnect to bluez if needed).
    connect(serviceWatcher, SIGNAL(serviceRegistered(const QString&)), this, SLOT(emitReady()), Qt::QueuedConnection);
    connect(serviceWatcher, SIGNAL(serviceUnregistered(const QString&)), this, SLOT(emitFailed()));
}

/// Called when a D-Bus error occurs when processing our
/// callWithCallback.
void BluezPlugin::replyDBusError(QDBusError err)
{
    emit failed("Cannot connect to BlueZ: " + err.message());
    foreach (const QString& key, pendingSubscriptions)
        emit subscribeFailed(key, "Cannot connect to Bluez: " + err.message());
    pendingSubscriptions.clear();
}

/// Called when the DefaultAdapter D-Bus call is done.
void BluezPlugin::replyDefaultAdapter(QDBusObjectPath path)
{
    contextDebug();
    adapterPath = path.path();
    adapter = new AsyncDBusInterface(serviceName, adapterPath, adapterInterface, busConnection, this);
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
        propertyCache[properties[key]] = value.variant();
        emit valueChanged(properties[key], value.variant());
    }
}

/// Called when the GetProperties D-Bus call is done.
void BluezPlugin::replyGetProperties(QMap<QString, QVariant> map)
{
    contextDebug();
    status = Connected;
    foreach(const QString& key, map.keys()) {
        if (properties.contains(key)) {
            contextDebug() << "Prop changed:" << properties[key];
            propertyCache[properties[key]] = map[key];
            emit valueChanged(properties[key], map[key]);
            // Note: the upper layer is responsible for checking if the
            // value was a different one.
        }
    }
    foreach (const QString& key, pendingSubscriptions) {
        if (propertyCache.contains(key))
            emit subscribeFinished(key, propertyCache[key]);
        else
            emit subscribeFailed(key, "Unknown key");
    }
    pendingSubscriptions.clear();
}

/// Implementation of the IPropertyProvider::subscribe. If we're connected to
/// bluez, no extra work is needed. Otherwise, initiate connecting to bluez.
void BluezPlugin::subscribe(QSet<QString> keys)
{
    if (status == Connected) {
        // we're already connected to bluez; so we know values for all the keys
        foreach(const QString& key, keys) {
            // Ensure that we give some values for the subscribed properties
            if (propertyCache.contains(key)) {
                contextDebug() << "Key" << key << "found in cache";
                wantedSubscriptions << key;
                emit subscribeFinished(key, propertyCache[key]);
            }
            else {
                // This shouldn't occur if the plugin and libcontextsubscriber function correctly
                contextCritical() << "Key not in cache" << key;
                emit subscribeFailed(key, "Unknown key");
            }
        }
    }
    else {
        foreach(const QString& key, keys) {
            pendingSubscriptions << key;
            wantedSubscriptions << key;
        }
        if (status == NotConnected)
            connectToBluez();
    }
}

/// Implementation of the IPropertyProvider::unsubscribe. If none of the
/// properties is needed, disconnect from bluez. The next subscriptions will
/// make us connect again.
void BluezPlugin::unsubscribe(QSet<QString> keys)
{
    foreach(const QString& key, keys)
        wantedSubscriptions.remove(key);
    if (wantedSubscriptions.isEmpty()) {
        disconnectFromBluez();
    }
}

/// For emitting the ready() signal in a delayed way.
void BluezPlugin::emitReady()
{
    emit ready();
}

/// For emitting the failed() signal in a delayed way.
void BluezPlugin::emitFailed(QString reason)
{
    status = NotConnected;
    emit failed(reason);
}

} // end namespace

