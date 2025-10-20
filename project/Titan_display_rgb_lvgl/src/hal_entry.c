/*
 * Copyright (c) 2006-2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author        Notes
 * 2024-03-11     kurisaw       first version
 */

#include <rtthread.h>
#include "hal_data.h"
#include <rtdevice.h>
#include <board.h>

#define DBG_TAG "lcd"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define LED_PIN_0    BSP_IO_PORT_00_PIN_12 /* Onboard LED pins */

void hal_entry(void)
{
    rt_kprintf("\nHello RT-Thread!\n");
    rt_kprintf("==================================================\n");
    rt_kprintf("This example project is an rgb lcd routine!\n");
    rt_kprintf("==================================================\n");

    while (1)
    {
        rt_pin_write(LED_PIN_0, PIN_HIGH);
        rt_thread_mdelay(1000);
        rt_pin_write(LED_PIN_0, PIN_LOW);
        rt_thread_mdelay(1000);
    }
}

static void i2c_scan(int argc, char **argv)
{
    struct rt_i2c_bus_device *i2c_bus;

    if (argc < 2)
    {
        rt_kprintf("Usage: i2c_scan <bus_name>\n");
        return;
    }

    i2c_bus = rt_i2c_bus_device_find(argv[1]);
    if (i2c_bus == RT_NULL) {
        rt_kprintf("I2C bus %s not found!\n", argv[1]);
        return;
    }

    rt_kprintf("Scanning I2C bus %s...\n", argv[1]);
    rt_kprintf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");

    for (rt_uint8_t i = 0; i < 0x80; i++)
    {
        if (i % 16 == 0)
        {
            rt_kprintf("%02x: ", i);
        }

        struct rt_i2c_msg msgs;
        rt_uint8_t dummy;

        msgs.addr = i;
        msgs.flags = RT_I2C_RD;
        msgs.buf = &dummy;
        msgs.len = 1;

        if (rt_i2c_transfer(i2c_bus, &msgs, 1) == 1) {
            rt_kprintf("%02x ", i);
        } else {
            rt_kprintf("-- ");
        }

        if (i % 16 == 15) rt_kprintf("\n");
    }
}
MSH_CMD_EXPORT(i2c_scan, Scan I2C bus);
