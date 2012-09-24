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

#include "bluetoothprovider.h"
#include <QDebug>
#include <QStringList>
#include <QVariant>
#include <contextkit_props/bluetooth.hpp>

IProviderPlugin* pluginFactory(const QString& constructionString)
{
    Q_UNUSED(constructionString)
    return new BluetoothProvider();
}

BluetoothProvider::BluetoothProvider()
{
  qDebug() << "BluetoothProvider::BluetoothProvider()";

  QMetaObject::invokeMethod(this, "ready", Qt::QueuedConnection);

  m_bluetoothDevices = new BluetoothDevicesModel(this);

  connect(m_bluetoothDevices, SIGNAL(connectedChanged(bool)),
      this, SLOT(connectedChanged(bool)));
  connect(m_bluetoothDevices, SIGNAL(discoverableChanged(bool)),
      this, SLOT(discoverableChanged(bool)));
  connect(m_bluetoothDevices, SIGNAL(poweredChanged(bool)),
      this, SLOT(poweredChanged(bool)));

  //sadly, QVariant is not a registered metatype
  qRegisterMetaType<QVariant>("QVariant");

  connectedChanged(m_bluetoothDevices->connected());
  poweredChanged(m_bluetoothDevices->powered());
  discoverableChanged(m_bluetoothDevices->discoverable());
}

BluetoothProvider::~BluetoothProvider()
{
  qDebug() << "BluetoothProvider::~BluetoothProvider()";
}

void BluetoothProvider::subscribe(QSet<QString> keys)
{
  qDebug() << "BluetoothProvider::subscribe(" << QStringList(keys.toList()).join(", ") << ")";

  m_subscribedProperties.unite(keys);

  QMetaObject::invokeMethod(this, "emitSubscribeFinished", Qt::QueuedConnection);
  QMetaObject::invokeMethod(this, "emitChanged", Qt::QueuedConnection);
}

void BluetoothProvider::unsubscribe(QSet<QString> keys)
{
  qDebug() << "BluetoothProvider::unsubscribe(" << QStringList(keys.toList()).join(", ") << ")";

  m_subscribedProperties.subtract(keys);
}


void BluetoothProvider::emitSubscribeFinished()
{
  foreach (QString key, m_subscribedProperties) {
    emit subscribeFinished(key);
  }
}

void BluetoothProvider::updateProps()
{
  m_properties[bluetooth_is_enabled] = m_bluetoothDevices->powered();
  m_properties[bluetooth_is_visible] = m_bluetoothDevices->discoverable();
}

void BluetoothProvider::connectedChanged(bool value)
{
  emit valueChanged(bluetooth_is_connected, value);
}

void BluetoothProvider::discoverableChanged(bool value)
{
  emit valueChanged(bluetooth_is_visible, value);
}

void BluetoothProvider::poweredChanged(bool value)
{
  emit valueChanged(bluetooth_is_enabled, value);
}
