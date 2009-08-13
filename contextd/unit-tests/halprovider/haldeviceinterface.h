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

#ifndef HALDEVICEINTERFACE_H
#define HALDEVICEINTERFACE_H

#include <QObject>
#include <QDBusAbstractInterface>

class QDBusConnection;
class QString;

class HalDeviceInterface : public QObject
{
    Q_OBJECT

public:
    HalDeviceInterface(const QDBusConnection connection, const QString &busName, const QString objectPath, QObject *parent = 0);
    void fakeModified();

signals:
    void PropertyModified();

public:
    QVariant readValue(const QString &prop);
};

#endif
