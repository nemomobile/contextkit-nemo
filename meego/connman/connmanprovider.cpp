/*  -*- Mode: C++ -*-
 *
 * contextkit-meego
 * Copyright © 2010, Intel Corporation.
 * Copyright © 2012, Jolla.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */

#include "connmanprovider.h"
#include <QDebug>
#include <QStringList>
#include <QVariant>

#include <contextkit_props/internet.hpp>

namespace ckit = contextkit::internet;

#define DBG qDebug() << __FILE__ << ":" << __LINE__ << ":"

IProviderPlugin* pluginFactory(const QString& constructionString)
{
  Q_UNUSED(constructionString)
    return new ConnmanProvider();
}

ConnmanProvider::ConnmanProvider(): activeService(NULL)
{
  DBG << "ConnmanProvider::ConnmanProvider()";

  m_nameMapper["wifi"] = "WLAN";
  m_nameMapper["gprs"] = "GPRS";
  m_nameMapper["edge"] = "GPRS";
  m_nameMapper["umts"] = "GPRS";
  m_nameMapper["ethernet"] = "ethernet";

  m_nameMapper["offline"] = "disconnected";
  m_nameMapper["idle"] = "disconnected";
  m_nameMapper["online"] = "connected";
  m_nameMapper["ready"] = "connected";

  //hack
  m_properties[ckit::traf_in] = 20;
  m_properties[ckit::traf_out] = 20;
  m_timerId = startTimer(5*1000);

  QMetaObject::invokeMethod(this, "ready", Qt::QueuedConnection);

  m_networkManager = NetworkManagerFactory::createInstance();;
  m_properties[ckit::net_type] = map("gprs");
  m_properties[ckit::net_state] = map(m_networkManager->state());

  if(m_networkManager->defaultRoute())
  {
      activeService = m_networkManager->defaultRoute();

      connect(activeService,SIGNAL(nameChanged(QString)),this,SLOT(nameChanged(QString)));
      connect(activeService,SIGNAL(strengthChanged(uint)),this,SLOT(signalStrengthChanged(uint)));

      m_properties[ckit::sig_strength] = m_networkManager->defaultRoute()->strength();
      m_properties[ckit::net_name] = m_networkManager->defaultRoute()->name();
      m_properties[ckit::net_type] = map(activeService->type());
  }

  connect(m_networkManager, SIGNAL(stateChanged(QString)),
	  this, SLOT(stateChanged(QString)));
  connect(m_networkManager, SIGNAL(defaultRouteChanged(NetworkService*)),
          this, SLOT(defaultRouteChanged(NetworkService*)));

  //sadly, QVariant is not a registered metatype
  qRegisterMetaType<QVariant>("QVariant");

  QMetaObject::invokeMethod(this, "valueChanged", Qt::QueuedConnection,
			    Q_ARG(QString, ckit::net_type),
			    Q_ARG(QVariant, m_properties[ckit::net_type]));
  QMetaObject::invokeMethod(this, "valueChanged", Qt::QueuedConnection,
			    Q_ARG(QString, ckit::net_state),
			    Q_ARG(QVariant, m_properties[ckit::net_state]));
  QMetaObject::invokeMethod(this, "valueChanged", Qt::QueuedConnection,
                Q_ARG(QString, ckit::sig_strength),
                Q_ARG(QVariant, m_properties[ckit::sig_strength]));
  QMetaObject::invokeMethod(this, "valueChanged", Qt::QueuedConnection,
                Q_ARG(QString, ckit::net_name),
                Q_ARG(QVariant, m_properties[ckit::net_name]));
}

ConnmanProvider::~ConnmanProvider()
{
  DBG << "ConnmanProvider::~ConnmanProvider()";
}

void ConnmanProvider::subscribe(QSet<QString> keys)
{
  qDebug() << "ConnmanProvider::subscribe(" << QStringList(keys.toList()).join(", ") << ")";

  m_subscribedProperties.unite(keys);

  QMetaObject::invokeMethod(this, "emitSubscribeFinished", Qt::QueuedConnection);
  QMetaObject::invokeMethod(this, "emitChanged", Qt::QueuedConnection);
}

