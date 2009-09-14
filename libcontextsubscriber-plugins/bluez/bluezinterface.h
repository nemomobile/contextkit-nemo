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

#ifndef BLUEZINTERFACE_H
#define BLUEZINTERFACE_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QDBusConnection>
#include <QVariant>
#include <QDBusObjectPath>
#include <QDBusInterface>
#include <QDBusError>
#include <QDBusVariant>

namespace ContextSubscriberBluez
{

/*!

  \class BluezInterface

  \brief Communicates with bluetoothd over D-Bus and signals the
  changes in Bluez properties.

 */
class BluezInterface : public QObject
{
    Q_OBJECT
public:
    BluezInterface();
    void connectToBluez();

signals:
    void propertyChanged(QString key, QVariant value);
    void ready();
    void failed(QString error);

private slots:
    void replyDBusError(QDBusError err);
    void replyDefaultAdapter(QDBusObjectPath path);
    void replyGetProperties(QMap<QString, QVariant> map);
    void onPropertyChanged(QString key, QDBusVariant value);
    void onNameOwnerChanged(QString name, QString oldOwner, QString newOwner);
private:
    void onPropertyChanged(QString key, QVariant value);

    QDBusInterface* manager;
    QDBusInterface* adapter;
    QString adapterPath;
    static const QString serviceName;
    static const QString managerPath;
    static const QString managerInterface;
    static const QString adapterInterface;
    static QDBusConnection busConnection;
};

}
#endif
