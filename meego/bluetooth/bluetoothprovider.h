/*  -*- Mode: C++ -*-
 *
 * contextkit-meego
 * Copyright © 2010, Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */

#ifndef CONNMANPROVIDER_H
#define CONNMANPROVIDER_H

#include <iproviderplugin.h>
#include <bluetoothdevicemodel.h>
#include <QSet>
#include <QMap>
#include <QString>
#include <QVariant>


using ContextSubscriber::IProviderPlugin;

extern "C"
{
  IProviderPlugin* pluginFactory(const QString& constructionString);
}

class BluetoothProvider : public IProviderPlugin
{
  Q_OBJECT;

public:
  BluetoothProvider();
  virtual ~BluetoothProvider();

  virtual void subscribe(QSet<QString> keys);
  virtual void unsubscribe(QSet<QString> keys);
  virtual void blockUntilReady() {}
  virtual void blockUntilSubscribed(const QString&) {}

private:

  void updateProps();

  QSet<QString> m_subscribedProperties;
  QVariantMap m_properties;
  BluetoothDevicesModel *m_bluetoothDevices;

private slots:
  void emitSubscribeFinished();
  void connectedChanged(bool);
  void discoverableChanged(bool);
  void poweredChanged(bool);
};

#endif //CONNMANPROVIDER_H
