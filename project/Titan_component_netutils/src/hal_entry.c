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
#include <board.h>
#include "hal_data.h"

#include <fal.h>
#include <dfs_fs.h>

#define LOG_TAG         "netutils"
#include <drv_log.h>

#define LED_PIN_0    		BSP_IO_PORT_00_PIN_12 /* Onboard LED pins */
#define FS_PARTITION_NAME	"filesystem"

void hal_entry(void)
{
    rt_kprintf("\nHello RT-Thread!\n");
    rt_kprintf("===========================================================\n");
    rt_kprintf("This example project is a component netutils routine(hyperflash)!\n");
    rt_kprintf("===========================================================\n");

    struct rt_device *mtd_dev = RT_NULL;

    extern int fal_init(void);
	fal_init();

	mtd_dev = fal_mtd_nor_device_create(FS_PARTITION_NAME);
	if (!mtd_dev)
	{
		LOG_E("Can't create a mtd device on '%s' partition.", FS_PARTITION_NAME);
	}
	else
	{
		 if (dfs_mount(FS_PARTITION_NAME, "/", "lfs", 0, 0) == 0)
		 {
			 LOG_I("Filesystem initialized!");
		 }
		 else
		 {
			 dfs_mkfs("lfs", FS_PARTITION_NAME);
			 if (dfs_mount("filesystem", "/", "lfs", 0, 0) == 0)
			 {
				 LOG_I("Filesystem initialized!");
			 }
			 else
			 {
				 LOG_E("Failed to initialize filesystem!");
			 }
		 }
	}

    while (1)
    {
        rt_pin_write(LED_PIN_0, PIN_HIGH);
        rt_thread_mdelay(1000);
        rt_pin_write(LED_PIN_0, PIN_LOW);
        rt_thread_mdelay(1000);
    }
}
