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
#include "bluezdevice.h"
#include "sconnect.h"

#include "logging.h"
// This is for getting rid of synchronous D-Bus introspection calls Qt does.
#include <asyncdbusinterface.h>

#include <QDBusServiceWatcher>
#include <QDBusPendingReply>
#include <QDBusPendingCallWatcher>

#include <contextkit_props/bluetooth.hpp>

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
const QString BluezPlugin::deviceInterface = "org.bluez.Device";

BluezPlugin::BluezPlugin()
    : manager(0), adapter(0), status(NotConnected), serviceWatcher(0),
      defaultAdapterWatcher(0), getPropertiesWatcher(0)
{
    // Create a mapping from Bluez properties to Context Properties
    properties["Powered"] = bluetooth_is_enabled;
    properties["Discoverable"] = bluetooth_is_visible;
    propertyCache[bluetooth_is_connected] = false;

    // We're ready to take in subscriptions right away; we'll connect to bluez
    // when we get subscriptions.
    QMetaObject::invokeMethod(this, "ready", Qt::QueuedConnection);
}

void BluezPlugin::disconnectFromBluez()
{
    status = NotConnected;

    // Disconnect D-Bus signals
    BLUEZ_PLUGIN_BUS.disconnect(serviceName, managerPath,
                             managerInterface, "DefaultAdapterChanged",
                             this, SLOT(onDefaultAdapterChanged(QDBusObjectPath)));

    BLUEZ_PLUGIN_BUS.disconnect(serviceName, adapterPath,
                             adapterInterface, "PropertyChanged",
                             this, SLOT(onPropertyChanged(QString, QDBusVariant)));

    BLUEZ_PLUGIN_BUS.disconnect(serviceName, adapterPath,
                          adapterInterface, "DeviceCreated",
                          this, SLOT(onDeviceCreated(QDBusObjectPath)));

    BLUEZ_PLUGIN_BUS.disconnect(serviceName, adapterPath,
                          adapterInterface, "DeviceRemoved",
                          this, SLOT(onDeviceRemoved(QDBusObjectPath)));

    Q_FOREACH (const QDBusObjectPath& path, devicesList.keys()) {
        onDeviceRemoved(path);
    }

    delete adapter;
    adapter = 0;
    delete manager;
    manager = 0;
    delete serviceWatcher;
    serviceWatcher = 0;

    delete defaultAdapterWatcher;
    defaultAdapterWatcher = 0;
    delete getPropertiesWatcher;
    getPropertiesWatcher = 0;
}

/// Try to establish the connection to BlueZ.
void BluezPlugin::connectToBluez()
{
    disconnectFromBluez();
    status = Connecting;

    // If this function is executed because Bluez has just appeared on D-Bus,
    // it might be too early for the default adaptor to exist. It might be
    // that the DefaultAdapter call fails. To tackle that, we don't treat that
    // as a real failure, but before calling DefaultAdapter, we subscribe to
    // the DefaultAdapterChanged signal from Bluez. When Bluez gets the
    // default adapter set up, we will get our callback called. A drawback: if
    // the default adapter is never assigned, this plugin won't inform the
    // upper layer of the error; it will just look as if everything is fine
    // (properties keep their old values or are unknown).

    BLUEZ_PLUGIN_BUS.connect(serviceName, managerPath, managerInterface, "DefaultAdapterChanged",
                          this, SLOT(onDefaultAdapterChanged(QDBusObjectPath)));
    manager = new AsyncDBusInterface(serviceName, managerPath, managerInterface, BLUEZ_PLUGIN_BUS, this);

    defaultAdapterWatcher =
        new QDBusPendingCallWatcher(manager->asyncCall("DefaultAdapter"));
    sconnect(defaultAdapterWatcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
             this, SLOT(defaultAdapterFinished(QDBusPendingCallWatcher*)));

    // When Bluez disappears from D-Bus, we emit failed to signal that we're
    // not able to take in subscriptions. And when Bluez reappears, we emit
    // "ready". Then the upper layer will renew its subscriptions (and we
    // reconnect to bluez if needed).
    serviceWatcher = new QDBusServiceWatcher(serviceName, BLUEZ_PLUGIN_BUS);
    sconnect(serviceWatcher, SIGNAL(serviceRegistered(const QString&)),
            this, SIGNAL(ready()), Qt::QueuedConnection);
    sconnect(serviceWatcher, SIGNAL(serviceUnregistered(const QString&)), this, SLOT(emitFailed()));
}

