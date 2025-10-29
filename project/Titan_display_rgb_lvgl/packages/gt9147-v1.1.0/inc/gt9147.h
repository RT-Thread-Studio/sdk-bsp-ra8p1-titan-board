/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-01-13     RiceChen     the first version
 */

#ifndef __GT9147_H__
#define __GT9147_H__

#include "rtdevice.h"

#define GT9147_ADDR_LEN          2
#define GT9147_REGITER_LEN       2
#define GT9147_MAX_TOUCH         5
#define GT9147_POINT_INFO_NUM    5

#define GT9147_ADDRESS_HIGH      0x5D
#define GT9147_ADDRESS_LOW       0x14

#define GT9147_COMMAND_REG       0x8040
#define GT9147_CONFIG_REG        0x8047

#define GT9147_PRODUCT_ID        0x8140
#define GT9147_VENDOR_ID         0x814A
#define GT9147_READ_STATUS       0x814E

#define GT9147_POINT1_REG        0x814F
#define GT9147_POINT2_REG        0x8157
#define GT9147_POINT3_REG        0x815F
#define GT9147_POINT4_REG        0x8167
#define GT9147_POINT5_REG        0x816F

#define GT9147_CHECK_SUM         0x80FF

int rt_hw_gt911_init(const char *name, struct rt_touch_config *cfg);

#endif /* gt911.h */
