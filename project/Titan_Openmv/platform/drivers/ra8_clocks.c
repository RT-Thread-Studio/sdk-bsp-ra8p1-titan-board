/* SPDX-License-Identifier: Apache-2.0 */
/* Keep RASC/FSP output intact; complete the SDCLK selector setup before
 * SystemInit configures HyperRAM pins or initializes external memory. */
#include "bsp_api.h"
#define bsp_clock_init titan_fsp_clock_init
#include <fsp/src/bsp/mcu/all/bsp_clocks.c>
#undef bsp_clock_init

void bsp_clock_init(void)
{
    titan_fsp_clock_init();

#if BSP_CFG_HyperRAM_ENABLED && BSP_CFG_EBCLKA_SEL
    /* FSP 6.4 only sets BCKCR when the EBCLK pin is enabled. SDCLK uses
     * that same selector even with EBCLK disabled. Follow RA8P1 HW manual
     * 9.2.64: stop outputs and update BCKCR while BCKACKSRDY is set.
     * C runtime is not initialized yet, so use registers rather than the
     * reference-counted register-protection API. */
    uint16_t protect = R_SYSTEM->PRCR;
    R_SYSTEM->PRCR = BSP_PRV_PRCR_KEY | protect | R_SYSTEM_PRCR_PRC0_Msk;
    R_SYSTEM->EBCKOCR = 0U;
    R_SYSTEM->SDCKOCR = 0U;
    R_SYSTEM->BCKACR_b.CKSREQ = 1U;
    FSP_HARDWARE_REGISTER_WAIT(R_SYSTEM->BCKACR_b.CKSRDY, 1U);

    R_SYSTEM->BCKCR = 0U;
    R_SYSTEM->BCKADIVCR = BSP_CFG_BCLKA_DIV;
    R_SYSTEM->BCKACR = BSP_CFG_BCLKA_SOURCE | R_SYSTEM_BCKACR_CKSREQ_Msk;
    R_SYSTEM->BCKCR = (BSP_CFG_EBCLKA_SEL << R_SYSTEM_BCKCR_EBCKASEL_Pos)
#if BSP_CFG_BCLK_OUTPUT > 0U
                     | (BSP_CFG_BCLK_OUTPUT - 1U)
#endif
                     ;
    R_SYSTEM->BCKACR_b.CKSREQ = 0U;
    FSP_HARDWARE_REGISTER_WAIT(R_SYSTEM->BCKACR_b.CKSRDY, 0U);
    R_SYSTEM->EBCKOCR = (BSP_CFG_BCLK_OUTPUT > 0U);
    R_SYSTEM->SDCKOCR = BSP_CFG_SDCLK_OUTPUT;
    R_SYSTEM->PRCR = BSP_PRV_PRCR_KEY | protect;
    __DSB();
    __ISB();
#endif
}