void BluezPlugin::evalConnected() {

    propertyCache[bluetooth_is_connected] = false;

    Q_FOREACH (BluezDevice* device, devicesList) {
        if (device->isConnected()) {
            propertyCache[bluetooth_is_connected] = true;
            break;
        }
    }

    Q_EMIT valueChanged(bluetooth_is_connected, propertyCache[bluetooth_is_connected]);
}

void BluezPlugin::onConnectionStateChanged(bool status)
{
    if (propertyCache[bluetooth_is_connected].toBool() && !status) {
        evalConnected();
    }

    if (!propertyCache[bluetooth_is_connected].toBool() && status) {
        propertyCache[bluetooth_is_connected] = status;
        Q_EMIT valueChanged(bluetooth_is_connected,
                            propertyCache[bluetooth_is_connected]);
    }
}

void BluezPlugin::onDeviceCreated(QDBusObjectPath devicePath)
{
    if (!devicesList.contains(devicePath)) {
        devicesList[devicePath] = new BluezDevice(devicePath.path());
        sconnect(devicesList[devicePath], SIGNAL(connectionStateChanged(bool)),
                 this,  SLOT(onConnectionStateChanged(bool)));
    }
}

void BluezPlugin::onDeviceRemoved(QDBusObjectPath devicePath)
{
    if (devicesList.contains(devicePath)) {
        disconnect(devicesList[devicePath], SIGNAL(connectionStateChanged(bool)),
                   this,  SLOT(onConnectionStateChanged(bool)));

        delete devicesList[devicePath];
        devicesList.remove(devicePath);
    }
}

/// Initates the async GetProperties D-Bus call.  Overwrites
/// getPropertiesWatcher, so the watcher of the old call is no longer pointed to
/// by BluezPlugin.
void BluezPlugin::callGetProperties()
{
    getPropertiesWatcher = new QDBusPendingCallWatcher(
        adapter->asyncCall("GetProperties"));
    sconnect(getPropertiesWatcher,
             SIGNAL(finished(QDBusPendingCallWatcher*)),
             this,
             SLOT(getPropertiesFinished(QDBusPendingCallWatcher*)));
}

/// Called when the DefaultAdapter D-Bus call finishes
void BluezPlugin::defaultAdapterFinished(QDBusPendingCallWatcher* pcw)
{
    QDBusPendingReply<QDBusObjectPath> reply = *pcw;
    if (reply.isError()) {
        if (reply.error().type() == QDBusError::ServiceUnknown) {
            // We need to fail explicitly so that we can emit ready() when the
            // provider is started. (We have already emitted ready(), and
            // emitting ready() 2 times has no effect.)
            Q_EMIT failed("Provider not present: bluez");
        }
        else {
            // It might be that we're calling DefaultAdapter but there is none
            // currently; try to get it later. Tell the upper layer that all
            // keys are now subscribed (they will keep their previous values)
            // and hope that the DefaultAdapter signal comes at some point.
            Q_FOREACH (const QString& key, pendingSubscriptions)
                Q_EMIT subscribeFinished(key);
            pendingSubscriptions.clear();
        }
    }
    else {
            adapterPath = reply.argumentAt<0>().path();
            adapter = new AsyncDBusInterface(serviceName, adapterPath,
                                             adapterInterface, BLUEZ_PLUGIN_BUS,
                                             this);

            BLUEZ_PLUGIN_BUS.connect(serviceName, adapterPath,
                          adapterInterface, "PropertyChanged",
                          this, SLOT(onPropertyChanged(QString, QDBusVariant)));

            BLUEZ_PLUGIN_BUS.connect(serviceName, adapterPath,
                          adapterInterface, "DeviceCreated",
                          this, SLOT(onDeviceCreated(QDBusObjectPath)));

            BLUEZ_PLUGIN_BUS.connect(serviceName, adapterPath,
                          adapterInterface, "DeviceRemoved",
                          this, SLOT(onDeviceRemoved(QDBusObjectPath)));

            callGetProperties();
    }

    if (defaultAdapterWatcher == pcw) {
        defaultAdapterWatcher = 0;
    }
    pcw->deleteLater();
}

