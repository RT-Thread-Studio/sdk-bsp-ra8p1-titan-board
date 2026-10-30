/* generated common header file - do not edit */
#ifndef COMMON_DATA_H_
#define COMMON_DATA_H_
#include <stdint.h>
#include "bsp_api.h"
#include "rm_ethosu_api.h"
            #include "rm_ethosu.h"
#include "r_mipi_csi.h"
#include "r_mipi_csi_api.h"
#include "r_vin.h"
#include "r_capture_api.h"
#include "r_glcdc.h"
            #include "r_display_api.h"
#include "r_icu.h"
#include "r_external_irq_api.h"
#include "dave_driver.h"
#include "r_rmac_phy.h"
#include "r_ether_phy_api.h"
#include "r_layer3_switch.h"
#include "r_ioport.h"
#include "bsp_pin_cfg.h"
FSP_HEADER
#include "ethosu_driver.h"
            extern struct ethosu_driver g_ethosu0;
extern rm_ethosu_instance_ctrl_t g_rm_ethosu0_ctrl;
            extern const rm_ethosu_cfg_t g_rm_ethosu0_cfg;
            extern const rm_ethosu_instance_t g_rm_ethosu0;
            #ifndef NULL
            void NULL(rm_ethosu_callback_args_t * p_arg);
            #endif
/* MIPI PHY on MIPI PHY Instance. */

        extern const mipi_phy_instance_t g_cam_mipi_phy;

        /* Access the MIPI PHY instance using these structures when calling API functions directly (::p_api is not used). */
        extern mipi_phy_ctrl_t g_cam_mipi_phy_ctrl;
        extern const mipi_phy_cfg_t g_cam_mipi_phy_cfg;
/* MIPI CSI on MIPI CSI Instance. */
            extern const mipi_csi_instance_t g_cam_mipi_csi;

            /* Access the MIPI CSI instance using these structures when calling API functions directly (::p_api is not used). */
            extern mipi_csi_instance_ctrl_t g_cam_mipi_csi_ctrl;
            extern const mipi_csi_cfg_t g_cam_mipi_csi_cfg;

            #ifndef cam_mipi_csi_callback
              void cam_mipi_csi_callback(mipi_csi_callback_args_t * p_args);
            #endif
/* MIPI VIN on MIPI VIN Instance. */
            extern const capture_instance_t g_cam_vin;

            /* Access the MIPI VIN instance using these structures when calling API functions directly (::p_api is not used). */
            extern vin_instance_ctrl_t g_cam_vin_ctrl;
            extern const capture_cfg_t g_cam_vin_cfg;

            #ifndef cam_vin_callback
              void cam_vin_callback(capture_callback_args_t * p_args);
            #endif


#ifndef VIN_CFG_IMAGE_STRIDE
  #define VIN_CFG_IMAGE_STRIDE (640)
#endif

#ifndef VIN_CFG_BYTES_PER_LINE
  #define VIN_CFG_BYTES_PER_LINE (1280)
#endif


#define VIN_BYTES_PER_FRAME (VIN_CFG_BYTES_PER_LINE * 480)

extern uint8_t vin_image_buffer_1[VIN_BYTES_PER_FRAME];
extern uint8_t vin_image_buffer_2[VIN_BYTES_PER_FRAME];
extern uint8_t vin_image_buffer_3[VIN_BYTES_PER_FRAME];

