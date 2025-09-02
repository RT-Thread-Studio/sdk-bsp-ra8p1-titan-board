/*
 * Copyright (c) 2006-2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author        Notes
 * 2024-03-11     kurisaW   first version
 */

#include <rtthread.h>
#include "hal_data.h"
#include <rtdevice.h>
#include <board.h>
#include "factory_test.h"
#include "camera_layer.h"
#include "camera_layer_config.h"

#define LED_PIN     BSP_IO_PORT_00_PIN_12 /* Onboard LED pin */
#define USER_BTN    BSP_IO_PORT_01_PIN_10 /* User key pin */

#define DISPLAY_SCREEN_WIDTH              (800)
#define DISPLAY_SCREEN_HEIGHT             (480)

extern struct rt_completion ceu_completion;
uint8_t display_layer1_buff_select = 0;

void user_callback(void)
{
    rt_kprintf("user callback.\n");
}

void hal_entry(void)
{
    rt_kprintf("\n==================================================\n");
    rt_kprintf("Hello, Titan Board!\n");
    rt_kprintf("Running Titan Board factory test code.\n");
    rt_kprintf("==================================================\n");

    rt_pin_write(LED_RED, PIN_HIGH);
    rt_pin_attach_irq(USER_BTN, PIN_IRQ_MODE_FALLING, user_callback, RT_NULL);
    rt_pin_irq_enable(USER_BTN, PIN_IRQ_ENABLE);

    adc_init();

    wifi_connect("TitanBoard", "12345678");

    test_board_entry();

    //Initialize camera interface
    fsp_err_t fsp_status = FSP_SUCCESS;
    fsp_status = camera_init(false);
    if(FSP_SUCCESS != fsp_status)
    {
        rt_kprintf ("camera_init fail!\n");
        return;
    }

    camera_image_buffer_initialize ();

    camera_capture_start ();

#if defined(VIN_CFG_USE_RUNTIME_BUFFER)
    rt_kprintf("The vin driver uses hardware mailboxes for the buffer.\n");
#else
    rt_kprintf("The vin driver uses isr for the buffer.\n");
#endif

    while (1)
    {
#ifndef VIN_CFG_USE_RUNTIME_BUFFER
        rt_completion_wait(&ceu_completion, RT_WAITING_FOREVER);
#endif

        /* Draw camera image to display buffer */
        uint16_t * p_src  = (uint16_t *)camera_data_ready_buffer_pointer_get();
        uint16_t * p_dest = (uint16_t *)&fb_background[display_layer1_buff_select][0];
        int x_offset = DISPLAY_SCREEN_WIDTH - CAMERA_CAPTURE_IMAGE_WIDTH;

        for(int y = 0; y < CAMERA_CAPTURE_IMAGE_HEIGHT; y++)
        {
            for(int x = 0; x < CAMERA_CAPTURE_IMAGE_WIDTH; x++)
            {
                *(p_dest + (y * DISPLAY_SCREEN_WIDTH) + (x_offset + x)) = *(p_src + (y * CAMERA_CAPTURE_IMAGE_WIDTH) + x);
            }
        }

        rt_thread_mdelay (10);
    }
}

void led_entry(void *param)
{
    while (1)
    {
        rt_pin_write(LED_PIN, PIN_HIGH);
        rt_thread_mdelay(500);
        rt_pin_write(LED_PIN, PIN_LOW);
        rt_thread_mdelay(500);
    }
}

int led_thread(void)
{
    rt_thread_t tid = rt_thread_create("led", led_entry, RT_NULL, 512, 15, 5);
    return rt_thread_startup(tid);
}
INIT_APP_EXPORT(led_thread);
