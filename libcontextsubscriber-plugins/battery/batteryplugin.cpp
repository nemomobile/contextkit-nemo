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
    bool newVal;
    bmestat_t st;
    int32_t sd = -1;

    contextDebug() << "Read stats";
    emit valueChanged(IS_CHARGING,propertiesCache[IS_CHARGING]);

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

    if (st[CHARGING_STATE] == CHARGING_STATE_STARTED || st[CHARGING_STATE] == CHARGING_STATE_SPECIAL)
        newVal = true;
    else newVal = false;

    if (propertiesCache[IS_CHARGING] != newVal) {
        propertiesCache[IS_CHARGING] = newVal;
        emit valueChanged(IS_CHARGING,propertiesCache[IS_CHARGING]);
    }

/*
    if (st[BATTERY_STATE] == BATTERY_STATE_LOW)
        propertiesCache["batteryLow"] = QVariant(true);

    propertiesCache["timeUntilLow"] = QVariant(st[BATTERY_TIME_LEFT]);
    propertiesCache["timeUntilFull"] = QVariant(st[CHARGING_TIME]);
    propertiesCache["chargePercentage"] = QVariant(st[BATTERY_LEVEL_NOW]/st[BATTERY_LEVEL_MAX]);
*/
    return true;
}


/// A BMEEvent is received
void BatteryPlugin::timedOut()
{
    if (readBatteryStats()){
    }
}

/// Check the current status of the Session.State property and emit
/// the valueChanged signal.
void BatteryPlugin::emitValueChanged()
{
}

} // end namespace

