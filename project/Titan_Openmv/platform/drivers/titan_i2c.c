/*
 * Copyright (c) 2006-2022, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-02-22     airm2m       first version
 */

#include <rtdevice.h>
#include <rtthread.h>
#include "board.h"

#include <stdlib.h>
#include "titan_machine_vectors.h"

#ifdef BSP_USING_HW_I2C

#define DBG_TAG              "drv.hwi2c"
#define DBG_LVL              DBG_INFO
#include <rtdbg.h>

#include <hal_data.h>

#ifndef BIT
    #define BIT(idx)        (1ul << (idx))
#endif

#ifndef BITS
    #define BITS(b,e)       ((((uint32_t)-1)<<(b))&(((uint32_t)-1)>>(31-(e))))
#endif

#define RA_SCI_EVENT_ABORTED        BIT(0)
#define RA_SCI_EVENT_RX_COMPLETE    BIT(1)
#define RA_SCI_EVENT_TX_COMPLETE    BIT(2)
#define RA_SCI_EVENT_ERROR          BIT(3)
#define RA_SCI_EVENT_ALL            BITS(0,3)

struct ra_i2c_handle
{
    struct rt_i2c_bus_device bus;
    char bus_name[RT_NAME_MAX];
    const i2c_master_cfg_t *i2c_cfg;
    void *i2c_ctrl;
    struct rt_event event;
};

#ifdef BSP_USING_HW_I2C1
/* I2C1 is routed to the CEU connector on P511/P512 (schematic pages 4/7).
 * Use the same PCLKB and verified 400 kHz timing as I2C0. */
static iic_master_instance_ctrl_t titan_i2c1_ctrl;
static i2c_master_cfg_t titan_i2c1_cfg;
#endif

static struct ra_i2c_handle ra_i2cs[] =
{
#ifdef BSP_USING_HW_I2C0
    {.bus_name = "i2c0", .i2c_cfg = &g_i2c_master0_cfg, .i2c_ctrl = &g_i2c_master0_ctrl,},
#endif
#ifdef BSP_USING_HW_I2C1
    {.bus_name = "i2c1", .i2c_cfg = &titan_i2c1_cfg, .i2c_ctrl = &titan_i2c1_ctrl,},
#endif
#ifdef BSP_USING_HW_I2C2
    {.bus_name = "i2c2", .i2c_cfg = &g_i2c_master2_cfg, .i2c_ctrl = &g_i2c_master2_ctrl,},
#endif
};

void i2c_master_callback(i2c_master_callback_args_t *p_args)
{
    rt_interrupt_enter();
    if (NULL != p_args)
    {
        /* capture callback event for validating the i2c transfer event*/
        struct ra_i2c_handle *obj = (struct ra_i2c_handle *)p_args->p_context;
        uint32_t event = 0;
        RT_ASSERT(obj != RT_NULL);
        switch (p_args->event)
        {
        case I2C_MASTER_EVENT_ABORTED:
            event |= RA_SCI_EVENT_ABORTED;
            break;
        case I2C_MASTER_EVENT_RX_COMPLETE:
            event |= RA_SCI_EVENT_RX_COMPLETE;
            break;
        case I2C_MASTER_EVENT_TX_COMPLETE:
            event |= RA_SCI_EVENT_TX_COMPLETE;
            break;
        case I2C_MASTER_EVENT_START:
        case I2C_MASTER_EVENT_BYTE_ACK:
            /* Intermediate notifications do not complete a transfer. */
            break;
        }
        if (event != 0)
        {
            rt_event_send(&obj->event, event);
        }
    }
    rt_interrupt_leave();
}

static void clear_i2c_events(struct ra_i2c_handle *handle)
{
    rt_uint32_t stale;
    (void)rt_event_recv(&handle->event, RA_SCI_EVENT_ALL,
                       RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, 0, &stale);
}

static rt_err_t validate_i2c_event(struct ra_i2c_handle *handle,
                                 rt_uint32_t expected, rt_uint32_t *observed)
{
    *observed = 0;
    rt_err_t result = rt_event_recv(&handle->event, RA_SCI_EVENT_ALL,
        RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, rt_tick_from_millisecond(100), observed);
    if (result != RT_EOK) { return result; }
    return *observed == expected ? RT_EOK : -RT_ERROR;
}

