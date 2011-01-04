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

#ifndef BLUEZPLUGIN_H
#define BLUEZPLUGIN_H

#include <iproviderplugin.h> // For IProviderPlugin definition

#include <QDBusError>
#include <QDBusObjectPath>
#include <QDBusInterface>
#include <QSet>
#include <QMap>
#include <QString>

class QDBusServiceWatcher;
class QDBusPendingCallWatcher;

using ContextSubscriber::IProviderPlugin;

extern "C" {
    IProviderPlugin* pluginFactory(const QString& constructionString);
}

class AsyncDBusInterface; // From libcontextsubscriber-dev

namespace ContextSubscriberBluez
{

/*!
  \class BluezPlugin

  \brief A libcontextsubscriber plugin for communicating with Bluez
  over D-Bus. Provides context properties Bluetooth.Enabled and
  Bluetooth.Visible.

 */

class BluezPlugin : public IProviderPlugin
{
    Q_OBJECT

public:
    explicit BluezPlugin();
    virtual void subscribe(QSet<QString> keys);
    virtual void unsubscribe(QSet<QString> keys);
    virtual void blockUntilReady();
    virtual void blockUntilSubscribed(const QString& key);

private Q_SLOTS:
    void onPropertyChanged(QString key, QDBusVariant value);
    void onDefaultAdapterChanged(QDBusObjectPath path);
    void emitFailed(QString reason = QString("Bluez left D-Bus"));
    void defaultAdapterFinished(QDBusPendingCallWatcher* pcw);
    void getPropertiesFinished(QDBusPendingCallWatcher* pcw);

private:
    void connectToBluez();
    void disconnectFromBluez();
    void callGetProperties();
    AsyncDBusInterface* manager; ///< Bluez Manager interface
    AsyncDBusInterface* adapter; ///< Bluez Adapter interface
    QString adapterPath; ///< Object path of the Bluez adapter
    static const QString serviceName; ///< Bluez service name
    static const QString managerPath; ///< Object path of Bluez Manager
    static const QString managerInterface; ///< Interface name of Bluez manager
    static const QString adapterInterface; ///< Interface name of Bluez adapter
    static QDBusConnection busConnection; ///< QDBusConnection used for talking with Bluez

    enum ConnectionStatus {NotConnected, Connecting, Connected};
    ConnectionStatus status; ///< Whether we're currently connected to Bluez
    QDBusServiceWatcher* serviceWatcher; ///< For watching Bluez appear and disappear
    QDBusPendingCallWatcher* defaultAdapterWatcher; ///< For watching the DefaultAdatpter D-Bus call
    QDBusPendingCallWatcher* getPropertiesWatcher; ///< For watching the DefaultAdatpter D-Bus call

    QMap<QString, QString> properties; ///< Mapping of Bluez properties to Context FW properties
    QMap<QString, QVariant> propertyCache;
    QSet<QString> pendingSubscriptions; ///< Keys for which subscribeFinished/Failed hasn't been emitted
    QSet<QString> wantedSubscriptions; ///< What the upper layer wants us to be subscribed to
};
}

#endif
