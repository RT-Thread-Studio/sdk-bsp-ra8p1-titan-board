/*
 * Copyright (c) 2006-2025, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author        Notes
 * 2025-06-24     kurisaW       first version
 */

#ifndef __RPMSG_FRAME_H__
#define __RPMSG_FRAME_H__

#include <rtthread.h>

// M33 -> M85 消息结构
typedef struct m33_to_m85_msg
{
    rt_uint32_t index;               // 消息索引
    float left_rear_vel;             // 左后轮转速 (m/s)
    float right_rear_vel;            // 右后轮转速 (m/s)
    float front_steering_angle;      // 前轮转向角 (度)
    rt_uint32_t checksum;            // 校验和
} m33_to_m85_msg_t;

// M85 -> M33 消息结构
typedef struct m85_to_m33_msg
{
    rt_uint32_t index;               // 消息索引
    float vehicle_speed;             // 车速 (m/s)
    float steering_angle;            // 转向角 (度)
    rt_uint32_t checksum;            // 校验和
} m85_to_m33_msg_t;

rt_uint32_t rpmsg_frame_calc_checksum(const void *msg, rt_size_t size);
rt_err_t rpmsg_frame_verify_checksum(const void *msg, rt_size_t size, rt_uint32_t received_checksum);
void rpmsg_frame_print_m33_to_m85(const m33_to_m85_msg_t *msg, const char *direction);
void rpmsg_frame_print_m85_to_m33(const m85_to_m33_msg_t *msg, const char *direction);

#endif /* __RPMSG_FRAME_H__ */
