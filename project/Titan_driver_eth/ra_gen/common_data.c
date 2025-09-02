/* generated common source file - do not edit */
#include "common_data.h"

const ether_phy_lsi_cfg_t g_rmac_phy_lsi1 =
{
    .address           = 1,
    .type              = ETHER_PHY_LSI_TYPE_CUSTOM,
};
rmac_phy_instance_ctrl_t g_rmac_phy1_ctrl;
#define RA_NOT_DEFINED (1)
const rmac_phy_extended_cfg_t g_rmac_phy1_extended_cfg =
{
    .p_target_init                     = rmac_phy_target_rtl8211_initialize,
    .p_target_link_partner_ability_get = rmac_phy_target_rtl8211_is_support_link_partner_ability,
    .frame_format                      = RMAC_PHY_FRAME_FORMAT_MDIO,
    .mdc_clock_rate                    = 2500000,
    .mdio_hold_time                    = 0,
    .mdio_capture_time                 = 0,
    .p_phy_lsi_cfg_list = {
#if (RA_NOT_DEFINED != RA_NOT_DEFINED)
    	&RA_NOT_DEFINED,
#else
    	NULL,
#endif
#if (RA_NOT_DEFINED != g_rmac_phy_lsi1)
    	&g_rmac_phy_lsi1,
#else
    	NULL,
#endif
    },
    .default_phy_lsi_cfg_index = 1,
};
#undef RA_NOT_DEFINED

const ether_phy_cfg_t g_rmac_phy1_cfg =
{

    .channel                   = 1,
    .phy_reset_wait_time       = 0x00020000,
    .mii_bit_access_wait_time  = 0,
    .flow_control              = ETHER_PHY_FLOW_CONTROL_DISABLE,
    .mii_type                  = ETHER_PHY_MII_TYPE_RGMII,
    .p_context                 = NULL,
    .p_extend                  = &g_rmac_phy1_extended_cfg,

};
/* Instance structure to use this module. */
const ether_phy_instance_t g_rmac_phy1 =
{
    .p_ctrl        = &g_rmac_phy1_ctrl,
    .p_cfg         = &g_rmac_phy1_cfg,
    .p_api         = &g_ether_phy_on_rmac_phy
};
const ether_phy_lsi_cfg_t g_rmac_phy_lsi0 =
{
    .address           = 2,
    .type              = ETHER_PHY_LSI_TYPE_CUSTOM,
};
rmac_phy_instance_ctrl_t g_rmac_phy0_ctrl;
#define RA_NOT_DEFINED (1)
const rmac_phy_extended_cfg_t g_rmac_phy0_extended_cfg =
{
    .p_target_init                     = rmac_phy_target_rtl8211_initialize,
    .p_target_link_partner_ability_get = rmac_phy_target_rtl8211_is_support_link_partner_ability,
    .frame_format                      = RMAC_PHY_FRAME_FORMAT_MDIO,
    .mdc_clock_rate                    = 2500000,
    .mdio_hold_time                    = 0,
    .mdio_capture_time                 = 0,
    .p_phy_lsi_cfg_list = {
#if (RA_NOT_DEFINED != g_rmac_phy_lsi0)
    	&g_rmac_phy_lsi0,
#else
    	NULL,
#endif
#if (RA_NOT_DEFINED != RA_NOT_DEFINED)
    	&RA_NOT_DEFINED,
#else
    	NULL,
#endif
    },
    .default_phy_lsi_cfg_index = 0,
};
#undef RA_NOT_DEFINED

const ether_phy_cfg_t g_rmac_phy0_cfg =
{

    .channel                   = 0,
    .phy_reset_wait_time       = 0x00020000,
    .mii_bit_access_wait_time  = 0,
    .flow_control              = ETHER_PHY_FLOW_CONTROL_DISABLE,
    .mii_type                  = ETHER_PHY_MII_TYPE_RGMII,
    .p_context                 = NULL,
    .p_extend                  = &g_rmac_phy0_extended_cfg,

};
/* Instance structure to use this module. */
const ether_phy_instance_t g_rmac_phy0 =
{
    .p_ctrl        = &g_rmac_phy0_ctrl,
    .p_cfg         = &g_rmac_phy0_cfg,
    .p_api         = &g_ether_phy_on_rmac_phy
};
layer3_switch_instance_ctrl_t g_layer3_switch0_ctrl;

uint8_t g_layer3_switch0_mac_address_port0[6] = { 0x00,0x11,0x22,0x33,0x44,0x55 };
uint8_t g_layer3_switch0_mac_address_port1[6] = { 0x00,0x11,0x22,0x33,0x44,0x55 };
layer3_switch_l3_filter_t g_layer3_switch0_l3_filter_list[10];

#define RA_NOT_DEFINED (1)
const layer3_switch_extended_cfg_t g_layer3_switch0_extended_cfg =
{
    .p_ether_phy_instances          = {
#if (RA_NOT_DEFINED != g_rmac_phy0)
    &g_rmac_phy0,
#else
    	NULL,
#endif
#if (RA_NOT_DEFINED != g_rmac_phy1)
    &g_rmac_phy1,
#else
    	NULL,
#endif
	},
    .fowarding_target_port_masks =
    {
        (LAYER3_SWITCH_PORT_BITMASK_PORT1 | LAYER3_SWITCH_PORT_BITMASK_PORT2 |  0U),
        (LAYER3_SWITCH_PORT_BITMASK_PORT0 | LAYER3_SWITCH_PORT_BITMASK_PORT2 |  0U),
    },
    .p_mac_addresses =
    {
    g_layer3_switch0_mac_address_port0,
    g_layer3_switch0_mac_address_port1
    },
    .l3_filter_list = g_layer3_switch0_l3_filter_list,
    .l3_filter_list_length = 10,
};
#undef RA_NOT_DEFINED

const ether_switch_cfg_t g_layer3_switch0_cfg =
{
    .channel        = 0,

#if defined(VECTOR_NUMBER_ETHER_GWDI0)
    .irq            = VECTOR_NUMBER_ETHER_GWDI0,
#else
    .irq            = FSP_INVALID_VECTOR,
#endif

    .ipl            = (12),

    .p_callback     = NULL,
    .p_context      = NULL,
    .p_extend       = &g_layer3_switch0_extended_cfg,
};

/* Instance structure to use this module. */
const ether_switch_instance_t g_layer3_switch0 =
{
    .p_ctrl        = &g_layer3_switch0_ctrl,
    .p_cfg         = &g_layer3_switch0_cfg,
    .p_api         = &g_ether_switch_on_layer3_switch
};
ioport_instance_ctrl_t g_ioport_ctrl;
const ioport_instance_t g_ioport =
        {
            .p_api = &g_ioport_on_ioport,
            .p_ctrl = &g_ioport_ctrl,
            .p_cfg = &g_bsp_pin_cfg,
        };
void g_common_init(void) {
}
