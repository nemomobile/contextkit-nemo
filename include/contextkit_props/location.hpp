#ifndef _CONTEXTKIT_PROPS_LOCATION_HPP_
#define _CONTEXTKIT_PROPS_LOCATION_HPP_

#include <contextkit_props/common.hpp>

CKIT_PROP(location_sat_pos_state, "Location.SatPositioningState", string);
CKIT_PROP(location_coord, "Location.Coordinates", list_string);
CKIT_PROP(location_heading, "Location.Heading", double);

#endif
