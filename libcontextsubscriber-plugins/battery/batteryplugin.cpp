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

#include "batteryplugin.h"
#include "logging.h"
#include "sconnect.h"
#include <fcntl.h>
#include <sys/inotify.h>
#include <sys/ioctl.h>
#include <QSocketNotifier>
#include <QFileSystemWatcher>
#include <QVarLengthArray>
#include <QFile>
#include <QStringList>
#include <QSet>
extern "C" {
#include <bme/bmeipc.h>
}

Q_DECLARE_METATYPE(QSet<QString>);

#define ON_BATTERY       "Battery.OnBattery"
#define LOW_BATTERY      "Battery.LowBattery"
#define CHARGE_PERCENT   "Battery.ChargePercentage"
#define CHARGE_BARS      "Battery.ChargeBars"
#define TIME_UNTIL_LOW   "Battery.TimeUntilLow"
#define TIME_UNTIL_FULL  "Battery.TimeUntilFull"
#define IS_CHARGING      "Battery.IsCharging"
#define BMEIPC_EVENT	 "/tmp/.bmeevt"
#define NANOSECS_PER_MIN (60 * 1000 * 1000LL)

IProviderPlugin* pluginFactory(const QString& /*constructionString*/)
{
    return new ContextSubscriberBattery::BatteryPlugin();
}

namespace ContextSubscriberBattery {

BatteryPlugin::BatteryPlugin():
    bmeevt_watch(-1), sn(NULL)
{
    inotifyFd = bmeipc_eopen(-1);
    if (inotifyFd < 0) {
        QMetaObject::invokeMethod(this, "failed", Qt::QueuedConnection,
                                  Q_ARG(QString, "bmeipc_eopen failed"));
        return;
    }
    fcntl(inotifyFd, F_SETFD, FD_CLOEXEC);
    sn = new QSocketNotifier(inotifyFd, QSocketNotifier::Read, this);
    sn->setEnabled(false);
    connect(sn, SIGNAL(activated(int)), this, SLOT(onBMEEvent()));
    QMetaObject::invokeMethod(this, "ready", Qt::QueuedConnection);
}

/// The provider source of the battery properties is initialised only on the
/// first subscription. Initialisation means adding watcher to BMEIPC_EVENT
void BatteryPlugin::subscribe(QSet<QString> keys)
{
    if (subscribedProperties.isEmpty()) {
        qRegisterMetaType<QSet<QString> >("QSet<QString>");
        initProviderSource();
        QMetaObject::invokeMethod(this, "readBatteryValues", Qt::QueuedConnection);
    }
    QMetaObject::invokeMethod(this, "emitSubscribeFinished", Qt::QueuedConnection,
                              Q_ARG(QSet<QString>, keys));
    subscribedProperties.unite(keys);
}

/// Implementation of the IPropertyProvider::unsubscribe.
/// Properties to unsubscribe are removed from the cache.
/// The provider source is closed and cleaned up when no properties remain in the cache.
void BatteryPlugin::unsubscribe(QSet<QString> keys)
{
    subscribedProperties.subtract(keys);
    if (subscribedProperties.isEmpty())
        cleanProviderSource();
}

/// Start to watch the provider source BMEIPC_EVENT on first subscription or when the source has been deleted or moved
bool BatteryPlugin::initProviderSource()
{
    bmeevt_watch = inotify_add_watch(inotifyFd, BMEIPC_EVENT, (IN_CLOSE_WRITE | IN_DELETE_SELF | IN_MOVE_SELF));
    if (bmeevt_watch < 0) {
        QMetaObject::invokeMethod(this, "failed", Qt::QueuedConnection,
                                  Q_ARG(QString, "Battery plugin failed to add watcher on BMEIPC_EVENT"));
        return false;
    }
    sn->setEnabled(true);
    return true;
}

/// Called when the provicer source BMEIPC_EVENT is deleted, moved or when all
/// properties have been unsubscribed watcher is removed
void BatteryPlugin::cleanProviderSource()
{
    inotify_rm_watch(inotifyFd, bmeevt_watch);
    bmeevt_watch = -1;
    sn->setEnabled(false);
}

/// Called when provider source has been modified and new values are available
bool BatteryPlugin::readBatteryValues()
{
    bmestat_t st;
    int sd = -1;

    if ((sd = bmeipc_open()) < 0) {
        contextWarning() << "Cannot open socket connected to BME server";
        return false;
    }

    if (bmeipc_stat(sd, &st) < 0) {
        contextWarning() << "Cannot get BME statistics";
        return false;
    }

    propertyCache[IS_CHARGING] = (st[CHARGING_STATE] == CHARGING_STATE_STARTED &&
                                  st[BATTERY_STATE] != BATTERY_STATE_FULL);

    propertyCache[ON_BATTERY] =  (st[CHARGER_STATE] != CHARGER_STATE_CONNECTED);
    propertyCache[LOW_BATTERY] = (st[BATTERY_STATE] == BATTERY_STATE_LOW);

    propertyCache[CHARGE_PERCENT] = st[BATTERY_LEVEL_PCT];

    if (st[BATTERY_LEVEL_MAX] != 0) {
        QList<QVariant> list;
        list << QVariant(st[BATTERY_LEVEL_NOW]) << QVariant(st[BATTERY_LEVEL_MAX]);
        propertyCache[CHARGE_BARS] = list;
    }
    else
        propertyCache[CHARGE_BARS] = QVariant();

    propertyCache[TIME_UNTIL_FULL] = (quint64)st[CHARGING_TIME] * NANOSECS_PER_MIN;
    propertyCache[TIME_UNTIL_LOW] = (quint64)st[BATTERY_TIME_LEFT] * NANOSECS_PER_MIN;

    bmeipc_close(sd);

    return true;

}

/// When provider source has been modified, we emit ValueChanged signal
/// Otherwise, we handle gracefully the deletion of the source and try to
/// recover
void BatteryPlugin::onBMEEvent()
{
    inotify_event ev;
    read(inotifyFd, &ev, sizeof(ev));

    // XXX: should we read the .bmeevt file and only act on relevant events?

    if ((ev.mask & IN_DELETE_SELF) || (ev.mask & IN_MOVE_SELF)) {
        cleanProviderSource();
        initProviderSource();
    } else if (!(ev.mask & IN_IGNORED)) {
        readBatteryValues();
        foreach(const QString& key, subscribedProperties) {
            contextDebug() << "emit ValueChanged" << key << "," << propertyCache[key];
            emit valueChanged(key, propertyCache[key]);
        }
    }
}

/// Called only on first subscription of each property
void BatteryPlugin::emitSubscribeFinished(QSet<QString> keys)
{
    foreach(const QString& key, keys) {
        emit subscribeFinished(key, propertyCache[key]);
    }
}

} // end namespace