#define GLCDC_CFG_LAYER_1_ENABLE (false)
            #define GLCDC_CFG_LAYER_2_ENABLE (false)

            #define GLCDC_CFG_CLUT_ENABLE (false)

            #define GLCDC_CFG_CORRECTION_GAMMA_ENABLE_R       (false)
            #define GLCDC_CFG_CORRECTION_GAMMA_ENABLE_G       (false)
            #define GLCDC_CFG_CORRECTION_GAMMA_ENABLE_B       (false)

            /* Display on GLCDC Instance. */
            extern const display_instance_t g_display0;
            extern display_runtime_cfg_t g_display0_runtime_cfg_fg;
            extern display_runtime_cfg_t g_display0_runtime_cfg_bg;

            /** Access the GLCDC instance using these structures when calling API functions directly (::p_api is not used). */
            extern glcdc_instance_ctrl_t g_display0_ctrl;
            extern const display_cfg_t g_display0_cfg;

            #if ((GLCDC_CFG_CORRECTION_GAMMA_ENABLE_R | GLCDC_CFG_CORRECTION_GAMMA_ENABLE_G | GLCDC_CFG_CORRECTION_GAMMA_ENABLE_B) && GLCDC_CFG_COLOR_CORRECTION_ENABLE && !(false))
            extern display_gamma_correction_t g_display0_gamma_cfg;
            #endif

            #if GLCDC_CFG_CLUT_ENABLE
            extern display_clut_cfg_t g_display0_clut_cfg_glcdc;
            #endif

            #ifndef DisplayVsyncCallback
            void DisplayVsyncCallback(display_callback_args_t * p_args);
            #endif

            #define DISPLAY_IN_FORMAT_16BITS_RGB565_0
            #if defined (DISPLAY_IN_FORMAT_32BITS_RGB888_0) || defined (DISPLAY_IN_FORMAT_32BITS_ARGB8888_0)
            #define DISPLAY_BITS_PER_PIXEL_INPUT0 (32)
            #elif defined (DISPLAY_IN_FORMAT_16BITS_RGB565_0) || defined (DISPLAY_IN_FORMAT_16BITS_ARGB1555_0) || defined (DISPLAY_IN_FORMAT_16BITS_ARGB4444_0)
            #define DISPLAY_BITS_PER_PIXEL_INPUT0 (16)
            #elif defined (DISPLAY_IN_FORMAT_CLUT8_0)
            #define DISPLAY_BITS_PER_PIXEL_INPUT0 (8)
            #elif defined (DISPLAY_IN_FORMAT_CLUT4_0)
            #define DISPLAY_BITS_PER_PIXEL_INPUT0 (4)
            #else
            #define DISPLAY_BITS_PER_PIXEL_INPUT0 (1)
            #endif
            #define DISPLAY_HSIZE_INPUT0                 (800)
            #define DISPLAY_VSIZE_INPUT0                 (480)
            #define DISPLAY_BUFFER_STRIDE_BYTES_INPUT0   (((DISPLAY_HSIZE_INPUT0 * DISPLAY_BITS_PER_PIXEL_INPUT0 + 0x1FF) >> 9) << 6)
            #define DISPLAY_BUFFER_STRIDE_PIXELS_INPUT0  ((DISPLAY_BUFFER_STRIDE_BYTES_INPUT0 * 8) / DISPLAY_BITS_PER_PIXEL_INPUT0)
            #if GLCDC_CFG_LAYER_1_ENABLE
            extern uint8_t fb_background[2][DISPLAY_BUFFER_STRIDE_BYTES_INPUT0 * DISPLAY_VSIZE_INPUT0];
            #endif

            #define DISPLAY_IN_FORMAT_16BITS_RGB565_1
            #if defined (DISPLAY_IN_FORMAT_32BITS_RGB888_1) || defined (DISPLAY_IN_FORMAT_32BITS_ARGB8888_1)
            #define DISPLAY_BITS_PER_PIXEL_INPUT1 (32)
            #elif defined (DISPLAY_IN_FORMAT_16BITS_RGB565_1) || defined (DISPLAY_IN_FORMAT_16BITS_ARGB1555_1) || defined (DISPLAY_IN_FORMAT_16BITS_ARGB4444_1)
            #define DISPLAY_BITS_PER_PIXEL_INPUT1 (16)
            #elif defined (DISPLAY_IN_FORMAT_CLUT8_1)
            #define DISPLAY_BITS_PER_PIXEL_INPUT1 (8)
            #elif defined (DISPLAY_IN_FORMAT_CLUT4_1)
            #define DISPLAY_BITS_PER_PIXEL_INPUT1 (4)
            #else
            #define DISPLAY_BITS_PER_PIXEL_INPUT1 (1)
            #endif
            #define DISPLAY_HSIZE_INPUT1                 (480)
            #define DISPLAY_VSIZE_INPUT1                 (854)
            #define DISPLAY_BUFFER_STRIDE_BYTES_INPUT1   (((DISPLAY_HSIZE_INPUT1 * DISPLAY_BITS_PER_PIXEL_INPUT1 + 0x1FF) >> 9) << 6)
            #define DISPLAY_BUFFER_STRIDE_PIXELS_INPUT1  ((DISPLAY_BUFFER_STRIDE_BYTES_INPUT1 * 8) / DISPLAY_BITS_PER_PIXEL_INPUT1)
            #if GLCDC_CFG_LAYER_2_ENABLE
            extern uint8_t fb_foreground[2][DISPLAY_BUFFER_STRIDE_BYTES_INPUT1 * DISPLAY_VSIZE_INPUT1];
            #endif
