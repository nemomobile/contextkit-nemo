/*
 * Copyright (C) 2010 Nokia Corporation.
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

#ifndef PROFILEPLUGIN_H
#define PROFILEPLUGIN_H

#include <iproviderplugin.h> // For IProviderPlugin definition

#include <QObject>

using ContextSubscriber::IProviderPlugin;

extern "C" {
    IProviderPlugin* pluginFactory(const QString& constructionString);
}

class QDBusPendingCallWatcher;
class QDBusServiceWatcher;
class QDBusArgument;
class AsyncDBusInterface;

struct MyStructure
{
    QString key, val, type;
};
QDBusArgument &operator<<(QDBusArgument &a, const MyStructure &mystruct);
const QDBusArgument &operator>>(const QDBusArgument &a, MyStructure &mystruct);
Q_DECLARE_METATYPE(MyStructure)
Q_DECLARE_METATYPE(QList<MyStructure>)

namespace ContextSubscriberProfile
{

/*!
  \class ProfilePlugin

  \brief A libcontextsubscriber plugin for communicating with ProfileD
  over D-Bus. Provides context property Profile.Name.

 */

class ProfilePlugin : public IProviderPlugin
{
    Q_OBJECT

public:
    explicit ProfilePlugin();
    virtual void subscribe(QSet<QString> keys);
    virtual void unsubscribe(QSet<QString> keys);
    virtual void blockUntilReady();
    virtual void blockUntilSubscribed(const QString& key);

private slots:
    void profileChanged(bool changed, bool active, QString profile, QList<MyStructure> values);
    void getProfileCallFinishedSlot(QDBusPendingCallWatcher *call);
    void serviceRegisteredSlot(const QString& serviceName);
    void serviceUnregisteredSlot(const QString& serviceName);

private:
    AsyncDBusInterface *interface;
    QDBusPendingCallWatcher *callWatcher;
    QDBusServiceWatcher *serviceWatcher;
    QString activeProfile;
};
}

#endif
