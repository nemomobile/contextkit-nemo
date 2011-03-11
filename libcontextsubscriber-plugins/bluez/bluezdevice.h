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

#ifndef BLUEZDEVICE_H
#define BLUEZDEVICE_H

#include <QDBusError>
#include <QDBusObjectPath>
#include <QDBusInterface>
#include <QSet>
#include <QMap>
#include <QString>

#define BLUEZ_PLUGIN_BUS QDBusConnection::systemBus()

class QDBusPendingCallWatcher;
class AsyncDBusInterface; // From libcontextsubscriber-dev

namespace ContextSubscriberBluez
{

/*!
  \class BluezDevice

  \brief A proxy class for Bluez D-Bus device interface.

 */

class BluezDevice : public QObject
{
    Q_OBJECT

public:
    explicit BluezDevice(const QString& path);
    ~BluezDevice();
    bool isConnected();

Q_SIGNALS:
    void connectionStateChanged(bool status);

private Q_SLOTS:
    void onPropertyChanged(QString key, QDBusVariant value);
    void getPropertiesFinished(QDBusPendingCallWatcher* pcw);

private:
    QDBusPendingCallWatcher* getPropertiesWatcher; ///< For watching the GetProperties D-Bus call
    bool connected; ///< Whether device is currently connected

    QDBusObjectPath devicePath; ///< Object path of the Bluez device
    AsyncDBusInterface* device; ///< Bluez Device interface

    static const QString serviceName; ///< Bluez service name
    static const QString deviceInterface; ///< Interface name of Bluez device
    static QDBusConnection busConnection; ///< QDBusConnection used for talking with Bluez


};
}

#endif
