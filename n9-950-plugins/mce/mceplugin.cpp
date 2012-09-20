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
#include <mce/mode-names.h> // from mce-dev

#include <QDBusServiceWatcher>
#include <QDBusPendingReply>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingCall>

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
const QString MCEPlugin::offlineModeKey = "System.OfflineMode";
const QString MCEPlugin::internetEnabledKey = "System.InternetEnabled";
const QString MCEPlugin::wlanEnabledKey = "System.WlanEnabled";

#define MCE_PLUGIN_BUS QDBusConnection::systemBus()

MCEPlugin::MCEPlugin() : mce(0), serviceWatcher(0), subscribeCount(0)
{
    // We're ready to take in subscriptions right away; we'll connect to MCE
    // when we get subscriptions.
    QMetaObject::invokeMethod(this, "ready", Qt::QueuedConnection);
}

void MCEPlugin::disconnectFromMce()
{
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
    mce = new AsyncDBusInterface(MCE_SERVICE, MCE_REQUEST_PATH, MCE_REQUEST_IF, MCE_PLUGIN_BUS, this);
    // When MCE disappears from D-Bus, we emit failed to signal that we're
    // not able to take in subscriptions. And when MCE reappears, we emit
    // "ready". Then the upper layer will renew its subscriptions (and we
    // reconnect if needed).
    serviceWatcher = new QDBusServiceWatcher(MCE_SERVICE, MCE_PLUGIN_BUS);
    connect(serviceWatcher, SIGNAL(serviceRegistered(const QString&)),
            this, SIGNAL(ready()), Qt::QueuedConnection);
    connect(serviceWatcher, SIGNAL(serviceUnregistered(const QString&)),
            this, SLOT(emitFailed()));
}

/// Callback for "get display status"
void MCEPlugin::getDisplayStatusFinished(QDBusPendingCallWatcher* pcw)
{
    QDBusPendingReply<QString> reply = *pcw;
    if (reply.isError()) {
        if (reply.error().type() == QDBusError::ServiceUnknown) {
            // We need to fail explicitly so that we can emit ready() when the
            // provider is started. (We have already emitted ready(), and
            // emitting ready() 2 times has no effect.)
            Q_EMIT failed("Provider not present: mce");
        }
        else
            Q_EMIT subscribeFailed(blankedKey, reply.error().message());
    }
    else {
        bool blanked = (reply.argumentAt<0>() == "off");
        // emitting valueChanged is needed since subscribeFinished is queued,
        // and we might need a value immediately (if we blockUntilSubscribed).
        Q_EMIT valueChanged(blankedKey, QVariant(blanked));
        Q_EMIT subscribeFinished(blankedKey);
    }
    pendingCallWatchers.remove(blankedKey);
    pcw->deleteLater();
}

/// Callback for "get powersave status"
void MCEPlugin::getPowerSaveFinished(QDBusPendingCallWatcher* pcw)
{
    QDBusPendingReply<bool> reply = *pcw;
    if (reply.isError()) {
        if (reply.error().type() == QDBusError::ServiceUnknown) {
            // We need to fail explicitly so that we can emit ready() when the
            // provider is started. (We have already emitted ready(), and
            // emitting ready() 2 times has no effect.)
            Q_EMIT failed("Provider not present: mce");
        }
        else
            Q_EMIT subscribeFailed(powerSaveKey, reply.error().message());
    }
    else {
        bool on = reply.argumentAt<0>();
        // emitting valueChanged is needed since subscribeFinished is queued,
        // and we might need a value immediately (if we blockUntilSubscribed).
        Q_EMIT valueChanged(powerSaveKey, QVariant(on));
        Q_EMIT subscribeFinished(powerSaveKey);
    }
    pendingCallWatchers.remove(powerSaveKey);
    pcw->deleteLater();
}

