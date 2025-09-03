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

#include "FreeRTOS.h"
#include <queue.h>
#include "usb_hhid_appl.h"

QueueHandle_t g_event_queue;
QueueHandle_t g_data_queue;
static StaticQueue_t g_event_queue_memory;

#define QUEUE_LENGTH        4 * 20
#define ITEM_SIZE sizeof( uint32_t )

static uint8_t g_event_queue_queue_memory[QUEUE_BUFFER_SIZE(QUEUE_LENGTH, ITEM_SIZE)];
static StaticQueue_t g_data_queue_memory;
static uint8_t g_data_queue_queue_memory[QUEUE_BUFFER_SIZE(QUEUE_LENGTH, ITEM_SIZE)];

#define LED_PIN_0    BSP_IO_PORT_00_PIN_12 /* Onboard LED pins */

void hal_entry(void)
{
    rt_kprintf("\nHello RT-Thread!\n");
    rt_kprintf("==================================================\n");
    rt_kprintf("This example project is an usb hhid routine!\n");
    rt_kprintf("Please connect the FS USB development board to the PC.\n");
    rt_kprintf("Please connect a USB keyboard to the USB FS interface of the board.\n");
    rt_kprintf("==================================================\n");

    rt_kprintf("Notices: hhid only supports FS.\n");

    g_data_queue  = xQueueCreateStatic (20, 4, &g_data_queue_queue_memory[0], &g_data_queue_memory);
    g_event_queue = xQueueCreateStatic (20, 4, &g_event_queue_queue_memory[0], &g_event_queue_memory);

    rt_thread_t hhid_event = rt_thread_create("hhid_event", usb_app_thread_entry, RT_NULL, 1024, 20, 10);
    rt_thread_t hhid_data = rt_thread_create("hhid_data", usb_logger_thread_entry, RT_NULL, 1024, 20, 10);

    rt_thread_startup(hhid_event);
    rt_thread_startup(hhid_data);

    while (1)
    {
        rt_pin_write(LED_PIN_0, PIN_HIGH);
        rt_thread_mdelay(1000);
        rt_pin_write(LED_PIN_0, PIN_LOW);
        rt_thread_mdelay(1000);
    }
}
