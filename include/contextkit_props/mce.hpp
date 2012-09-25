#ifndef _CONTEXTKIT_PROPS_CELLULAR_HPP_
#define _CONTEXTKIT_PROPS_CELLULAR_HPP_

#include <contextkit_props/common.hpp>

CKIT_PROP(mce_is_screen_blanked, "Screen.Blanked", bool);
CKIT_PROP(mce_is_psm, "System.PowerSaveMode", bool);
CKIT_PROP(mce_is_offline, "System.OfflineMode", bool);
CKIT_PROP(mce_is_inet_enabled, "System.InternetEnabled", bool);
CKIT_PROP(mce_is_wlan_enabled, "System.WlanEnabled", bool);

#endif
