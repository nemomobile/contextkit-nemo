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

#include "profileplugin.h"
#include "logging.h"
#include "profile_dbus.h"
#include <QtDBus>

// Marshall the MyStructure data into a D-Bus argument
QDBusArgument &operator<<(QDBusArgument &argument, const MyStructure &mystruct)
{
    argument.beginStructure();
    argument << mystruct.key << mystruct.val << mystruct.type;
    argument.endStructure();
    return argument;
}

// Retrieve the MyStructure data from the D-Bus argument
const QDBusArgument &operator>>(const QDBusArgument &argument, MyStructure &mystruct)
{
    argument.beginStructure();
    argument >> mystruct.key;
    argument >> mystruct.val;
    argument >> mystruct.type;
    argument.endStructure();
    return argument;
}

/// The factory method for constructing the IPropertyProvider instance.
IProviderPlugin* pluginFactory(const QString& /*constructionString*/)
{
    // Note: it's the caller's responsibility to delete the plugin if
    // needed.
    return new ContextSubscriberProfile::ProfilePlugin();
}

namespace ContextSubscriberProfile {

/// Constructor. Try to connect to ProfileD right away.
ProfilePlugin::ProfilePlugin()
{
    qDBusRegisterMetaType<MyStructure>();
    qDBusRegisterMetaType<QList<MyStructure > >();
 
    bool succ = QDBusConnection::sessionBus().connect(PROFILED_SERVICE, PROFILED_PATH, PROFILED_INTERFACE, 
                                                      PROFILED_CHANGED, QString("bbsa(sss)"), this, 
                                                      SLOT(profileChanged(bool, bool, QString, QList<MyStructure>)));
    if (!succ) {
      qDebug() << "profileplugin: cannot connect to profiled.";
    }

    activeProfile = "";
    QDBusInterface *interface = new QDBusInterface(PROFILED_SERVICE, PROFILED_PATH, PROFILED_INTERFACE, QDBusConnection::sessionBus());
    QDBusPendingCall async = interface->asyncCall(PROFILED_GET_PROFILE);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(async, this);

    QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                     this, SLOT(getProfileCallFinishedSlot(QDBusPendingCallWatcher*)));
}

void ProfilePlugin::getProfileCallFinishedSlot(QDBusPendingCallWatcher *call)
{
    QDBusPendingReply<QString> reply = *call;
    if (reply.isError()) {
        qDebug() << Q_FUNC_INFO << "error reply:" << reply.error().name();
    } else {
        activeProfile = reply.argumentAt<0>();
        emit ready();
        emit valueChanged("Profile.Name", activeProfile);
    }
}

void ProfilePlugin::profileChanged(bool changed, bool active, QString profile, QList<MyStructure> /*keyValType*/)
{
    if (changed && active) {
        activeProfile = profile;
        emit valueChanged("Profile.Name", activeProfile);
    }
}

/// Implementation of the IPropertyProvider::subscribe. We don't need
/// any extra work for subscribing to keys, thus subscribe is finished
/// right away.
void ProfilePlugin::subscribe(QSet<QString> keys)
{
    contextDebug() << keys;

    foreach(const QString& key, keys) {
        // Ensure that we give some values for the subscribed properties
        if (key == "Profile.Name" && activeProfile != "") {
            contextDebug() << "Key" << key << "found in cache";
            emit subscribeFinished(key, QVariant(activeProfile));
        }
        else {
            // This shouldn't occur if the plugin functions correctly
            contextCritical() << "Key not in cache" << key;
            emit subscribeFailed(key, "Requested properties not supported by ProfileD");
            emit failed("Requested properties not supported by ProfileD");
        }
    }
}

/// Implementation of the IPropertyProvider::unsubscribe. We're not
/// keeping track on subscriptions, so we don't need to do anything.
void ProfilePlugin::unsubscribe(QSet<QString> keys)
{
}

} // end namespace

