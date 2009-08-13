/*
 * Copyright (C) 2008, 2009 Nokia Corporation.
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

#include "haldeviceinterface.h"

namespace ContextD {

#define INTERFACE_NAME "org.freedesktop.Hal.Device"

/*!
    \class HalDeviceInterface

    \brief The interface to a specified Hal Device dbus object.
   
    Hal device is a basic object in the hal subsystem. In HalProvider
    we create interfaces to hal battery devices.
*/

/// Creates a new interface to the specified device with \a objectPath at
/// \a busName with a given \a connection dbus connection.
HalDeviceInterface::HalDeviceInterface(const QDBusConnection connection, const QString &busName, const QString objectPath, QObject *parent)
    : QDBusAbstractInterface(busName, objectPath, INTERFACE_NAME, connection, parent)
{
}

/// Returns the current value for the given \a prop property.
QVariant HalDeviceInterface::readValue(const QString &prop)
{
    QDBusMessage msg = call(QDBus::Block, "GetPropertyInteger", prop);
    if (msg.arguments().length() == 0)
        return QVariant();
    else
        return msg.arguments().at(0);
}

}
