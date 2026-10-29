/* Titan Board HW V1.1 labels, paired with ra_gen/pin_data.c. */
#ifndef BSP_PIN_CFG_H_
#define BSP_PIN_CFG_H_
#include "r_ioport.h"
FSP_HEADER
#define LED_R BSP_IO_PORT_00_PIN_13
#define LED_G BSP_IO_PORT_06_PIN_13
#define LED_B BSP_IO_PORT_00_PIN_12
#define USER_KEY BSP_IO_PORT_01_PIN_10
#define CAMERA_RESET BSP_IO_PORT_04_PIN_11
#define CAMERA_CLKOUT BSP_IO_PORT_10_PIN_11
#define USB_CH_SEL BSP_IO_PORT_04_PIN_13
#define HYPERRAM_RESET BSP_IO_PORT_12_PIN_07
#define SD0_CD BSP_IO_PORT_13_PIN_07
extern const ioport_cfg_t g_bsp_pin_cfg;
void R_BSP_PinCfgSecurityInit(void);
FSP_FOOTER
#endif
