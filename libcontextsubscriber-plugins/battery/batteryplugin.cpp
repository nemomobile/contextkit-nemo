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

#include <fcntl.h>
#include <sys/inotify.h>
#include <sys/ioctl.h>
#include "logging.h"
#include "batteryplugin.h"
#include "sconnect.h"
#include <QSocketNotifier>
#include <QFileSystemWatcher>
#include <QVarLengthArray>

extern "C" {
    #include "bme/bmeipc.h"
}

#define ON_BATTERY       "Battery.OnBattery"
#define LOW_BATTERY      "Battery.LowBattery"
#define CHARGE_PERCENT   "Battery.ChargePercentage"
#define TIME_UNTIL_LOW   "Battery.TimeUntilLow"
#define TIME_UNTIL_FULL  "Battery.TimeUntilFull"
#define IS_CHARGING      "Battery.IsCharging"
#define BMEIPC_EVENT	 "/tmp/.bmeevt"

/// The factory method for constructing the IPropertyProvider instance.
IProviderPlugin* pluginFactory(const QString& /*constructionString*/)
{
    // Note: it's the caller's responsibility to delete the plugin if
    // needed.
    return new ContextSubscriberBattery::BatteryPlugin();
}

namespace ContextSubscriberBattery {


/// Constructor. Try to connect to Battery right away.
BatteryPlugin::BatteryPlugin()
{
    propertiesCache.insert(ON_BATTERY,QVariant());
    propertiesCache.insert(LOW_BATTERY,QVariant());
    propertiesCache.insert(CHARGE_PERCENT,QVariant());
    propertiesCache.insert(TIME_UNTIL_LOW,QVariant());
    propertiesCache.insert(TIME_UNTIL_FULL,QVariant());
    propertiesCache.insert(IS_CHARGING,QVariant());

    inotifyFd = bmeipc_eopen(-1); // Mask is thrown away
    fcntl(inotifyFd, F_SETFD, FD_CLOEXEC);
    int wd = inotify_add_watch(inotifyFd, BMEIPC_EVENT, (IN_CLOSE_WRITE | IN_DELETE_SELF));
    if (0 > wd) {
        contextDebug() << "inotify_add_watch failed";
        emit failed("Requested properties not supported by BME");
    }
    QSocketNotifier *sn = new QSocketNotifier(inotifyFd, QSocketNotifier::Read, this);
    connect(sn, SIGNAL(activated(int)), this, SLOT(onBMEEvent()));

    QMetaObject::invokeMethod(this, "emitReady", Qt::QueuedConnection);
}

/// Destructor.
BatteryPlugin::~BatteryPlugin()
{
}

/// Implementation of the IPropertyProvider::subscribe. We don't need
/// any extra work for subscribing to keys, thus subscribe is finished
/// right away.
void BatteryPlugin::subscribe(QSet<QString> keys)
{
    QMetaObject::invokeMethod(this, "readBatteryStats", Qt::QueuedConnection);
    //readBatteryStats();

    // Check for invalid keys
    foreach(const QString& key, keys) {
        if (propertiesCache.contains(key)) {
            contextDebug() << "Key" << key << "found in cache";
            emit subscribeFinished(key, propertiesCache[key]);
        }
        else {
            // This shouldn't occur if the plugin functions correctly
            contextCritical() << "Key not in cache" << key;
            emit failed("Requested properties not supported by BME");
        }
    }
}

/// Implementation of the IPropertyProvider::unsubscribe. We're not
/// keeping track on subscriptions, so we don't need to do anything.
void BatteryPlugin::unsubscribe(QSet<QString> keys)
{
}

bool BatteryPlugin::readBatteryStats()
{
    bmestat_t st;
    bool newVal;
    //int32_t sd = -1;
    int intVal = 0, sd = -1;

    if (0 > (sd = bmeipc_open())){
        contextDebug() << "Cannot open BME file descriptor";
        emit failed("Cannot open BME file descriptor");
        return false;
    }

    if (0 > bmeipc_stat(sd, &st)) {
        contextDebug() << "Cannot get BME statistics";
        emit failed("Cannot get BME statistics");
        return false;
    }

    if (st[CHARGING_STATE] == CHARGING_STATE_STARTED || st[CHARGING_STATE] == CHARGING_STATE_SPECIAL)
        newVal = true;

    if (st[CHARGING_STATE] == CHARGING_STATE_STOPPED || st[CHARGING_STATE] == CHARGING_STATE_ERROR)
        newVal = false;

    if (propertiesCache[IS_CHARGING] != newVal) {
        propertiesCache[IS_CHARGING] = newVal;
        emit valueChanged(IS_CHARGING, propertiesCache[IS_CHARGING]);
        //QMetaObject::invokeMethod(this, "emitValueChanged", Qt::QueuedConnection, Q_ARG(QString, IS_CHARGING));
    }

    if (propertiesCache[ON_BATTERY] != newVal) {
        propertiesCache[ON_BATTERY] = !newVal;
        emit valueChanged(ON_BATTERY, propertiesCache[ON_BATTERY]);
        //QMetaObject::invokeMethod(this, "emitValueChanged", Qt::QueuedConnection, Q_ARG(QString, ON_BATTERY));
    }

    intVal = st[CHARGING_TIME] * 60;
    if (propertiesCache[TIME_UNTIL_FULL] != intVal) {
        propertiesCache[TIME_UNTIL_FULL] = intVal;
        emit valueChanged(TIME_UNTIL_FULL, propertiesCache[TIME_UNTIL_FULL]);
        //QMetaObject::invokeMethod(this, "emitValueChanged", Qt::QueuedConnection, Q_ARG(QString, TIME_UNTIL_FULL));
    }

    if (st[BATTERY_STATE] == BATTERY_STATE_LOW)
        newVal = true;
    else newVal = false;

    if (propertiesCache[LOW_BATTERY] != newVal) {
        propertiesCache[LOW_BATTERY] = newVal;
        emit valueChanged(LOW_BATTERY, propertiesCache[LOW_BATTERY]);
        //QMetaObject::invokeMethod(this, "emitValueChanged", Qt::QueuedConnection, Q_ARG(QString, LOW_BATTERY));
    }

    intVal = st[BATTERY_CAPA_NOW] * 100 / st[BATTERY_CAPA_MAX];

    if (propertiesCache[CHARGE_PERCENT] != intVal) {
        propertiesCache[CHARGE_PERCENT] = intVal;
        emit valueChanged(CHARGE_PERCENT, propertiesCache[CHARGE_PERCENT]);
        //QMetaObject::invokeMethod(this, "emitValueChanged", Qt::QueuedConnection, Q_ARG(QString, CHARGE_PERCENT));
    }

    intVal = st[BATTERY_TIME_LEFT] * 60;

    if (propertiesCache[TIME_UNTIL_LOW] != intVal) {
        propertiesCache[TIME_UNTIL_LOW] = intVal;
        emit valueChanged(TIME_UNTIL_LOW, propertiesCache[TIME_UNTIL_LOW]);
        //QMetaObject::invokeMethod(this, "emitValueChanged", Qt::QueuedConnection, Q_ARG(QString, TIME_UNTIL_LOW));
    }

    bmeipc_close(sd);

    return true;
}


/// A BMEEvent is received
void BatteryPlugin::onBMEEvent()
{
    // Empty buffer. Should not blog, since we got signal from QSocketNotifier
    int buffSize = 0;
    ioctl(inotifyFd, FIONREAD, (char *) &buffSize);
    QVarLengthArray<char, 4096> buffer(buffSize);
    buffSize = read(inotifyFd, buffer.data(), buffSize);

    readBatteryStats();
}

/// For emitting the ready() signal in a delayed way.
void BatteryPlugin::emitReady()
{
    emit ready();
}

///
void BatteryPlugin::emitValueChanged(QString key)
{
    if (key == IS_CHARGING)
        emit valueChanged(IS_CHARGING, propertiesCache[IS_CHARGING]);

    if (key == ON_BATTERY)
        emit valueChanged(ON_BATTERY, propertiesCache[ON_BATTERY]);

    if (key == TIME_UNTIL_LOW)
        emit valueChanged(TIME_UNTIL_LOW, propertiesCache[TIME_UNTIL_LOW]);

    if (key == TIME_UNTIL_FULL)
        emit valueChanged(TIME_UNTIL_FULL, propertiesCache[TIME_UNTIL_FULL]);

    if (key == LOW_BATTERY)
        emit valueChanged(LOW_BATTERY, propertiesCache[LOW_BATTERY]);

    if (key == CHARGE_PERCENT)
        emit valueChanged(CHARGE_PERCENT, propertiesCache[CHARGE_PERCENT]);

}


} // end namespace

