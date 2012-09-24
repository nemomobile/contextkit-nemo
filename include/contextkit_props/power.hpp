#ifndef _CONTEXTKIT_PROPS_POWER_HPP_
#define _CONTEXTKIT_PROPS_POWER_HPP_

#include <contextkit_props/common.hpp>

CKIT_PROP(power_on_battery, "Battery.OnBattery", bool);
CKIT_PROP(power_low_battery, "Battery.LowBattery", bool);
CKIT_PROP(power_charge_percent, "Battery.ChargePercentage", int);
CKIT_PROP(power_charge_bars, "Battery.ChargeBars",  int);
CKIT_PROP(power_time_until_low, "Battery.TimeUntilLow", ulonglong);
CKIT_PROP(power_time_until_full, "Battery.TimeUntilFull", ulonglong);
CKIT_PROP(power_is_charging, "Battery.IsCharging",  bool);

#endif
