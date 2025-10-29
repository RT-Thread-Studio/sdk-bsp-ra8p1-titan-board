/*
 * Copyright (c) 2006-2025, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author        Notes
 * 2025-06-24     kurisaW       first version
 */

#include "rpmsg_frame.h"

#define DBG_TAG "rpmsg.frame"
 #define DBG_LVL DBG_INFO
#include <rtdbg.h>

rt_uint32_t rpmsg_frame_calc_checksum(const void *msg, rt_size_t size)
{
    rt_uint32_t sum = 0;
    const rt_uint8_t *bytes = (const rt_uint8_t *)msg;
    for (rt_size_t i = 0; i < size - sizeof(rt_uint32_t); i++)
    {
        sum += bytes[i];
    }
    return sum;
}

rt_err_t rpmsg_frame_verify_checksum(const void *msg, rt_size_t size, rt_uint32_t received_checksum)
{
    rt_uint32_t expected = rpmsg_frame_calc_checksum(msg, size);
    return (expected == received_checksum) ? RT_EOK : RT_ERROR;
}

// 打印 M33 -> M85 消息
void rpmsg_frame_print_m33_to_m85(const m33_to_m85_msg_t *msg, const char *direction)
{
    LOG_I("[%s][index:%u][checksum:%u] M33 → M85 | LEFT:%.2f m/s | RIGHT:%.2f m/s | STEER:%.2f°",
          direction, msg->index, msg->checksum,
          msg->left_rear_vel,
          msg->right_rear_vel,
          msg->front_steering_angle);
}

// 打印 M85 -> M33 消息
void rpmsg_frame_print_m85_to_m33(const m85_to_m33_msg_t *msg, const char *direction)
{
    LOG_I("[%s][index:%u][checksum:%u] M85 → M33 | SPEED:%.2f m/s | STEER:%.2f°",
          direction, msg->index, msg->checksum,
          msg->vehicle_speed,
          msg->steering_angle);
}
