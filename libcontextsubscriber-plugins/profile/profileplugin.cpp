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

ProfilePlugin::ProfilePlugin()
    : interface(0), callWatcher(0), serviceWatcher(0)
{
    qDBusRegisterMetaType<MyStructure>();
    qDBusRegisterMetaType<QList<MyStructure > >();

    // Emit delayed signal, so caller has a change to connect to us before.
    QMetaObject::invokeMethod(this, "ready", Qt::QueuedConnection);
}

void ProfilePlugin::getProfileCallFinishedSlot(QDBusPendingCallWatcher *call)
{
    QDBusPendingReply<QString> reply = *call;
    if (reply.isError()) {
        qDebug() << Q_FUNC_INFO << "error reply:" << reply.error().name();
        Q_EMIT failed("Can not connect to profiled.");
    } else {
        activeProfile = reply.argumentAt<0>();
        Q_EMIT subscribeFinished(PROPERTY_PROFILE_NAME, QVariant(activeProfile));
    }
    if (call == callWatcher)
        callWatcher = 0;
    call->deleteLater();
}

void ProfilePlugin::profileChanged(bool changed, bool active, QString profile, QList<MyStructure> /*keyValType*/)
{
    if (changed && active) {
        activeProfile = profile;
        Q_EMIT valueChanged(PROPERTY_PROFILE_NAME, activeProfile);
    }
}

/// Implementation of the IPropertyProvider::subscribe. Connect to the profile
/// changed signal, and check the current profile with an asynchronous D-Bus
/// call. subscribeFinished() will be emitted when the call has finished.
void ProfilePlugin::subscribe(QSet<QString> keys)
{
    contextDebug() << "subscribed keys:" << keys;

    // If this plugin had more than one property, we would have check here if we
    // are already connected to ProfileD.

    if (serviceWatcher == 0) { // Is first subscribe after construction (or unsubscribe)?
        // Connect to profile changed signal
        bool succ = QDBusConnection::sessionBus().connect(PROFILED_SERVICE, PROFILED_PATH, PROFILED_INTERFACE,
                                                          PROFILED_CHANGED, QString("bbsa(sss)"), this,
                                                          SLOT(profileChanged(bool, bool, QString, QList<MyStructure>)));
        if (!succ) {
            Q_EMIT failed("Can not connect to dbus.");
            return;
        }

        serviceWatcher = new QDBusServiceWatcher(PROFILED_SERVICE, QDBusConnection::sessionBus());
        connect(serviceWatcher, SIGNAL(serviceRegistered(const QString&)),
                this, SLOT(serviceRegisteredSlot(const QString&)));
        connect(serviceWatcher, SIGNAL(serviceUnregistered(const QString&)),
                this, SLOT(serviceUnregisteredSlot(const QString&)));

        interface = new AsyncDBusInterface(PROFILED_SERVICE, PROFILED_PATH, PROFILED_INTERFACE,
                                           QDBusConnection::sessionBus(), this);
    }
    // The empty else-branch is executed when the following happens:
    // The provider goes away
    // The provider comes back and the plugin emits ready
    // The upper layer resubscribes

    // Get current profile from ProfileD.  It is possible that there was a
    // previous call ongoing, here we override a pointer to it (callWatcher).
    // But the slot will delete it nicely.
    QDBusPendingCall async = interface->asyncCall(PROFILED_GET_PROFILE);
    callWatcher = new QDBusPendingCallWatcher(async, this);

    connect(callWatcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
            this, SLOT(getProfileCallFinishedSlot(QDBusPendingCallWatcher*)));
}

/// Implementation of the IPropertyProvider::unsubscribe. Stop listening to
/// the profile changed signal and profiled disappearing from D-Bus.
void ProfilePlugin::unsubscribe(QSet<QString> keys)
{
    delete serviceWatcher;
    serviceWatcher = 0;
    delete interface;
    interface = 0;
    if (!QDBusConnection::sessionBus().disconnect(PROFILED_SERVICE, PROFILED_PATH, PROFILED_INTERFACE,
                                                  PROFILED_CHANGED, QString("bbsa(sss)"), this,
                                                  SLOT(profileChanged(bool, bool, QString, QList<MyStructure>)))) {
        contextWarning() << "profileplugin: cannot disconnect from dbus.";
    }
    activeProfile = "";
}

void ProfilePlugin::blockUntilReady()
{
    Q_EMIT ready();
}

void ProfilePlugin::blockUntilSubscribed(const QString& key)
{
    // If there is a pending GetProfile call ongoing, wait until it finishes.
    if (callWatcher)
        callWatcher->waitForFinished();
}

void ProfilePlugin::serviceRegisteredSlot(const QString& /*serviceName*/)
{
    Q_EMIT ready();
}

void ProfilePlugin::serviceUnregisteredSlot(const QString& /*serviceName*/)
{
    Q_EMIT failed("ProfileD unregistered from DBus.");
    // We are still connected to the "service registered" signal, so we'll
    // notice if profiled comes back. We also keep the connection to the
    // profile changed signal. When profiled comes back, we emit ready and the
    // upper layer re-subscribes. In subscribe, serviceWatcher is not
    // recreated.
}

} // end namespace

