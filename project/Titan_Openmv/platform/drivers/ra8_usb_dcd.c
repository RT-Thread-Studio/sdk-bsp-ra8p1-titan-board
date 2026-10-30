/* SPDX-License-Identifier: Apache-2.0 */
#include <rtthread.h>
#include "tusb_option.h"
#include "device/dcd.h"
#include "device/usbd.h"
#include "ra8_rusb2_regs.h"
#include "ra8_usb_dcd.h"
#include "../port/rusb2_ra8.h"

/* ACLRM readback timing is proven for this PCLKA and the platform's
 * BWAIT >= 7 setting (reset BWAIT = 15 also meets the bound). */
_Static_assert(BSP_STARTUP_PCLKA_HZ <= 125000000U,
               "Review USB ACLRM timing if PCLKA is increased");

#define dcd_edpt_xfer ra8_edpt_xfer_unlocked
#define dcd_edpt_xfer_fifo ra8_edpt_xfer_fifo_unlocked
#include "../port/dcd_rusb2_ra8.c"
#undef dcd_edpt_xfer
#undef dcd_edpt_xfer_fifo

static void ra8_resume_parked(uint8_t port, uint8_t ep, bool is_isr)
{
    unsigned num = _dcd.ep[tu_edpt_dir(ep)][tu_edpt_number(ep)];
    if (!tu_edpt_dir(ep) && num && (ra8_usb_rx_parked & TU_BIT(num))) {
        /* The unarmed IRQ already acknowledged BRDYSTS. No new edge is
         * guaranteed for its unread bank, so deliver it on this submission.
         */
        ra8_usb_rx_parked &= (uint16_t)~TU_BIT(num);
        if (pipe_xfer_out(RUSB2_REG(port), num)) {
            pipe_xfer_complete(port, num, is_isr);
        }
    }
}

bool dcd_edpt_xfer(uint8_t port, uint8_t ep, uint8_t *buffer, uint16_t size, bool is_isr)
{
    rt_base_t level = rt_hw_interrupt_disable();
    bool result = ra8_edpt_xfer_unlocked(port, ep, buffer, size, is_isr);
    if (result) { ra8_resume_parked(port, ep, is_isr); }
    rt_hw_interrupt_enable(level);
    return result;
}

bool dcd_edpt_xfer_fifo(uint8_t port, uint8_t ep, tu_fifo_t *fifo, uint16_t size, bool is_isr)
{
    /* Current CDC uses this path to avoid the extra endpoint RAM copy. */
    rt_base_t level = rt_hw_interrupt_disable();
    bool result = ra8_edpt_xfer_fifo_unlocked(port, ep, fifo, size, is_isr);
    if (result) { ra8_resume_parked(port, ep, is_isr); }
    rt_hw_interrupt_enable(level);
    return result;
}
