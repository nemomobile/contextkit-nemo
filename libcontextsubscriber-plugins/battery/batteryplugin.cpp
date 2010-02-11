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
#include "batteryplugin.h"
#include "sconnect.h"
#include "logging.h"
extern "C" {
    #include "bme/bmeipc.h"
}

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
    timer = new QTimer(this);
    sconnect(timer, SIGNAL(timeout()), this, SLOT(timedOut()));
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
    readBatteryStats();

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

    timer->start(1000);

}

/// Implementation of the IPropertyProvider::unsubscribe. We're not
/// keeping track on subscriptions, so we don't need to do anything.
void BatteryPlugin::unsubscribe(QSet<QString> keys)
{
    timer->stop();
}

bool BatteryPlugin::readBatteryStats()
{
    bool newVal, newIval;
    bmestat_t st;
    int32_t sd = -1;
    int intVal = 0;
    double floatVal;
    
    contextDebug() << "Read stats";
    //emit valueChanged(IS_CHARGING,propertiesCache[IS_CHARGING]);

    if (0 > (sd = bmeipc_open())){
        contextDebug() << "Can not open bme file descriptor";
        emit failed("Can not open bme file descriptor");
        return false;
    }

    if (0 > bmeipc_stat(sd, &st)) {
        contextDebug() << "Can not get bme statistics";
        emit failed("Can not get bme statistics");
        return false;
    }

    if (st[CHARGING_STATE] == CHARGING_STATE_STARTED || st[CHARGING_STATE] == CHARGING_STATE_SPECIAL) {
        newVal = true;
        newIval = false;
    }

    if (st[CHARGING_STATE] == CHARGING_STATE_STOPPED || st[CHARGING_STATE] == CHARGING_STATE_ERROR) {
        newVal = false;
        newIval = true;
    }

    if (propertiesCache[IS_CHARGING] != newVal) {
        propertiesCache[IS_CHARGING] = newVal;
        contextDebug() << "Value queued for emission" << propertiesCache[IS_CHARGING];
        QMetaObject::invokeMethod(this, "emitValueChanged", Qt::QueuedConnection, Q_ARG(QString, IS_CHARGING));
    }

    if (propertiesCache[ON_BATTERY] != newIval) {
        propertiesCache[ON_BATTERY] = newIval;
        contextDebug() << "Value queued for emission" << propertiesCache[ON_BATTERY];
        QMetaObject::invokeMethod(this, "emitValueChanged", Qt::QueuedConnection, Q_ARG(QString, ON_BATTERY));
    }

    intVal = st[CHARGING_TIME] / 60;
    if (propertiesCache[TIME_UNTIL_FULL] != intVal) {
        propertiesCache[TIME_UNTIL_FULL] = intVal;
        QMetaObject::invokeMethod(this, "emitValueChanged", Qt::QueuedConnection, Q_ARG(QString, TIME_UNTIL_FULL));
    }

    if (st[BATTERY_STATE] == BATTERY_STATE_LOW)
        newVal = true;

    if (propertiesCache[LOW_BATTERY] != newVal) {
        propertiesCache[LOW_BATTERY] = newVal;
        QMetaObject::invokeMethod(this, "emitValueChanged", Qt::QueuedConnection, Q_ARG(QString, LOW_BATTERY));
    }

    //floatVal = ((double)(st[BATTERY_LEVEL_NOW]/st[BATTERY_LEVEL_MAX])) * 100;
    floatVal = ((double)(st[BATTERY_CAPA_NOW] * 100 / st[BATTERY_CAPA_MAX]));

    if (propertiesCache[CHARGE_PERCENT] != (int)floatVal) {
        propertiesCache[CHARGE_PERCENT] = (int)floatVal;
        QMetaObject::invokeMethod(this, "emitValueChanged", Qt::QueuedConnection, Q_ARG(QString, CHARGE_PERCENT));
    }

    intVal = st[BATTERY_TIME_LEFT] / 3600;
    if (propertiesCache[TIME_UNTIL_LOW] != intVal) {
        propertiesCache[TIME_UNTIL_LOW] = intVal;
        QMetaObject::invokeMethod(this, "emitValueChanged", Qt::QueuedConnection, Q_ARG(QString, TIME_UNTIL_LOW));
    }

    bmeipc_close(sd);

    return true;
}


/// A BMEEvent is received
void BatteryPlugin::timedOut()
{
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

