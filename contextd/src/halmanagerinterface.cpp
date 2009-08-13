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

#include "halmanagerinterface.h"
#include <QDBusInterface>
#include <QStringList>

namespace ContextD {

#define INTERFACE_NAME  "org.freedesktop.Hal.Manager"
#define OBJECT_PATH     "/org/freedesktop/Hal/Manager"

/*!
    \class HalManagerInterface

    \brief The interface to the Hal Manager dbus object.
   
    It's used to get a list of devices (ie. batteries).
*/

/// Constructor. Creates a new interface to the Hal Manager object. 
HalManagerInterface::HalManagerInterface(const QDBusConnection connection, const QString &busName, QObject *parent)
    : QDBusAbstractInterface(busName, OBJECT_PATH, INTERFACE_NAME, connection, parent)
{
}

/// Returns a list of devices matching the \a capability existing in Hal.
/// It's used to get the list of batteries.
QStringList HalManagerInterface::findDeviceByCapability(const QString &capability)
{
    QDBusMessage msg = call(QDBus::Block, "FindDeviceByCapability", capability);
    return msg.arguments().at(0).toStringList();
}

}