static rt_ssize_t ra_i2c_mst_xfer(struct rt_i2c_bus_device *bus,
                                struct rt_i2c_msg msgs[], rt_uint32_t num)
{
    RT_ASSERT(bus != RT_NULL);
    struct ra_i2c_handle *handle = rt_container_of(bus, struct ra_i2c_handle, bus);
    i2c_master_ctrl_t *ctrl = handle->i2c_ctrl;
    rt_uint32_t i;
    clear_i2c_events(handle);
    for (i = 0; i < num; i++)
    {
        struct rt_i2c_msg *msg = &msgs[i];
        bool read = (msg->flags & RT_I2C_RD) != 0;
        bool restart = (i + 1U < num) || ((msg->flags & RT_I2C_NO_STOP) != 0);
        /* FSP cannot concatenate raw byte fragments without an address phase.
         * The OpenMV/machine adapters join such writes before entering here. */
        if ((!msg->buf && msg->len) || (read && !msg->len) || (msg->flags & RT_I2C_NO_START) ||
            (i && (msg->addr != msgs[0].addr ||
                   ((msg->flags ^ msgs[0].flags) & RT_I2C_ADDR_10BIT))))
        {
            LOG_E("%s unsupported message %u flags=0x%x len=%u",
                  handle->bus_name, (unsigned)i, msg->flags, msg->len);
            break;
        }
        fsp_err_t err = FSP_SUCCESS;
        /* Do not call SlaveAddressSet between the write and repeated-start
         * read: its checked FSP API rejects an active restart transaction. */
        if (!i)
        {
            err = R_IIC_MASTER_SlaveAddressSet(ctrl, msg->addr,
                msg->flags & RT_I2C_ADDR_10BIT ?
                I2C_MASTER_ADDR_MODE_10BIT : I2C_MASTER_ADDR_MODE_7BIT);
        }
        if (err != FSP_SUCCESS)
        {
            LOG_E("%s address=0x%02x set failed fsp=%d", handle->bus_name, msg->addr, err);
            break;
        }
        clear_i2c_events(handle);
        /* R_IIC_MASTER_Write permits bytes=0, but still requires a non-NULL
         * pointer when FSP parameter checks are enabled. It never dereferences
         * this dummy byte: the TXI/TEI state machine only emits the address.
         * Keep it live until completion/Abort, just like normal caller data. */
        uint8_t address_probe = 0;
        err = read ? R_IIC_MASTER_Read(ctrl, msg->buf, msg->len, restart) :
                     R_IIC_MASTER_Write(ctrl, msg->len ? msg->buf : &address_probe, msg->len, restart);
        if (err != FSP_SUCCESS)
        {
            LOG_E("%s addr=0x%02x %s[%u] len=%u start failed fsp=%d",
                  handle->bus_name, msg->addr, read ? "read" : "write",
                  (unsigned)i, msg->len, err);
            break;
        }
        rt_uint32_t observed;
        rt_err_t result = validate_i2c_event(handle,
            read ? RA_SCI_EVENT_RX_COMPLETE : RA_SCI_EVENT_TX_COMPLETE, &observed);
        if (result != RT_EOK)
        {
            /* NACK is expected for absent addresses during scan(). */
            if (msg->len || result == -RT_ETIMEOUT) {
                LOG_E("%s addr=0x%02x %s[%u] len=%u restart=%u result=%d event=0x%x",
                      handle->bus_name, msg->addr, read ? "read" : "write",
                      (unsigned)i, msg->len, restart, result, (unsigned)observed);
            }
            break;
        }
    }
    if (i != num)
    {
        (void)R_IIC_MASTER_Abort(ctrl);
        clear_i2c_events(handle);
    }
    return (rt_ssize_t)i;
}

static const struct rt_i2c_bus_device_ops ra_i2c_ops =
{
    .master_xfer        = ra_i2c_mst_xfer,
    .slave_xfer         = RT_NULL,
    .i2c_bus_control    = RT_NULL
};

int ra_hw_i2c_init(void)
{
    fsp_err_t err     = FSP_SUCCESS;
#ifdef BSP_USING_HW_I2C1
    titan_i2c1_cfg = g_i2c_master0_cfg;
    titan_i2c1_cfg.channel = 1;
    titan_i2c1_cfg.rxi_irq = TITAN_IIC1_RXI_IRQ;
    titan_i2c1_cfg.txi_irq = TITAN_IIC1_TXI_IRQ;
    titan_i2c1_cfg.tei_irq = TITAN_IIC1_TEI_IRQ;
    titan_i2c1_cfg.eri_irq = TITAN_IIC1_ERI_IRQ;
    const uint32_t pin_cfg = IOPORT_CFG_PERIPHERAL_PIN | IOPORT_PERIPHERAL_IIC;
    if (R_IOPORT_PinCfg(&g_ioport_ctrl, BSP_IO_PORT_05_PIN_11, pin_cfg) != FSP_SUCCESS ||
        R_IOPORT_PinCfg(&g_ioport_ctrl, BSP_IO_PORT_05_PIN_12, pin_cfg) != FSP_SUCCESS) {
        return -RT_EIO;
    }
#endif
    for (rt_uint32_t i = 0; i < sizeof(ra_i2cs) / sizeof(ra_i2cs[0]); i++)
    {
        ra_i2cs[i].bus.ops = &ra_i2c_ops;
        ra_i2cs[i].bus.priv = 0;

        if (RT_EOK != rt_event_init(&ra_i2cs[i].event, ra_i2cs[i].bus_name, RT_IPC_FLAG_FIFO))
        {
            LOG_E("Init event failed");
            continue;
        }
        /* opening IIC master module */
        err = R_IIC_MASTER_Open(ra_i2cs[i].i2c_ctrl, ra_i2cs[i].i2c_cfg);
        if (FSP_SUCCESS != err)
        {
            LOG_E("R_I2C_MASTER_Open API failed,%d", err);
            rt_event_detach(&ra_i2cs[i].event);
            continue;
        }
        err = R_IIC_MASTER_CallbackSet(ra_i2cs[i].i2c_ctrl, i2c_master_callback, &ra_i2cs[i], RT_NULL);
        /* handle error */
        if (FSP_SUCCESS != err)
        {
            LOG_E("R_I2C_CallbackSet API failed,%d", err);
            R_IIC_MASTER_Close(ra_i2cs[i].i2c_ctrl);
            rt_event_detach(&ra_i2cs[i].event);
            continue;
        }
        rt_i2c_bus_device_register(&ra_i2cs[i].bus, ra_i2cs[i].bus_name);
    }

    return 0;
}
INIT_DEVICE_EXPORT(ra_hw_i2c_init);

#endif /* BSP_USING_I2C */
