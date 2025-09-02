/*
* Copyright (c) 2006-2025, RT-Thread Development Team
*
* SPDX-License-Identifier: Apache-2.0
*
* Change Logs:
* Date           Author        Notes
* 2025-06-24     kurisaW       first version
*/

#include <rtthread.h>
#include "rpmsg_comm.h"
#include "rpmsg_frame.h"

#define DBG_TAG "rpmsg.remote"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#define MAX_MESSAGE_COUNT 100
#define THREAD_STACK_SIZE 1024
#define THREAD_PRIORITY   25
#define THREAD_TICK       5

static rpmsg_comm_t rpmsg_comm;

static void rpmsg_app_entry(void *param)
{
   m85_to_m33_msg_t rx_msg;
   m33_to_m85_msg_t *tx_msg;
   rt_uint32_t msg_ptr, len;

   if (rpmsg_comm_remote_init(&rpmsg_comm) != RT_EOK)
   {
       LOG_E("Failed to initialize rpmsg communication!");
       return;
   }

   while (1)
   {
       while (rt_mb_recv(rpmsg_comm.tx_mb, &msg_ptr, RT_WAITING_NO) == RT_EOK)
       {
           tx_msg = (m33_to_m85_msg_t *)msg_ptr;
           if (rpmsg_comm_send(&rpmsg_comm, tx_msg, sizeof(m33_to_m85_msg_t)) == RT_EOK)
           {
               LOG_I("Sent message for index %u", tx_msg->index);
           }
           rt_free(tx_msg);
       }

       if (rpmsg_comm_recv(&rpmsg_comm, &rx_msg, sizeof(m85_to_m33_msg_t), &len) == RT_EOK)
       {
           if (rpmsg_frame_verify_checksum(&rx_msg, sizeof(m85_to_m33_msg_t), rx_msg.checksum) == RT_EOK)
           {
               rt_kprintf("===>>>\n");
               rpmsg_frame_print_m85_to_m33(&rx_msg, "RX");
               if (rx_msg.index >= MAX_MESSAGE_COUNT)
               {
                   break;
               }

               tx_msg = rt_malloc(sizeof(m33_to_m85_msg_t));
               if (tx_msg)
               {
                   tx_msg->index = rx_msg.index;
                   tx_msg->left_rear_rpm = 100.0f + tx_msg->index * 5.0f;
                   tx_msg->right_rear_rpm = 105.0f + tx_msg->index * 5.0f;
                   tx_msg->front_steering_angle = 5.0f * (tx_msg->index % 10);
                   tx_msg->checksum = rpmsg_frame_calc_checksum(tx_msg, sizeof(m33_to_m85_msg_t));

                   rpmsg_frame_print_m33_to_m85(tx_msg, "TX");

                   if (rt_mb_send(rpmsg_comm.tx_mb, (rt_uint32_t)tx_msg) != RT_EOK)
                   {
                       LOG_E("Failed to send message to tx mailbox!");
                       rt_free(tx_msg);
                   }
               }
               else
               {
                   LOG_E("Failed to allocate memory for tx message!");
               }
           }
           else
           {
               LOG_E("Checksum verification failed for index %u!", rx_msg.index);
           }
       }
   }

   rpmsg_comm_deinit(&rpmsg_comm);
   LOG_I("=====================================");
   LOG_I("           RPMsg demo ends");
   LOG_I("=====================================");
}

void fram_send()
{
   m85_to_m33_msg_t *tx_msg;

   tx_msg = rt_malloc(sizeof(m85_to_m33_msg_t));
   if (tx_msg)
   {
       tx_msg->index = 111;
       tx_msg->vehicle_speed = 15;
       tx_msg->steering_angle = 56;
       tx_msg->checksum = rpmsg_frame_calc_checksum(tx_msg, sizeof(m85_to_m33_msg_t));
       rpmsg_frame_print_m33_to_m85(tx_msg, "TX");

       if (rt_mb_send(rpmsg_comm.tx_mb, (rt_uint32_t)tx_msg) != RT_EOK)
       {
           LOG_E("Failed to send message to tx mailbox!");
           rt_free(tx_msg);
       }
   }
}
MSH_CMD_EXPORT(fram_send, fram_send);

int rp_remote(void)
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
