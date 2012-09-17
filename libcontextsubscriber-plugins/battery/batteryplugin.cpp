/*
 * Copyright (C) 2010 Nokia Corporation.
 *
 * Contact: Marius Vollmer <marius.vollmer@nokia.com>
 *
 * Copyright (C) 2012 Jolla Ltd.
 * Contact: Denis Zalevskiy <denis.zalevskiy@jollamobile.com>
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

#include "batteryplugin.hpp"
#include <fcntl.h>
#include <sys/inotify.h>
#include <sys/ioctl.h>
#include <QSocketNotifier>
#include <QFileSystemWatcher>
#include <QVarLengthArray>
#include <QFile>
#include <QStringList>
#include <QSet>
#include <QtDebug>

Q_DECLARE_METATYPE(QSet<QString>);

#define ON_BATTERY       "Battery.OnBattery"
#define LOW_BATTERY      "Battery.LowBattery"
#define CHARGE_PERCENT   "Battery.ChargePercentage"
#define CHARGE_BARS      "Battery.ChargeBars"
#define TIME_UNTIL_LOW   "Battery.TimeUntilLow"
#define TIME_UNTIL_FULL  "Battery.TimeUntilFull"
#define IS_CHARGING      "Battery.IsCharging"
#define NANOSECS_PER_MIN (60 * 1000 * 1000LL)

IProviderPlugin* pluginFactory(const QString& /*constructionString*/)
{
    return new ContextSubscriberBattery::BatteryPlugin();
}

namespace ContextSubscriberBattery {

BatteryPlugin::BatteryPlugin()
{
    xchg = bme_xchg_open();
    if (xchg == BME_XCHG_INVAL) {
        QMetaObject::invokeMethod(this, "failed", Qt::QueuedConnection,
                                  Q_ARG(QString, "bme_xchg_open failed"));
        return;
    }
    fcntl(bme_xchg_inotify_desc(xchg), F_SETFD, FD_CLOEXEC);
    sn.reset(new QSocketNotifier(bme_xchg_inotify_desc(xchg),
                                 QSocketNotifier::Read, this));
    sn->setEnabled(false);
    connect(sn.data(), SIGNAL(activated(int)),
            this, SLOT(onBMEEvent()));
    QMetaObject::invokeMethod(this, "ready", Qt::QueuedConnection);
}

BatteryPlugin::~BatteryPlugin()
{
}

/// The provider source of the battery properties is initialised only on the
/// first subscription. Initialisation means adding watcher to BME_EVENT
void BatteryPlugin::subscribe(QSet<QString> keys)
{
    if (subscribedProperties.isEmpty()) {
        qRegisterMetaType<QSet<QString> >("QSet<QString>");
        initProviderSource();
        readBatteryValues();
    }
    emitSubscribeFinished(keys);
    subscribedProperties.unite(keys);
}

/// Implementation of the IPropertyProvider::unsubscribe.
/// Properties to unsubscribe are removed from the cache.
/// The provider source is closed and cleaned up when no properties
/// remain in the cache.
void BatteryPlugin::unsubscribe(QSet<QString> keys)
{
    subscribedProperties.subtract(keys);
    if (subscribedProperties.isEmpty())
        cleanProviderSource();
}

void BatteryPlugin::blockUntilReady()
{
    if (xchg < 0)
        Q_EMIT failed("bme_eopen failed");
    else
        Q_EMIT ready();
}

void BatteryPlugin::blockUntilSubscribed(const QString& key)
{
}

/// Start to watch the provider source BME_EVENT on first
/// subscription or when the source has been deleted or moved
bool BatteryPlugin::initProviderSource()
{
    int rc = bme_inotify_watch_add(xchg);
    if (rc < 0) {
        QMetaObject::invokeMethod(this, "failed", Qt::QueuedConnection,
                                  Q_ARG(QString, "Battery plugin failed to add"
                                        " watcher on BME_EVENT"));
        return false;
    }
    sn->setEnabled(true);
    return true;
}

/// Called when the provicer source BME_EVENT is deleted, moved or
/// when all properties have been unsubscribed watcher is removed
void BatteryPlugin::cleanProviderSource()
{
    bme_inotify_watch_rm(xchg);
    sn->setEnabled(false);
}

/// Called when provider source has been modified and new values are available
bool BatteryPlugin::readBatteryValues()
{
    bme_stat_t st;
    int sd = -1;

    if ((sd = bme_open()) < 0) {
        qDebug() << "Cannot open socket connected to BME server";
        return false;
    }

    if (bme_stat_get(sd, &st) < 0) {
        qDebug() << "Cannot get BME statistics";
        return false;
    }

    propertyCache[IS_CHARGING]
        = (st[bme_stat_charger_state] == bme_charging_state_started
           && st[bme_stat_bat_state] != bme_bat_state_full);

    propertyCache[ON_BATTERY]
        = (st[bme_stat_charger_state] != bme_charger_state_connected);
    propertyCache[LOW_BATTERY]
        = (st[bme_stat_bat_state] == bme_bat_state_low);

    propertyCache[CHARGE_PERCENT] = st[bme_stat_bat_pct_remain];

    if (st[bme_stat_bat_units_max] != 0) {
        QList<QVariant> list;
        list << QVariant(st[bme_stat_bat_units_now])
             << QVariant(st[bme_stat_bat_units_max]);
        propertyCache[CHARGE_BARS] = list;
    } else {
        propertyCache[CHARGE_BARS] = QVariant();
    }

    propertyCache[TIME_UNTIL_FULL]
        = (quint64)st[bme_stat_charging_time_left_min] * NANOSECS_PER_MIN;
    propertyCache[TIME_UNTIL_LOW]
        = (quint64)st[bme_stat_bat_time_left] * NANOSECS_PER_MIN;

    bme_close(sd);

    return true;

}

/// When provider source has been modified, we emit ValueChanged signal
/// Otherwise, we handle gracefully the deletion of the source and try to
/// recover
void BatteryPlugin::onBMEEvent()
{
    inotify_event ev;
    int rc;
    rc = bme_xchg_inotify_read(xchg, &ev);
    if (rc < 0) {
        qDebug() << "can't read bmeipc xchg inotify event";
        return;
    }

    // XXX: should we read the .bmeevt file and only act on relevant events?

    if ((ev.mask & IN_DELETE_SELF) || (ev.mask & IN_MOVE_SELF)) {
        cleanProviderSource();
        initProviderSource();
    } else if (!(ev.mask & IN_IGNORED)) {
        readBatteryValues();
        foreach(const QString& key, subscribedProperties) {
            Q_EMIT valueChanged(key, propertyCache[key]);
        }
    }
}

/// Called only on first subscription of each property
void BatteryPlugin::emitSubscribeFinished(QSet<QString> keys)
{
    foreach(const QString& key, keys) {
        Q_EMIT subscribeFinished(key, propertyCache[key]);
    }
}

} // end namespace
