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

#include "profileplugin.h"
#include "logging.h"
#include "profile_dbus.h"
#include <QDBusServiceWatcher>
#include <QtDBus>
// This is for getting rid of synchronous D-Bus introspection calls Qt does.
#include <asyncdbusinterface.h>

#define PROPERTY_PROFILE_NAME "Profile.Name"

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
 
    activeProfile = "";
    interface = NULL;
    callWatcher = NULL;
    serviceWatcher = NULL;

    // Emit delayed signal, so caller has a change to connect to us before.
    QMetaObject::invokeMethod(this, "ready", Qt::QueuedConnection);
}

void ProfilePlugin::getProfileCallFinishedSlot(QDBusPendingCallWatcher *call)
{
    QDBusPendingReply<QString> reply = *call;
    if (reply.isError()) {
        qDebug() << Q_FUNC_INFO << "error reply:" << reply.error().name();
        emit failed("Can not connect to profiled.");
        emit subscribeFailed(PROPERTY_PROFILE_NAME, "Can not connect to profiled.");
    } else {
        activeProfile = reply.argumentAt<0>();
        emit subscribeFinished(PROPERTY_PROFILE_NAME, QVariant(activeProfile));
    }
    delete interface;
    interface = NULL;
    callWatcher->deleteLater();
    callWatcher = NULL;
}

void ProfilePlugin::profileChanged(bool changed, bool active, QString profile, QList<MyStructure> /*keyValType*/)
{
    if (changed && active) {
        activeProfile = profile;
        emit valueChanged(PROPERTY_PROFILE_NAME, activeProfile);
    }
}

/// Implementation of the IPropertyProvider::subscribe. We don't need
/// any extra work for subscribing to keys, thus subscribe is finished
/// right away.
void ProfilePlugin::subscribe(QSet<QString> keys)
{
    contextDebug() << "subscribed keys:" << keys;

    // If would have more than one property, we would have check here if we are already connected to ProfileD.

    if (serviceWatcher == NULL) { // Is first subscribe after construction?
        // Connect to profile changed signal
        bool succ = QDBusConnection::sessionBus().connect(PROFILED_SERVICE, PROFILED_PATH, PROFILED_INTERFACE,
                                                          PROFILED_CHANGED, QString("bbsa(sss)"), this,
                                                          SLOT(profileChanged(bool, bool, QString, QList<MyStructure>)));
        if (!succ) {
            emit failed("Can not connect to dbus.");
            return;
        }

        qDebug() << "subscribe";
        serviceWatcher = new QDBusServiceWatcher(PROFILED_SERVICE, QDBusConnection::sessionBus());
        connect(serviceWatcher, SIGNAL(serviceRegistered(const QString&)),
                this, SLOT(serviceRegisteredSlot(const QString&)));
        connect(serviceWatcher, SIGNAL(serviceUnregistered(const QString&)),
                this, SLOT(serviceUnregisteredSlot(const QString&)));
    }

    // Get current profile from ProfileD
    interface = new AsyncDBusInterface(PROFILED_SERVICE, PROFILED_PATH, PROFILED_INTERFACE,
                                       QDBusConnection::sessionBus(), this);
    QDBusPendingCall async = interface->asyncCall(PROFILED_GET_PROFILE);
    callWatcher = new QDBusPendingCallWatcher(async, this);

    connect(callWatcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
            this, SLOT(getProfileCallFinishedSlot(QDBusPendingCallWatcher*)));
}

/// Implementation of the IPropertyProvider::unsubscribe. We're not
/// keeping track on subscriptions, so we don't need to do anything.
void ProfilePlugin::unsubscribe(QSet<QString> keys)
{
    delete serviceWatcher;
    serviceWatcher = NULL;
    if (!QDBusConnection::sessionBus().disconnect(PROFILED_SERVICE, PROFILED_PATH, PROFILED_INTERFACE,
                                                  PROFILED_CHANGED, QString("bbsa(sss)"), this,
                                                  SLOT(profileChanged(bool, bool, QString, QList<MyStructure>)))) {
       qDebug() << "profileplugin: cannot disconnect from dbus.";
    }
    activeProfile = "";
}

void ProfilePlugin::serviceRegisteredSlot(const QString& serviceName)
{
    emit ready();
}

void ProfilePlugin::serviceUnregisteredSlot(const QString& serviceName)
{
    emit failed("ProfileD unregistered from DBus.");
}

} // end namespace