void ConnmanProvider::unsubscribe(QSet<QString> keys)
{
  qDebug() << "ConnmanProvider::unsubscribe(" << QStringList(keys.toList()).join(", ") << ")";

  m_subscribedProperties.subtract(keys);
}

void ConnmanProvider::timerEvent(QTimerEvent *event)
{
  Q_UNUSED(event);
  m_properties[ckit::traf_in] = qrand()*10.0 / RAND_MAX + 20;
  m_properties[ckit::traf_out] = qrand()*10.0 / RAND_MAX + 20;

  emit valueChanged(ckit::traf_in, m_properties[ckit::traf_in]);
  emit valueChanged(ckit::traf_out, m_properties[ckit::traf_out]);
}

QString ConnmanProvider::map(const QString &input) const
{
  return m_nameMapper[input];
}

void ConnmanProvider::signalStrengthChanged(uint strength)
{
    if(!activeService) return;

    m_properties[ckit::sig_strength] = strength;

    if (m_subscribedProperties.contains(ckit::sig_strength)) {
      emit valueChanged(ckit::sig_strength, QVariant(m_properties[ckit::sig_strength]));
    }
}

void ConnmanProvider::emitSubscribeFinished()
{
  foreach (QString key, m_subscribedProperties) {
    DBG << "emit subscribedFinished(" << key << ")";
    emit subscribeFinished(key);
  }
}

void ConnmanProvider::emitChanged()
{
  foreach (QString key, m_subscribedProperties) {
    DBG << "ConnmanProvider::emitChanged(): " << key << "=" << m_properties[key];
    emit valueChanged(key, QVariant(m_properties[key]));
  }
}

void ConnmanProvider::defaultRouteChanged(NetworkService *item)
{
    if(activeService)
    {
        DBG << "disconnecting from " << activeService->name();
        activeService->disconnect(this,SLOT(nameChanged(QString)));
        activeService->disconnect(this,SLOT(signalStrengthChanged(uint)));
    }

    activeService = item;

    if (item) {
        DBG << "new default route: " << item->name();
        QString ntype = map(item->type());
        if (m_properties[ckit::net_type] != ntype) {
            if (m_subscribedProperties.contains(ckit::net_type)) {
                m_properties[ckit::net_type] = ntype;
                DBG << "networkType has changed to " << ntype;
                emit valueChanged(ckit::net_type, QVariant(m_properties[ckit::net_type]));
            }
        }

        m_properties[ckit::net_name] = item->name();
        m_properties[ckit::sig_strength] = item->strength();

        DBG << "connecting to " << activeService->name();
        connect(activeService,SIGNAL(strengthChanged(uint)),this,SLOT(signalStrengthChanged(uint)));
        connect(activeService,SIGNAL(nameChanged(QString)),this,SLOT(nameChanged(QString)));
    }
    else
        m_properties[ckit::sig_strength] = 0;

    if (m_subscribedProperties.contains(ckit::sig_strength)) {
      DBG << "emit valueChanged(strength)";
      emit valueChanged(ckit::sig_strength, QVariant(m_properties[ckit::sig_strength]));
    }

    if (m_subscribedProperties.contains(ckit::net_name)) {
      DBG << "emit valueChanged(naetworkName)";
      emit valueChanged(ckit::net_name, QVariant(m_properties[ckit::net_name]));
    }
}

void ConnmanProvider::nameChanged(const QString &name)
{
    DBG << "ConnmanProvider::nameChanged(" << name << ")";
    m_properties[ckit::net_name] = name;
    if (m_subscribedProperties.contains(ckit::net_name)) {
      emit valueChanged(ckit::net_name, QVariant(m_properties[ckit::net_name]));
    }
}

void ConnmanProvider::stateChanged(QString State)
{
  DBG << "ConnmanProvider::stateChanged(" << State << ")";
  m_properties[ckit::net_state] = map(State);
  if (m_subscribedProperties.contains(ckit::net_state)) {
    emit valueChanged(ckit::net_state, QVariant(m_properties[ckit::net_state]));
  }
}