/** External IRQ on ICU Instance. */
extern const external_irq_instance_t g_external_irq4;

/** Access the ICU instance using these structures when calling API functions directly (::p_api is not used). */
extern icu_instance_ctrl_t g_external_irq4_ctrl;
extern const external_irq_cfg_t g_external_irq4_cfg;

#ifndef irq_callback
void irq_callback(external_irq_callback_args_t * p_args);
#endif
#if DRW_CFG_CUSTOM_MALLOC
            void * d1_malloc(size_t size);
            void   d1_free(void * ptr);
            #endif
#ifndef ETHER_PHY_LSI_TYPE_KIT_COMPONENT
  #define ETHER_PHY_LSI_TYPE_KIT_COMPONENT ETHER_PHY_LSI_TYPE_DEFAULT
#endif

#ifndef rmac_phy_target_rtl8211_initialize
void rmac_phy_target_rtl8211_initialize(rmac_phy_instance_ctrl_t * p_instance_ctrl);
#endif

#ifndef rmac_phy_target_rtl8211_is_support_link_partner_ability
bool rmac_phy_target_rtl8211_is_support_link_partner_ability(rmac_phy_instance_ctrl_t * p_instance_ctrl, uint32_t line_speed_duplex);
#endif

/** ETHER_PHY on RMAC_PHY Instance. */
extern const ether_phy_instance_t g_rmac_phy1;

/** Access the Ethernet PHY instance using these structures when calling API functions directly (::p_api is not used). */
extern rmac_phy_instance_ctrl_t g_rmac_phy1_ctrl;
extern const ether_phy_cfg_t g_rmac_phy1_cfg;
extern const rmac_phy_extended_cfg_t g_rmac_phy1_extended_cfg;

#ifndef NULL
void NULL(ether_phy_callback_args_t * p_args);
#endif
#ifndef ETHER_PHY_LSI_TYPE_KIT_COMPONENT
  #define ETHER_PHY_LSI_TYPE_KIT_COMPONENT ETHER_PHY_LSI_TYPE_DEFAULT
#endif

#ifndef rmac_phy_target_rtl8211_initialize
void rmac_phy_target_rtl8211_initialize(rmac_phy_instance_ctrl_t * p_instance_ctrl);
#endif

#ifndef rmac_phy_target_rtl8211_is_support_link_partner_ability
bool rmac_phy_target_rtl8211_is_support_link_partner_ability(rmac_phy_instance_ctrl_t * p_instance_ctrl, uint32_t line_speed_duplex);
#endif

/** ETHER_PHY on RMAC_PHY Instance. */
extern const ether_phy_instance_t g_rmac_phy0;

/** Access the Ethernet PHY instance using these structures when calling API functions directly (::p_api is not used). */
extern rmac_phy_instance_ctrl_t g_rmac_phy0_ctrl;
extern const ether_phy_cfg_t g_rmac_phy0_cfg;
extern const rmac_phy_extended_cfg_t g_rmac_phy0_extended_cfg;

#ifndef NULL
void NULL(ether_phy_callback_args_t * p_args);
#endif
#ifndef NULL
void NULL(ether_switch_callback_args_t * p_args);
#endif
#ifndef NULL
void NULL(ether_switch_callback_args_t * p_args);
#endif
#ifndef NULL
void NULL(ether_switch_callback_args_t * p_args);
#endif

/** Ethernet Switch on Layer3 Switch Instance. */
extern const ether_switch_instance_t g_layer3_switch0;

/** Access the Layer3 Switch instance using these structures when calling API functions directly (::p_api is not used). */
extern layer3_switch_instance_ctrl_t g_layer3_switch0_ctrl;
extern const ether_switch_cfg_t g_layer3_switch0_cfg;
#define IOPORT_CFG_NAME g_bsp_pin_cfg
#define IOPORT_CFG_OPEN R_IOPORT_Open
#define IOPORT_CFG_CTRL g_ioport_ctrl

/* IOPORT Instance */
extern const ioport_instance_t g_ioport;

/* IOPORT control structure. */
extern ioport_instance_ctrl_t g_ioport_ctrl;
void g_common_init(void);
FSP_FOOTER
#endif /* COMMON_DATA_H_ */
