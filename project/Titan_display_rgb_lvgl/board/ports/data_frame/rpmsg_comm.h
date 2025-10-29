/*
 * Copyright (c) 2006-2025, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author        Notes
 * 2025-06-24     kurisaW       first version
 */

#ifndef __RPMSG_COMM_H__
#define __RPMSG_COMM_H__

#include <rtthread.h>
#include "rpmsg_lite.h"
#include "rpmsg_queue.h"
#include "rpmsg_ns.h"
#include "rpmsg_frame.h"

#define RPMSG_LOCAL_EPT_ADDR 0x00

#define RPMSG_SHMEM_BASE     0x22174000
#define RPMSG_SHMEM_SIZE     (64 * 1024)
#define RPMSG_TX_MB_SIZE     20 // Mailbox size for outgoing messages

typedef struct rpmsg_comm
{
    struct rpmsg_lite_instance *rpmsg_inst;
    struct rpmsg_lite_endpoint *ept;
    rpmsg_queue_handle queue;
    volatile rt_uint32_t remote_addr;
    rt_mailbox_t tx_mb;
    volatile rpmsg_ns_handle ns_handle;
} rpmsg_comm_t;

extern rpmsg_comm_t rpmsg_comm;

rt_err_t rpmsg_comm_master_init(rpmsg_comm_t *comm);
rt_err_t rpmsg_comm_remote_init(rpmsg_comm_t *comm);
void rpmsg_comm_deinit(rpmsg_comm_t *comm);
rt_err_t rpmsg_comm_send(rpmsg_comm_t *comm, void *msg, rt_size_t size);
rt_err_t rpmsg_comm_recv(rpmsg_comm_t *comm, void *msg, rt_size_t size, rt_uint32_t *len);
rt_err_t rpmsg_send_to_core0(int argc, char *argv[]);
rt_err_t rpmsg_send_to_core1(int argc, char *argv[]);

#endif /* __RPMSG_COMM_H__ */
