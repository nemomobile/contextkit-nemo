#ifndef _CONTEXTKIT_PROPS_CELLULAR_HPP_
#define _CONTEXTKIT_PROPS_CELLULAR_HPP_

#include <contextkit_props/common.hpp>

CKIT_PROP(cell_sig_strength, "Cellular.SignalStrength", int);
CKIT_PROP(cell_data_tech, "Cellular.DataTechnology", string);
CKIT_PROP(cell_reg_status, "Cellular.RegistrationStatus", int);
CKIT_PROP(cell_technology, "Cellular.Technology", string);
CKIT_PROP(cell_sig_bars, "Cellular.SignalBars", int);
CKIT_PROP(cell_cell_name, "Cellular.CellName", string);
CKIT_PROP(cell_net_name, "Cellular.NetworkName", string);
//CKIT_PROP(cell_pkt_data, "Cellular.PacketData", string);
//CKIT_PROP(cell_gsm_band "Cellular.GSMBand", string);
//CKIT_PROP(cell_ext_net_name, "Cellular.ExtendedNetworkName", string);
//CKIT_PROP(cell_name_disp, "Cellular.CellNameDisplayEnabled", string);

#endif
