/* SPDX-License-Identifier: Apache-2.0 */
#include <rtthread.h>
/* This BSP selects BSP_CFG_RTOS=0: FSP_CONTEXT_SAVE/RESTORE are empty.
 * Keep the FSP device clock/IRQ body; platform owns RT nesting and cache. */
#define ethosu_flush_dcache titan_fsp_unused_flush_dcache
#define ethosu_invalidate_dcache titan_fsp_unused_invalidate_dcache
#define rm_ethosu_isr titan_fsp_ethosu_isr
#include <fsp/src/rm_ethosu/rm_ethosu.c>
#undef rm_ethosu_isr
#if BSP_CFG_RTOS != 0
#error "Review NPU interrupt nesting for the selected FSP RTOS"
#endif
void rm_ethosu_isr(void)
{
    rt_interrupt_enter();
    titan_fsp_ethosu_isr();
    rt_interrupt_leave();
}
