/*
 * Copyright (c) 2006-2025, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author        Notes
 * 2025-07-02     kurisaw       Support real-time monitoring, with send actions determined by the user
 */

#include <rtthread.h>
#include <drv_log.h>
#include "rpmsg_comm.h"
#include "rpmsg_frame.h"

#if defined(SOC_SERIES_R7KA8P1_CORE1)

#define DRV_DEBUG
#define LOG_TAG "rpmsg_remote"

#define MAX_MESSAGE_COUNT 100
#define THREAD_STACK_SIZE 1024
#define THREAD_PRIORITY   24
#define THREAD_TICK       5

rpmsg_comm_t rpmsg_comm;

static void rpmsg_app_entry()
{
    m85_to_m33_msg_t rx_msg;
    m33_to_m85_msg_t *tx_msg;
    rt_uint32_t msg_ptr, len;

    if (rpmsg_comm_remote_init(&rpmsg_comm) != RT_EOK)
    {
        LOG_E("Failed to initialize rpmsg communication!");
        return;
    }

    LOG_I("====================================================");
    LOG_I("Start listening for data from core0...");
    LOG_I("====================================================");

    while (1)
    {
        if (rpmsg_comm_recv(&rpmsg_comm, &rx_msg, sizeof(m85_to_m33_msg_t), &len) == RT_EOK)
        {
            if (rpmsg_frame_verify_checksum(&rx_msg, sizeof(m85_to_m33_msg_t), rx_msg.checksum) == RT_EOK)
            {
                rpmsg_frame_print_m85_to_m33(&rx_msg, "RX");
            }
            else
            {
                LOG_E("Checksum verification failed for index %u!", rx_msg.index);
            }
        }

        while (rt_mb_recv(rpmsg_comm.tx_mb, &msg_ptr, RT_WAITING_NO) == RT_EOK)
        {
            tx_msg = (m33_to_m85_msg_t *)msg_ptr;
            rpmsg_comm_send(&rpmsg_comm, tx_msg, sizeof(m33_to_m85_msg_t));
            rt_free(tx_msg);
        }
    }

    rpmsg_comm_deinit(&rpmsg_comm);
    LOG_I("RPMsg demo ends");
}

int rp_remote()
{
    rt_thread_t tid = rt_thread_create("rpmsg_app", rpmsg_app_entry, RT_NULL,
                                       THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TICK);
    if (tid)
    {
        rt_thread_startup(tid);
    }
    else
    {
        LOG_E("Failed to create rpmsg app thread!");
    }
    return 0;
}
MSH_CMD_EXPORT(rp_remote, rpmsg frame for remote);

#endif /* (SOC_SERIES_R7KA8P1_CORE1) */
