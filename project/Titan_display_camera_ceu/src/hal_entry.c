/*
 * Copyright (c) 2006-2025, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author        Notes
 * 2024-06-11     kurisaw       first version
 */

#include <rtthread.h>
#include "hal_data.h"
#include <rtdevice.h>

#include <board.h>
#include "sensor.h"

#define LED_PIN_1    BSP_IO_PORT_00_PIN_12

volatile bool led_status = false;

#define CAM_WIDTH   640
#define CAM_HEIGHT  480

extern sensor_t sensor;
uint8_t g_image_rgb565_sdram_buffer[CAM_WIDTH * CAM_HEIGHT * 2] BSP_PLACE_IN_SECTION(".ospi1_cs0_noinit") BSP_ALIGN_VARIABLE(8);

void hal_entry(void)
{
    rt_kprintf("\nHello RT-Thread!\n");
    rt_kprintf("===========================================================\n");
    rt_kprintf("This example project is an display with ceu camera routine!\n");
    rt_kprintf("===========================================================\n\n");

    sensor_init();
    sensor_reset();
    sensor_set_pixformat(PIXFORMAT_RGB565);
    sensor_set_framesize(FRAMESIZE_VGA);

    while(1)
    {
        sensor_snapshot(&sensor, g_image_rgb565_sdram_buffer, 0);

        lcd_draw_jpg(0, 0, g_image_rgb565_sdram_buffer, CAM_WIDTH, CAM_HEIGHT);

        rt_thread_mdelay(1);

        led_status = !led_status;
        if(led_status)
        {
            rt_pin_write(LED_PIN_1, PIN_HIGH);
        }
        else
        {
            rt_pin_write(LED_PIN_1, PIN_LOW);
        }
    }
}
