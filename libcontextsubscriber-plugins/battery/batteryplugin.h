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

#ifndef BATTERYPLUGIN_H
#define BATTERYPLUGIN_H

#include <iproviderplugin.h> // For IProviderPlugin definition

#include <QDBusError>
#include <QDBusObjectPath>
#include <QDBusInterface>

using ContextSubscriber::IProviderPlugin;

extern "C" {
    IProviderPlugin* pluginFactory(const QString& constructionString);
}

namespace ContextSubscriberBattery
{

/*!
  \class BatteryPlugin

  \brief A libcontextsubscriber plugin for communicating with BME. 
  Provides context properties Battery.*

 */

class BatteryPlugin : public IProviderPlugin
{
    Q_OBJECT

public:
    explicit BatteryPlugin();
    virtual void subscribe(QSet<QString> keys);
    virtual void unsubscribe(QSet<QString> keys);

private slots:
    void emitValueChanged();
    void onBMEEvent();
};
}

#endif
