/*
 * Copyright (C) 2008 Nokia Corporation.
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

#include <QDBusConnection>
#include <ContextProvider>

#include "halprovider.h"
#include "halmanagerinterface.h"
#include "logging.h"
#include "loggingfeatures.h"
#include "sconnect.h"

namespace ContextD {

#define BATTERY_LOW_THRESHOLD       10
#define BATTERY_CHARGE_PERCENTAGE   "battery.charge_level.percentage"
#define BATTERY_CHARGE_CURRENT      "battery.charge_level.current" 
#define BATTERY_CHARGE_RATE         "battery.charge_level.rate"
#define BATTERY_IS_DISCHARGING      "battery.rechargeable.is_discharging"
#define BATTERY_IS_CHARGING         "battery.rechargeable.is_charging"
#define BATTERY_LAST_FULL           "battery.charge_level.last_full"

/*!
    \class HalProvider

    \brief Provides the hal information about power.

    This provider provides information about the power & battery. The info
    is obtained from the Hal subsystem. The following properties are provided:

    \code
    Battery.OnBattery - is machine is running on battery power
    Battery.ChargePercentage - battery charge percentage
    Battery.LowBattery - is battery low?
    Battery.TimeUntilLow - time (in seconds) until battery is low
    Battery.TimeUntilFull - time (in seconds) until battery is full
    \endcode

    Communication with Hal happens over DBus.
*/

HalProvider::HalProvider() 
    : onBattery("Battery.OnBattery"), lowBattery("Battery.LowBattery"), 
      chargePercentage("Battery.ChargePercentage"), timeUntilLow("Battery.TimeUntilLow"), 
      timeUntilFull("Battery.TimeUntilFull")
{
    contextDebug() << F_HAL << "Initializing hal provider";
  
    group << onBattery;
    group << chargePercentage;
    group << lowBattery;
    group << timeUntilLow;
    group << timeUntilFull;

    sconnect(&group, SIGNAL(firstSubscriberAppeared()),
            this, SLOT(onFirstSubscriberAppeared()));

    sconnect(&group, SIGNAL(lastSubscriberDisappeared()),
            this, SLOT(onLastSubscriberDisappeared()));
}

/// Called when the first subscriber for any of the keys appears. We initialize
/// the connection to Hal and pick the first battery to inspect. We keep this connection
/// and listen for updates as long as there is an active subscriber.
void HalProvider::onFirstSubscriberAppeared()
{
    contextDebug() << F_HAL << "First subscriber appeared, connecting to HAL";
    HalManagerInterface manager(QDBusConnection::systemBus(), "org.freedesktop.Hal", this);
    QStringList batteries = manager.findDeviceByCapability("battery");

    if (batteries.length() == 1) {
        contextDebug() << F_HAL << "Analyzing info from battery:" << batteries.at(0);
        batteryDevice = new HalDeviceInterface(QDBusConnection::systemBus(), "org.freedesktop.Hal", 
                                               batteries.at(0), this);

        sconnect(batteryDevice, SIGNAL(PropertyModified()),
                 this, SLOT(onDevicePropertyModified()));

        updateProperties();

    } else if (batteries.length() == 0) {
        contextWarning() << F_HAL << "No valid battery device found";
    } else {
        contextWarning() << F_HAL << "Having more than one battery is currently unsupported!";
    }
}

/// Called when the last subscriber stops watching any of our properties. 
/// We terminate the connection to Hal, stop watching for battery changes 
/// and free the resources.
void HalProvider::onLastSubscriberDisappeared()
{
    contextDebug() << F_HAL << "Last subscriber gone, destroying HAL connections";
    delete batteryDevice;
    batteryDevice = NULL;
}

/// Called when a battery property changed. We recompute all our properties.
void HalProvider::onDevicePropertyModified()
{
    contextDebug() << F_HAL << "Battery property changed.";
    updateProperties();
}

/// This fetches values of the relevant battery properties and recomputes the context 
/// info based on that. It recomputes all data (all context properties). 
void HalProvider::updateProperties()
{
    contextDebug() << F_HAL << "Updating properties";

    QVariant chargePercentageV = batteryDevice->readValue(BATTERY_CHARGE_PERCENTAGE);
    QVariant chargeCurrentV = batteryDevice->readValue(BATTERY_CHARGE_CURRENT);
    QVariant isDischargingV = batteryDevice->readValue(BATTERY_IS_DISCHARGING);
    QVariant isChargingV = batteryDevice->readValue(BATTERY_IS_CHARGING);
    QVariant lastFullV = batteryDevice->readValue(BATTERY_LAST_FULL);
    QVariant rateV = batteryDevice->readValue(BATTERY_CHARGE_RATE);
   
    // Calculate and set ChargePercentage
    if (chargePercentageV != QVariant()) 
        chargePercentage.setValue(chargePercentageV.toInt());
    else
        chargePercentage.unsetValue();

    // Calculate and set OnBattery
    if (isDischargingV != QVariant()) 
        onBattery.setValue(isDischargingV.toBool());
    else
        onBattery.unsetValue();

    // Calculate the LowBattery
    if (isDischargingV == QVariant()) 
        lowBattery.unsetValue();
    else if (isDischargingV.toBool() == false) 
        lowBattery.setValue(false);
    else if (chargePercentageV == QVariant())
        lowBattery.unsetValue();
    else if (chargePercentageV.toInt() < BATTERY_LOW_THRESHOLD)
        lowBattery.setValue(true);
    else {
        lowBattery.setValue(false);
    }

    // Calculate the time until low. 
    if (chargeCurrentV != QVariant() && 
        isDischargingV != QVariant() && isDischargingV.toBool() == true &&
        lastFullV != QVariant() && lastFullV.toInt() != 0 &&
        rateV != QVariant() && rateV.toInt() != 0) {

        double timeUntilLowV = (chargeCurrentV.toDouble() - BATTERY_LOW_THRESHOLD / 100.0 * lastFullV.toDouble()) / 
                               rateV.toDouble();
        if (timeUntilLowV < 0)
            timeUntilLowV = 0;

        timeUntilLowV *= 3600; // Seconds
        timeUntilLow.setValue((int) timeUntilLowV);
    } else
        timeUntilLow.unsetValue();

    // Calculate the time until full.
    if (chargeCurrentV != QVariant() &&
        isChargingV != QVariant() && isChargingV.toBool() == true &&
        lastFullV != QVariant() && lastFullV.toInt() != 0 &&
        rateV != QVariant() && rateV.toInt() != 0) {

        double timeUntilFullV = (lastFullV.toDouble() - chargeCurrentV.toDouble()) / rateV.toDouble();
        timeUntilFullV *= 3600; // Seconds
        timeUntilFull.setValue((int) timeUntilFullV);
    } else
        timeUntilFull.unsetValue();
}

}
