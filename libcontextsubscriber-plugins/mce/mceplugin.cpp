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

#include "mceplugin.h"
#include "sconnect.h"

#include "logging.h"
// This is for getting rid of synchronous D-Bus introspection calls Qt does.
#include <asyncdbusinterface.h>

#include <mce/dbus-names.h> // from mce-dev

#include <QDBusServiceWatcher>

/// The factory method for constructing the IPropertyProvider instance.
IProviderPlugin* pluginFactory(const QString& /*constructionString*/)
{
    // Note: it's the caller's responsibility to delete the plugin if
    // needed.
    return new ContextSubscriberMCE::MCEPlugin();
}

namespace ContextSubscriberMCE {
const QString MCEPlugin::blankedKey = "Screen.Blanked";
const QString MCEPlugin::powerSaveKey = "System.PowerSaveMode";

QDBusConnection MCEPlugin::busConnection = QDBusConnection::systemBus();

MCEPlugin::MCEPlugin() : mce(0), serviceWatcher(0), subscribeCount(0)
{
    // We're ready to take in subscriptions right away; we'll connect to MCE
    // when we get subscriptions.
    QMetaObject::invokeMethod(this, "ready", Qt::QueuedConnection);
}

void MCEPlugin::disconnectFromMce()
{
    // Signal connection is also done on key-by-key basis when doing unsubscribe; this takes care of
    // the situations when we disconnectFromMce because of errors.
    busConnection.disconnect(MCE_SERVICE, MCE_SIGNAL_PATH,
                             MCE_SIGNAL_IF, MCE_DISPLAY_SIG,
                             this, SLOT(onDisplayStateChanged(QString)));
    busConnection.disconnect(MCE_SERVICE, MCE_SIGNAL_PATH,
                             MCE_SIGNAL_IF, MCE_PSM_MODE_SIG,
                             this, SLOT(onPowerSaveChanged(QString)));
    delete mce;
    mce = 0;
    delete serviceWatcher;
    serviceWatcher = 0;
}

/// Try to establish the connection to MCE.
void MCEPlugin::connectToMce()
{
    if (mce) // already connected
        return;
    mce = new AsyncDBusInterface(MCE_SERVICE, MCE_REQUEST_PATH, MCE_REQUEST_IF, busConnection, this);
    // When MCE disappears from D-Bus, we emit failed to signal that we're
    // not able to take in subscriptions. And when MCE reappears, we emit
    // "ready". Then the upper layer will renew its subscriptions (and we
    // reconnect if needed).
    serviceWatcher = new QDBusServiceWatcher(MCE_SERVICE, busConnection);
    connect(serviceWatcher, SIGNAL(serviceRegistered(const QString&)), this, SIGNAL(ready()), Qt::QueuedConnection);
    connect(serviceWatcher, SIGNAL(serviceUnregistered(const QString&)), this, SLOT(emitFailed()));
}

/// Called when a D-Bus error occurs when processing our
/// callWithCallback.
void MCEPlugin::replyGetError(QDBusError err)
{
    // This will also emit subscribeFailed for all our keys. Note: no recovery; the plugin will stay
    // at failed state forever.
    disconnectFromMce();
    subscribeCount = 0;
    emit failed("Cannot connect to MCE: " + err.message());
}

/// Callback for "get display status"
void MCEPlugin::replyGetDisplayState(QString state)
{
    bool blanked = (state == "off");
    emit subscribeFinished(blankedKey, QVariant(blanked));
}

/// Callback for "get powersave status"
void MCEPlugin::replyGetPowerSave(bool on)
{
    emit subscribeFinished(powerSaveKey, QVariant(on));
}

/// Connected to the D-Bus signal from MCE.
void MCEPlugin::onDisplayStateChanged(QString state)
{
    bool blanked = (state == "off");
    emit valueChanged(blankedKey, QVariant(blanked));
}

/// Connected to the D-Bus signal from MCE.
void MCEPlugin::onPowerSaveChanged(bool on)
{
    emit valueChanged(powerSaveKey, QVariant(on));
}

/// Implementation of the IPropertyProvider::subscribe.
void MCEPlugin::subscribe(QSet<QString> keys)
{
    // ensure the connection; it's safe to call this multiple times
    connectToMce();

    if (keys.contains(blankedKey)) {
        busConnection.connect(MCE_SERVICE, MCE_SIGNAL_PATH, MCE_SIGNAL_IF, MCE_DISPLAY_SIG,
                              this, SLOT(onDisplayStateChanged(QString)));
        // this will emit subscribeFinished when done
        mce->callWithCallback(MCE_DISPLAY_STATUS_GET, QList<QVariant>(),
                              this, SLOT(replyGetDisplayState(QString)), SLOT(replyGetError(QDBusError)));
        ++subscribeCount;
    }
    if (keys.contains(powerSaveKey)) {
        busConnection.connect(MCE_SERVICE, MCE_SIGNAL_PATH, MCE_SIGNAL_IF, MCE_PSM_MODE_SIG,
                              this, SLOT(onPowerSaveChanged(bool)));

        // this will emit subscribeFinished when done
        mce->callWithCallback(MCE_PSM_MODE_GET, QList<QVariant>(),
                              this, SLOT(replyGetPowerSave(bool)), SLOT(replyGetError(QDBusError)));
        ++subscribeCount;
    }
}

/// Implementation of the IPropertyProvider::unsubscribe.
void MCEPlugin::unsubscribe(QSet<QString> keys)
{
    if (keys.contains(blankedKey)) {
        busConnection.disconnect(MCE_SERVICE, MCE_SIGNAL_PATH,
                                 MCE_SIGNAL_IF, MCE_DISPLAY_SIG,
                                 this, SLOT(onDisplayStateChanged(QString)));
        --subscribeCount;
    }
    if (keys.contains(powerSaveKey)) {
        busConnection.disconnect(MCE_SERVICE, MCE_SIGNAL_PATH,
                                 MCE_SIGNAL_IF, MCE_PSM_MODE_SIG,
                                 this, SLOT(onPowerSaveChanged(QString)));
        --subscribeCount;
    }
    if (subscribeCount == 0)
        disconnectFromMce();
}

/// For emitting the failed() signal in a delayed way.
void MCEPlugin::emitFailed(QString reason)
{
    disconnectFromMce();
    subscribeCount = 0;
    emit failed(reason);
}

} // end namespace