/// Callback for "get offline mode status"
void MCEPlugin::getOfflineModeFinished(QDBusPendingCallWatcher* pcw)
{
    QDBusPendingReply<uint> reply = *pcw;
    if (reply.isError()) {
        if (reply.error().type() == QDBusError::ServiceUnknown) {
            // We need to fail explicitly so that we can emit ready() when the
            // provider is started. (We have already emitted ready(), and
            // emitting ready() 2 times has no effect.)
            Q_EMIT failed("Provider not present: mce");
        }
        else {
            Q_EMIT subscribeFailed(offlineModeKey, reply.error().message());
            Q_EMIT subscribeFailed(internetEnabledKey, reply.error().message());
	    Q_EMIT subscribeFailed(wlanEnabledKey, reply.error().message());
	}
    }
    else {
        bool offline = !(reply.argumentAt<0>() & MCE_RADIO_STATE_CELLULAR);
	bool wlan = (reply.argumentAt<0>() & MCE_RADIO_STATE_WLAN);
	bool internet = (reply.argumentAt<0>() & MCE_RADIO_STATE_MASTER);

        // emitting valueChanged is needed since subscribeFinished is queued,
        // and we might need a value immediately (if we blockUntilSubscribed).
	if (pendingCallWatchers.contains(offlineModeKey)) {
	    Q_EMIT valueChanged(offlineModeKey, QVariant(offline));
	    Q_EMIT subscribeFinished(offlineModeKey);
	}

	if (pendingCallWatchers.contains(internetEnabledKey)) {
	    Q_EMIT valueChanged(internetEnabledKey, QVariant(internet));
	    Q_EMIT subscribeFinished(internetEnabledKey);
	}

	if (pendingCallWatchers.contains(wlanEnabledKey)) {
	    Q_EMIT valueChanged(wlanEnabledKey, QVariant(wlan));
	    Q_EMIT subscribeFinished(wlanEnabledKey);
	}
    }

    pendingCallWatchers.remove(pendingCallWatchers.key(pcw));
    pcw->deleteLater();
}

/// Connected to the D-Bus signal from MCE.
void MCEPlugin::onDisplayStateChanged(QString state)
{
    bool blanked = (state == "off");
    Q_EMIT valueChanged(blankedKey, QVariant(blanked));
}

/// Connected to the D-Bus signal from MCE.
void MCEPlugin::onPowerSaveChanged(bool on)
{
    Q_EMIT valueChanged(powerSaveKey, QVariant(on));
}

/// Connected to the D-Bus signal from MCE.
void MCEPlugin::onOfflineModeChanged(uint state)
{

    bool offline = !(state & MCE_RADIO_STATE_CELLULAR);
    bool wlan = state & MCE_RADIO_STATE_WLAN;
    bool internet = state & MCE_RADIO_STATE_MASTER;

    if (subscribedRadioProperties.contains(offlineModeKey))
	Q_EMIT valueChanged(offlineModeKey, QVariant(offline));

    if (subscribedRadioProperties.contains(internetEnabledKey))
	Q_EMIT valueChanged(internetEnabledKey, QVariant(internet));

    if (subscribedRadioProperties.contains(wlanEnabledKey))
	Q_EMIT valueChanged(wlanEnabledKey, QVariant(wlan));
}

/// Implementation of the IPropertyProvider::subscribe.
void MCEPlugin::subscribe(QSet<QString> keys)
{
    // ensure the connection; it's safe to call this multiple times
    connectToMce();

    if (keys.contains(blankedKey)) {
        MCE_PLUGIN_BUS.connect(MCE_SERVICE, MCE_SIGNAL_PATH,
                              MCE_SIGNAL_IF, MCE_DISPLAY_SIG,
                              this, SLOT(onDisplayStateChanged(QString)));
        // this will emit subscribeFinished when done
        QDBusPendingCallWatcher* pcw = new QDBusPendingCallWatcher(
            mce->asyncCall(MCE_DISPLAY_STATUS_GET));
        sconnect(pcw, SIGNAL(finished(QDBusPendingCallWatcher*)),
                 this, SLOT(getDisplayStatusFinished(QDBusPendingCallWatcher*)));
        pendingCallWatchers.insert(blankedKey, pcw);

        ++subscribeCount;
    }
    if (keys.contains(powerSaveKey)) {
        MCE_PLUGIN_BUS.connect(MCE_SERVICE, MCE_SIGNAL_PATH,
                              MCE_SIGNAL_IF, MCE_PSM_STATE_SIG,
                              this, SLOT(onPowerSaveChanged(bool)));

        // this will emit subscribeFinished when done
        QDBusPendingCallWatcher* pcw = new QDBusPendingCallWatcher(
            mce->asyncCall(MCE_PSM_STATE_GET));
        sconnect(pcw, SIGNAL(finished(QDBusPendingCallWatcher*)),
                 this, SLOT(getPowerSaveFinished(QDBusPendingCallWatcher*)));
        pendingCallWatchers.insert(powerSaveKey, pcw);

        ++subscribeCount;
    }

    if (keys.contains(offlineModeKey)) {
	initRadioProvider(offlineModeKey);
	subscribedRadioProperties.insert(offlineModeKey);
	++subscribeCount;
    }

    if (keys.contains(internetEnabledKey)) {
	initRadioProvider(internetEnabledKey);
	subscribedRadioProperties.insert(internetEnabledKey);
	++subscribeCount;
    }

    if (keys.contains(wlanEnabledKey)) {
	initRadioProvider(wlanEnabledKey);
	subscribedRadioProperties.insert(wlanEnabledKey);
	++subscribeCount;
    }
}

