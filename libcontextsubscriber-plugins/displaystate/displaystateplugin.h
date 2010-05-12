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

#ifndef DISPLAYSTATEPLUGIN_H
#define DISPLAYSTATEPLUGIN_H

#include <iproviderplugin.h> // For IProviderPlugin definition

#include <QDBusError>
#include <QDBusObjectPath>
#include <QDBusInterface>
#include <QString>

class QDBusServiceWatcher;

using ContextSubscriber::IProviderPlugin;

extern "C" {
    IProviderPlugin* pluginFactory(const QString& constructionString);
}

class AsyncDBusInterface; // From libcontextsubscriber-dev

namespace ContextSubscriberDisplayState
{

/*!
  \class DisplayStatePlugin

  \brief A libcontextsubscriber plugin for communicating with MCE
  over D-Bus. Provides the context property Screen.Blanked.

 */

class DisplayStatePlugin : public IProviderPlugin
{
    Q_OBJECT

public:
    explicit DisplayStatePlugin();
    virtual void subscribe(QSet<QString> keys);
    virtual void unsubscribe(QSet<QString> keys);

private Q_SLOTS:
    void replyGetError(QDBusError err);
    void replyGet(QString state);
    void onDisplayStateChanged(QString state);
    void emitFailed(QString reason = QString("MCE left D-Bus"));

private:
    void connectToMce();
    void disconnectFromMce();
    AsyncDBusInterface* mce;
    // Specification of the D-Bus signal we listen to
    static const QString blankedKey;
    static const QString serviceName;
    static const QString signalObjectPath;
    static const QString signalInterface;
    static const QString signal;
    static const QString getObjectPath;
    static const QString getInterface;
    static const QString getFunction;
    static QDBusConnection busConnection;

    enum ConnectionStatus {NotConnected, Connecting, Connected};
    ConnectionStatus status; ///< Whether we're currently connected to MCE
    QDBusServiceWatcher* serviceWatcher; ///< For watching MCE appear and disappear

};
}

#endif
