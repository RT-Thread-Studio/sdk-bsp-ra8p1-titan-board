/* FSP initializes hardware/memory; RT-Thread runs constructors after heap init. */
#include <stdint.h>
#if defined(__ARMCC_VERSION) && !defined(__MICROLIB)
/* Reset_Handler calls main directly, so Arm's __main/__rt_entry never runs.
 * Enable FSP's standard-library initialization hook for stdio, locale and
 * library state. RT-Thread's existing $Sub$$__cpp_initialize__aeabi_ keeps
 * constructors deferred until cplusplus_system_init after the RT heap exists.
 */
#ifndef __ARMCC_USING_STANDARDLIB
#define __ARMCC_USING_STANDARDLIB 1
#endif
#endif
#ifdef __ARMCC_VERSION
/* ALIGN sets a region's start, not its limit. This retained last section
 * makes the ITCM load/copy extent an exact multiple of the ECC granule. */
const uint64_t ra8_itcm_end_padding __attribute__((section(".itcm_padding"), aligned(8), used)) = 0;
#endif
#if defined(__GNUC__) && !defined(__ARMCC_VERSION)
#define __init_array_start __fsp_init_array_start
#define __init_array_end __fsp_init_array_end
#endif
#include <fsp/src/bsp/cmsis/Device/RENESAS/Source/system.c>
