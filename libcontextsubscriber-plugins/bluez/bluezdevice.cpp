/*
 * Copyright (C) 2011 Nokia Corporation.
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
#include "bluezdevice.h"
#include "sconnect.h"
#include "logging.h"
// This is for getting rid of synchronous D-Bus introspection calls Qt does.
#include <asyncdbusinterface.h>

#include <QDBusPendingReply>
#include <QDBusPendingCallWatcher>

namespace ContextSubscriberBluez {

const QString BluezDevice::serviceName = "org.bluez";
const QString BluezDevice::deviceInterface = "org.bluez.Device";
QDBusConnection BluezDevice::busConnection = QDBusConnection::systemBus();

BluezDevice::BluezDevice(const QString& path)
    : getPropertiesWatcher(0), connected(false), devicePath(path), device(0)
{
    busConnection.connect(serviceName, path, deviceInterface, "PropertyChanged",
                          this, SLOT(onPropertyChanged(QString, QDBusVariant)));

    device = new AsyncDBusInterface(serviceName, path,
				    deviceInterface, busConnection, this);

    getPropertiesWatcher = new QDBusPendingCallWatcher(
	device->asyncCall("GetProperties"));

    sconnect(getPropertiesWatcher,
             SIGNAL(finished(QDBusPendingCallWatcher*)),
	     this,
             SLOT(getPropertiesFinished(QDBusPendingCallWatcher*)));
}

BluezDevice::~BluezDevice() {
    busConnection.disconnect(serviceName, devicePath.path(),
                          deviceInterface, "PropertyChanged",
                          this, SLOT(onPropertyChanged(QString, QDBusVariant)));
    delete device;
    device = 0;
    delete getPropertiesWatcher;
    getPropertiesWatcher = 0;
}

/// Called when the GetProperties D-Bus call is done.
void BluezDevice::getPropertiesFinished(QDBusPendingCallWatcher* pcw)
{
    QDBusPendingReply<QMap<QString, QVariant> > reply = *pcw;
    QMap<QString, QVariant> map = reply.argumentAt<0>();
    Q_FOREACH (const QString& key, map.keys()) {
        if (key == "Connected") {
	    connected = map[key].toBool();
	    Q_EMIT connectionStateChanged(devicePath, connected);
	}
    }

    if (getPropertiesWatcher == pcw) {
        getPropertiesWatcher = 0;
    }
    pcw->deleteLater();
}

/// Connected to the D-Bus signal PropertyChanged from BlueZ /
/// device. Check if the change is relevant, and if so, signal
/// that the device is connected.
void BluezDevice::onPropertyChanged(QString key, QDBusVariant value)
{
    contextDebug() << "On Property Changed";
    if (key == "Connected") {
	connected = value.variant().toBool();
	Q_EMIT connectionStateChanged(devicePath, connected);
	contextDebug() << "connection state changed signal emitted: " << connected;
    }
}

bool BluezDevice::isConnected() {
    return connected;
}

} // end namespace
