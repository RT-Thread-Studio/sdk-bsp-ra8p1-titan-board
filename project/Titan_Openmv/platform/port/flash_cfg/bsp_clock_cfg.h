/* Project Flash clock override; common clocks come from RASC output. */
#ifndef TITAN_FLASH_BSP_CLOCK_CFG_H
#define TITAN_FLASH_BSP_CLOCK_CFG_H
#include <ra_gen/bsp_clock_cfg.h>
#undef BSP_CFG_OCTACLK_SOURCE
#undef BSP_CFG_OCTACLK_DIV
#define BSP_CFG_OCTACLK_SOURCE (BSP_CLOCKS_SOURCE_CLOCK_MAIN_OSC)
#define BSP_CFG_OCTACLK_DIV (BSP_CLOCKS_OCTA_CLOCK_DIV_1)
#endif
