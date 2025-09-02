/*
* Copyright (c) 2006-2025, RT-Thread Development Team
*
* SPDX-License-Identifier: Apache-2.0
*
* Change Logs:
* Date           Author        Notes
* 2025-06-24     kurisaW       first version
*/

#include <stdio.h>
#include "rpmsg_lite.h"
#include "rpmsg_queue.h"
#include "rpmsg_ns.h"

#include "hal_data.h"

#include "rpmsg_platform.h"

#include <rtthread.h>

#define LOCAL_EPT_ADDR             0x00

typedef struct the_message
{
    uint32_t DATA;
} THE_MESSAGE, *THE_MESSAGE_PTR;

static THE_MESSAGE msg = {0};

#define RPMSG_LITE_SHMEM_BASE 0x22174000

#define SH_MEM_TOTAL_SIZE (64 * 1024)

rt_mailbox_t my_rpmsg_mb;

static void app_nameservice_isr_cb(uint32_t new_ept, const char *new_ept_name, uint32_t flags, void *user_data)
{
    uint32_t *data = (uint32_t *)user_data;

    *data = new_ept;
}

void rpmsg_entry(void *param)
{
    volatile uint32_t remote_addr = 0U;
    struct rpmsg_lite_endpoint *my_ept;
    rpmsg_queue_handle my_queue;
    struct rpmsg_lite_instance *my_rpmsg;
    uint32_t len;

    my_rpmsg = rpmsg_lite_master_init((void *)RPMSG_LITE_SHMEM_BASE, SH_MEM_TOTAL_SIZE, RL_PLATFORM_RA8P1_M85_LINK_ID, RL_NO_FLAGS);

    my_queue  = rpmsg_queue_create(my_rpmsg);
    my_ept    = rpmsg_lite_create_ept(my_rpmsg, LOCAL_EPT_ADDR, rpmsg_queue_rx_cb, my_queue);

    rt_thread_mdelay(100);

    /* Send the first message to the remoteproc */
    msg.DATA = 0;
    (void)rpmsg_lite_send(my_rpmsg, my_ept, remote_addr, (char *)&msg, sizeof(THE_MESSAGE), RL_DONT_BLOCK);

    while (msg.DATA <= 100U)
    {
        (void)rpmsg_queue_recv(my_rpmsg, my_queue, (uint32_t *)&remote_addr, (char *)&msg, sizeof(THE_MESSAGE), &len,
                               RL_BLOCK);
        (void)rt_kprintf("Primary core received a msg\r\n");
        (void)rt_kprintf("Message: Size=%x, DATA = %i\r\n", len, msg.DATA);
        msg.DATA++;

        (void)rpmsg_lite_send(my_rpmsg, my_ept, remote_addr, (char *)&msg, sizeof(THE_MESSAGE), RL_BLOCK);
    }

    (void)rpmsg_lite_destroy_ept(my_rpmsg, my_ept);
    my_ept = ((void *)0);
    (void)rpmsg_queue_destroy(my_rpmsg, my_queue);
    my_queue = ((void *)0);
    (void)rpmsg_lite_deinit(my_rpmsg);

    /* Print the ending banner */
    (void)rt_kprintf("\r\nRPMsg demo ends\r\n");
}

int rp_ping_pong(void)
{
    rt_thread_t tid = RT_NULL;

    tid = rt_thread_create("rpmsg",
                           rpmsg_entry, RT_NULL,
                           1024,
                           25,
                           5);
    if (tid != RT_NULL)
        rt_thread_startup(tid);

    return 0;
}
MSH_CMD_EXPORT(rp_ping_pong, rp_ping_pong);
