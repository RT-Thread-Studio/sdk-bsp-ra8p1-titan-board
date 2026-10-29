/* SPDX-License-Identifier: Apache-2.0 */
/* Hardware behavior follows the verified Titan_Mini_driver_all SDK driver. */
#include "titan_sdhi.h"
#include <errno.h>
#include <drv_sdhi.h>
static volatile rt_bool_t sd_media_invalidated;
static volatile rt_bool_t sd_enumeration_complete;
static rt_bool_t sd_probe_started;
#include "../port/titan_sdhi_host.c"

/* Read the physical CD pad without altering its reference peripheral mux. */
static fsp_err_t titan_sdhi_cd_read(bsp_io_level_t *level)
{
    return R_IOPORT_PinRead(&g_ioport_ctrl, BSP_IO_PORT_13_PIN_07, level);
}

int titan_sdhi_present(void)
{
    bsp_io_level_t level = BSP_IO_LEVEL_HIGH;
    if (!host0 || sd_media_invalidated) { return 0; }
    if (titan_sdhi_cd_read(&level) != FSP_SUCCESS) { return 0; }
    if (level != BSP_IO_LEVEL_LOW)
    {
        /* Also cover a removal observed before the CARD_REMOVED interrupt. */
        if (sd_enumeration_complete) { sd_media_invalidated = RT_TRUE; }
        return 0;
    }
    return 1;
}

int titan_sdhi_is_ready(void)
{
    return sd_enumeration_complete && titan_sdhi_present();
}

int titan_sdhi_write_protected(void)
{
    /* This board's microSD socket has no mechanical WP signal. The saved SD
     * CSD still provides permanent/temporary protection in bits 13 and 12. */
    return !titan_sdhi_is_ready() || (host0->card->resp_csd[3] & (3U << 12)) != 0;
}

int titan_sdhi_probe(void)
{
    if (!host0)
    {
        return -EIO;
    }
    if (sd_media_invalidated)
    {
        return -EIO;
    }
    for (unsigned sample = 0; sample < 3; sample++)
    {
        if (!titan_sdhi_present())
        {
            return -ENODEV;
        }
        rt_thread_mdelay(5);
    }
    /* mmcsd_change toggles attach/detach. Do not repeat an in-flight probe. */
    rt_base_t irq_level = rt_hw_interrupt_disable();
    if (!sd_probe_started)
    {
        sd_probe_started = RT_TRUE;
        mmcsd_change(host0);
        rt_hw_interrupt_enable(irq_level);
    }
    else
    {
        rt_hw_interrupt_enable(irq_level);
    }
    return RT_EOK;
}

void titan_sdhi_probe_complete(int success)
{
    if (success && host0 && host0->card && !sd_media_invalidated)
    {
        sd_enumeration_complete = RT_TRUE;
    }
    if (!success && host0 && !host0->card && !sd_media_invalidated)
    {
        sd_probe_started = RT_FALSE;
    }
}

int titan_sdhi_sync(void)
{
    if (!titan_sdhi_is_ready()) { return -ENODEV; }
    /* The generic MMCSD driver's BLK_SYNC ioctl does not issue a command.
     * A WRITE transfer can finish while the SD card is still programming.
     * CMD13 is the synchronization barrier for MSC writes and safe eject. */
    mmcsd_host_lock(host0);
    const rt_tick_t started = rt_tick_get();
    const rt_tick_t timeout = rt_tick_from_millisecond(1000);
    int result = -ETIMEDOUT;
    do
    {
        if (!titan_sdhi_present()) { result = -ENODEV; break; }
        struct rt_mmcsd_cmd cmd = {0};
        cmd.cmd_code = SEND_STATUS;
        cmd.arg = host0->card->rca << 16;
        cmd.flags = RESP_R1 | CMD_AC;
        if (mmcsd_send_cmd(host0, &cmd, 0) != RT_EOK) { result = -EIO; break; }
        const uint32_t error_mask = R1_OUT_OF_RANGE | R1_ADDRESS_ERROR | R1_BLOCK_LEN_ERROR |
            R1_WP_VIOLATION | R1_CARD_IS_LOCKED | R1_COM_CRC_ERROR | R1_ILLEGAL_COMMAND |
            R1_CARD_ECC_FAILED | R1_CC_ERROR | R1_ERROR | R1_UNDERRUN | R1_OVERRUN;
        if (cmd.resp[0] & error_mask) { result = -EIO; break; }
        if ((cmd.resp[0] & R1_READY_FOR_DATA) && R1_CURRENT_STATE(cmd.resp[0]) == 4)
        {
            result = RT_EOK;
            break;
        }
        rt_thread_mdelay(1);
    } while ((rt_tick_t)(rt_tick_get() - started) < timeout);
    mmcsd_host_unlock(host0);
    return result;
}