void MCEPlugin::initRadioProvider(const QString& key)
{
    if (subscribedRadioProperties.isEmpty())
        MCE_PLUGIN_BUS.connect(MCE_SERVICE, MCE_SIGNAL_PATH,
                              MCE_SIGNAL_IF, MCE_RADIO_STATES_SIG,
                              this, SLOT(onOfflineModeChanged(uint)));

    if (!subscribedRadioProperties.contains(key)) {
        // this will emit subscribeFinished when done
        QDBusPendingCallWatcher* pcw = new QDBusPendingCallWatcher(
            mce->asyncCall(MCE_RADIO_STATES_GET));
        sconnect(pcw, SIGNAL(finished(QDBusPendingCallWatcher*)),
                 this, SLOT(getOfflineModeFinished(QDBusPendingCallWatcher*)));
        pendingCallWatchers.insert(key, pcw);
    }
}

/// Implementation of the IPropertyProvider::unsubscribe.
void MCEPlugin::unsubscribe(QSet<QString> keys)
{
    // The Subscribe call can still be in progress.  In that case we'll emit
    // subscribeFinished later, and the upper layer should just deal with it.
    if (keys.contains(blankedKey)) {
        MCE_PLUGIN_BUS.disconnect(MCE_SERVICE, MCE_SIGNAL_PATH,
                                 MCE_SIGNAL_IF, MCE_DISPLAY_SIG,
                                 this, SLOT(onDisplayStateChanged(QString)));
        --subscribeCount;
    }
    if (keys.contains(powerSaveKey)) {
        MCE_PLUGIN_BUS.disconnect(MCE_SERVICE, MCE_SIGNAL_PATH,
                                 MCE_SIGNAL_IF, MCE_PSM_STATE_SIG,
                                 this, SLOT(onPowerSaveChanged(bool)));
        --subscribeCount;
    }

    if (keys.contains(offlineModeKey)) {
	subscribedRadioProperties.remove(offlineModeKey);
        stopRadioProvider();
        --subscribeCount;
    }

    if (keys.contains(internetEnabledKey)) {
	subscribedRadioProperties.remove(internetEnabledKey);
        stopRadioProvider();
        --subscribeCount;
    }

    if (keys.contains(wlanEnabledKey)) {
	subscribedRadioProperties.remove(wlanEnabledKey);
        stopRadioProvider();
        --subscribeCount;
    }

    if (subscribeCount == 0)
        disconnectFromMce();
}

void MCEPlugin::stopRadioProvider()
{
    if (subscribedRadioProperties.isEmpty()) {

        MCE_PLUGIN_BUS.disconnect(MCE_SERVICE, MCE_SIGNAL_PATH,
                                 MCE_SIGNAL_IF, MCE_RADIO_STATES_SIG,
                                 this, SLOT(onOfflineModeChanged(uint)));
    }
}


void MCEPlugin::blockUntilReady()
{
    // This plugin is optimistic, it's ready even if we don't know whether MCE
    // is running.
    Q_EMIT ready();
}

void MCEPlugin::blockUntilSubscribed(const QString& key)
{
    if (pendingCallWatchers.contains(key)) {
        QDBusPendingCallWatcher* pcw = pendingCallWatchers.value(key);
        pcw->waitForFinished();
    }
}

/// For emitting the failed() signal in a delayed way.  When the plugin has emitted failed(), it's
/// supposed to be in the "nothing subscribed" state.
void MCEPlugin::emitFailed(QString reason)
{
    // Don't disconnectFromMce here; that would kill the D-Bus name watcher and we wouldn't notice
    // when MCE comes back.
    MCE_PLUGIN_BUS.disconnect(MCE_SERVICE, MCE_SIGNAL_PATH,
                             MCE_SIGNAL_IF, MCE_DISPLAY_SIG,
                             this, SLOT(onDisplayStateChanged(QString)));
    MCE_PLUGIN_BUS.disconnect(MCE_SERVICE, MCE_SIGNAL_PATH,
                             MCE_SIGNAL_IF, MCE_PSM_STATE_SIG,
                             this, SLOT(onPowerSaveChanged(bool)));
    MCE_PLUGIN_BUS.disconnect(MCE_SERVICE, MCE_SIGNAL_PATH,
                             MCE_SIGNAL_IF, MCE_RADIO_STATES_SIG,
                             this, SLOT(onOfflineModeChanged(uint)));

    subscribeCount = 0;
    Q_EMIT failed(reason);
}

} // end namespace

