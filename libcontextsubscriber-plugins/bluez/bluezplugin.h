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

#ifndef BLUEZPLUGIN_H
#define BLUEZPLUGIN_H

#include "bluezinterface.h"
#include <iproviderplugin.h> // For IProviderPlugin definition

using ContextSubscriber::IProviderPlugin;

extern "C" {
    IProviderPlugin* pluginFactory(const QString& constructionString);
}

namespace ContextSubscriberBluez
{

/*!
  \class BluezPlugin

  \brief A libcontextsubscriber plugin for communicating with Bluez
  over D-Bus. Provides context properties Bluetooth.Enabled and
  Bluetooth.Visible.

 */


class BluezPlugin : public IProviderPlugin
{
    Q_OBJECT

public:
    explicit BluezPlugin();
    virtual void subscribe(QSet<QString> keys);
    virtual void unsubscribe(QSet<QString> keys);

private slots:
    void onPropertyChanged(QString key, QVariant variant);

private:
    BluezInterface bluezInterface;
    QMap<QString, QString> properties; ///< Mapping of Bluez properties to Context FW properties

};
}

#endif
