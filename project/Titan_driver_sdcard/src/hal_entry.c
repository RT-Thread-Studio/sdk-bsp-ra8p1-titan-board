/*
 * Copyright (c) 2006-2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author        Notes
 * 2024-03-11     kurisaw       first version
 */

/*
* SD-Card Pin mapping:
* ===========================
* CS             ---> NULL
* SD CMD(MOSI)   ---> P401
* SD DATA0(MISO) ---> P402
* SD CLK(SCK)    ---> P400
* SD DAT1(DAT1)  ---> P403
* SD DAT2(DAT2)  ---> P404
* SD CD(CD)      ---> NULL
* SD WP(NC)      ---> NULL
*/

#include <rtthread.h>
#include "hal_data.h"
#include <rtdevice.h>
#include <board.h>

#define LED_PIN_0    BSP_IO_PORT_00_PIN_12 /* Onboard LED pins */

void hal_entry(void)
{
    rt_kprintf("\nHello RT-Thread!\n");
    rt_kprintf("==================================================\n");
    rt_kprintf("This example project is the sdio routine!\n");
    rt_kprintf("==================================================\n");

    while (1)
    {
        rt_pin_write(LED_PIN_0, PIN_HIGH);
        rt_thread_mdelay(1000);
        rt_pin_write(LED_PIN_0, PIN_LOW);
        rt_thread_mdelay(1000);
    }
}
