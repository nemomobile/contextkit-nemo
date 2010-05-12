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

#include "displaystateplugin.h"
#include "sconnect.h"

#include "logging.h"
// This is for getting rid of synchronous D-Bus introspection calls Qt does.
#include <asyncdbusinterface.h>

#include <QDBusServiceWatcher>

/// The factory method for constructing the IPropertyProvider instance.
IProviderPlugin* pluginFactory(const QString& /*constructionString*/)
{
    // Note: it's the caller's responsibility to delete the plugin if
    // needed.
    return new ContextSubscriberDisplayState::DisplayStatePlugin();
}

namespace ContextSubscriberDisplayState {
const QString DisplayStatePlugin::blankedKey = "Screen.Blanked";

const QString DisplayStatePlugin::serviceName = "com.nokia.mce";
const QString DisplayStatePlugin::signalObjectPath = "/com/nokia/mce/signal";
const QString DisplayStatePlugin::signalInterface = "com.nokia.mce.signal";
const QString DisplayStatePlugin::signal = "display_status_ind";

const QString DisplayStatePlugin::getObjectPath = "/com/nokia/mce/request";
const QString DisplayStatePlugin::getInterface = "com.nokia.mce.request";
const QString DisplayStatePlugin::getFunction = "get_display_status";
QDBusConnection DisplayStatePlugin::busConnection = QDBusConnection::systemBus();

DisplayStatePlugin::DisplayStatePlugin() : mce(0), serviceWatcher(0)
{
    // We're ready to take in subscriptions right away; we'll connect to MCE
    // when we get subscriptions.
    QMetaObject::invokeMethod(this, "ready", Qt::QueuedConnection);
}

void DisplayStatePlugin::disconnectFromMce()
{
    // Disconnect D-Bus signal
    busConnection.disconnect(serviceName, signalObjectPath,
                             signalInterface, signal,
                             this, SLOT(onDisplayStateChanged(QString)));

    delete mce;
    mce = 0;
    delete serviceWatcher;
    serviceWatcher = 0;
}

/// Try to establish the connection to MCE.
void DisplayStatePlugin::connectToMce()
{
    disconnectFromMce();
    busConnection.connect(serviceName, signalObjectPath, signalInterface, signal,
                          this, SLOT(onDisplayStateChanged(QString)));

    mce = new AsyncDBusInterface(serviceName, getObjectPath, getInterface, busConnection, this);
    mce->callWithCallback(getFunction, QList<QVariant>(),
                          this, SLOT(replyGet(QString)), SLOT(replyGetError(QDBusError)));

    // When MCE disappears from D-Bus, we emit failed to signal that we're
    // not able to take in subscriptions. And when MCE reappears, we emit
    // "ready". Then the upper layer will renew its subscriptions (and we
    // reconnect if needed).
    serviceWatcher = new QDBusServiceWatcher(serviceName, busConnection);
    connect(serviceWatcher, SIGNAL(serviceRegistered(const QString&)), this, SIGNAL(ready()), Qt::QueuedConnection);
    connect(serviceWatcher, SIGNAL(serviceUnregistered(const QString&)), this, SLOT(emitFailed()));
}

/// Called when a D-Bus error occurs when processing our
/// callWithCallback.
void DisplayStatePlugin::replyGetError(QDBusError err)
{
    // This will also emit subscribeFailed for all our keys. Note: no recovery; the plugin will stay
    // at failed state forever.
    disconnectFromMce();
    emit failed("Cannot connect to MCE: " + err.message());
}

/// Called when the DefaultAdapter D-Bus call is done.
void DisplayStatePlugin::replyGet(QString state)
{
    bool blanked = (state == "off");
    emit subscribeFinished(blankedKey, QVariant(blanked));
}

/// Connected to the D-Bus signal from MCE.
void DisplayStatePlugin::onDisplayStateChanged(QString state)
{
    bool blanked = (state == "off");
    emit valueChanged(blankedKey, QVariant(blanked));
}

/// Implementation of the IPropertyProvider::subscribe.
void DisplayStatePlugin::subscribe(QSet<QString> keys)
{
    // This will try to get the display state asynchronously and emit subscribeFinished when done.
    connectToMce();
}

/// Implementation of the IPropertyProvider::unsubscribe.
void DisplayStatePlugin::unsubscribe(QSet<QString> keys)
{
    disconnectFromMce();
}

/// For emitting the failed() signal in a delayed way.
void DisplayStatePlugin::emitFailed(QString reason)
{
    emit failed(reason);
}

} // end namespace