/// Called when the GetProperties D-Bus call is done.
void BluezPlugin::getPropertiesFinished(QDBusPendingCallWatcher* pcw)
{
    status = Connected;
    QDBusPendingReply<QMap<QString, QVariant> > reply = *pcw;
    QMap<QString, QVariant> map = reply.argumentAt<0>();
    Q_FOREACH (const QString& key, map.keys()) {
        if (properties.contains(key)) {
            propertyCache[properties[key]] = map[key];
            Q_EMIT valueChanged(properties[key], map[key]);
            // Note: the upper layer is responsible for checking if the
            // value was a different one.
        }

        if (key == "Devices") {
            QList<QDBusObjectPath> devicePaths = qdbus_cast<QList<QDBusObjectPath> >(map[key]);
            Q_FOREACH(const QDBusObjectPath& path, devicePaths)
                onDeviceCreated(path);

            evalConnected();
        }
    }

    Q_FOREACH (const QString& key, pendingSubscriptions) {
        if (propertyCache.contains(key))
            Q_EMIT subscribeFinished(key, propertyCache[key]);
        else
            Q_EMIT subscribeFailed(key, "Unknown key");
    }
    pendingSubscriptions.clear();

    if (getPropertiesWatcher == pcw) {
        getPropertiesWatcher = 0;
    }
    pcw->deleteLater();
}

/// Called when Bluez changes its default adapter.
void BluezPlugin::onDefaultAdapterChanged(QDBusObjectPath path)
{
    adapterPath = path.path();
    delete adapter;
    adapter = new AsyncDBusInterface(serviceName, adapterPath, adapterInterface, BLUEZ_PLUGIN_BUS, this);
    BLUEZ_PLUGIN_BUS.connect(serviceName, adapterPath,
                          adapterInterface, "PropertyChanged",
                          this, SLOT(onPropertyChanged(QString, QDBusVariant)));

    BLUEZ_PLUGIN_BUS.connect(serviceName, adapterPath,
                          adapterInterface, "DeviceCreated",
                          this, SLOT(onDeviceCreated(QDBusObjectPath)));

    BLUEZ_PLUGIN_BUS.connect(serviceName, adapterPath,
                          adapterInterface, "DeviceRemoved",
                          this, SLOT(onDeviceRemoved(QDBusObjectPath)));

    // It is possible that a previous GetProperties call is still ongoing.  Here
    // we start another one, and overwrite getPropertiesWatcher.  The
    // QDBusPendingCallWatcher of the old call will be deleted when the call
    // finishes.
    callGetProperties();
}

/// Connected to the D-Bus signal PropertyChanged from BlueZ /
/// adaptor. Check if the change is relevant, and if so, signal the
/// value change of the corresponding context property.
void BluezPlugin::onPropertyChanged(QString key, QDBusVariant value)
{
    if (properties.contains(key)) {
        propertyCache[properties[key]] = value.variant();
        Q_EMIT valueChanged(properties[key], value.variant());
    }
}

/// Implementation of the IPropertyProvider::subscribe. If we're connected to
/// bluez, no extra work is needed. Otherwise, initiate connecting to bluez.
void BluezPlugin::subscribe(QSet<QString> keys)
{
    if (status == Connected) {
        // we're already connected to bluez; so we know values for all the keys
        Q_FOREACH (const QString& key, keys) {
            // Ensure that we give some values for the subscribed properties
            if (propertyCache.contains(key)) {
                wantedSubscriptions << key;
                Q_EMIT subscribeFinished(key, propertyCache[key]);
            }
            else {
                // This shouldn't occur if the plugin and libcontextsubscriber function correctly
                Q_EMIT subscribeFailed(key, "Unknown key");
            }
        }
    }
    else {
        pendingSubscriptions.unite(keys);
        wantedSubscriptions.unite(keys);
        if (status == NotConnected)
            connectToBluez();
    }
}

/// Implementation of the IPropertyProvider::unsubscribe. If none of the
/// properties is needed, disconnect from bluez. The next subscriptions will
/// make us connect again.
void BluezPlugin::unsubscribe(QSet<QString> keys)
{
    wantedSubscriptions.subtract(keys);
    if (wantedSubscriptions.isEmpty()) {
        disconnectFromBluez();
    }
}

void BluezPlugin::blockUntilReady()
{
    // This plugin is ready immediately
    Q_EMIT ready();
}

void BluezPlugin::blockUntilSubscribed(const QString&)
{
    if (defaultAdapterWatcher)
        defaultAdapterWatcher->waitForFinished();
    if (getPropertiesWatcher)
        getPropertiesWatcher->waitForFinished();
}

/// For emitting the failed() signal in a delayed way.
void BluezPlugin::emitFailed(QString reason)
{
    status = NotConnected;
    Q_EMIT failed(reason);
}

} // end namespace
