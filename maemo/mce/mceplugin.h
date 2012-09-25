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

#ifndef MCEPLUGIN_H
#define MCEPLUGIN_H

#include <iproviderplugin.h> // For IProviderPlugin definition

#include <QDBusError>
#include <QDBusObjectPath>
#include <QDBusInterface>
#include <QString>
#include <QSet>

class QDBusServiceWatcher;

using ContextSubscriber::IProviderPlugin;

extern "C" {
    IProviderPlugin* pluginFactory(const QString& constructionString);
}

class AsyncDBusInterface; // From libcontextsubscriber-dev
class QDBusPendingCallWatcher;

namespace ContextSubscriberMCE
{

/*!
  \class MCEPlugin

  \brief A libcontextsubscriber plugin for communicating with MCE
  over D-Bus. Provides the context properties Screen.Blanked and Device.PowerSaveMode.

 */

class MCEPlugin : public IProviderPlugin
{
    Q_OBJECT

public:
    explicit MCEPlugin();
    virtual void subscribe(QSet<QString> keys);
    virtual void unsubscribe(QSet<QString> keys);
    virtual void blockUntilReady();
    virtual void blockUntilSubscribed(const QString& key);

private Q_SLOTS:
    void getDisplayStatusFinished(QDBusPendingCallWatcher* pcw);
    void getPowerSaveFinished(QDBusPendingCallWatcher* pcw);
    void getOfflineModeFinished(QDBusPendingCallWatcher* pcw);
    void onDisplayStateChanged(QString state);
    void onPowerSaveChanged(bool on);
    void onOfflineModeChanged(uint state);
    void onInternetEnabledKeyChanged(uint state);
    void onWlanEnabledKeyChanged(uint state);
    void emitFailed(QString reason = QString("Provider not present: mce"));

private:
    void connectToMce();
    void disconnectFromMce();
    void initRadioProvider(const QString& key);
    void stopRadioProvider();
    AsyncDBusInterface* mce;
    static const QString blankedKey;
    static const QString powerSaveKey;
    static const QString offlineModeKey;
    static const QString internetEnabledKey;
    static const QString wlanEnabledKey;

    QDBusServiceWatcher* serviceWatcher; ///< For watching MCE appear and disappear
    int subscribeCount;
    QSet<QString> subscribedRadioProperties;
    QHash<QString, QDBusPendingCallWatcher*> pendingCallWatchers;
};
}

#endif
