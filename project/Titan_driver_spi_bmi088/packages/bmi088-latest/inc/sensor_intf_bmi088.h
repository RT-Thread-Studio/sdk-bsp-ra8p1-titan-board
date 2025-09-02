/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-03-06     MYGuo        the first version
 */

#ifndef __SENSOR_INTF_BMI088_H__
#define __SENSOR_INTF_BMI088_H__

#include <rtthread.h>
#include <rtdevice.h>
#include "bmi088.h"

#if defined(RT_VERSION_CHECK)
    #if (RTTHREAD_VERSION >= RT_VERSION_CHECK(5, 0, 2))
        #define RT_SIZE_TYPE   rt_ssize_t
    #else
        #define RT_SIZE_TYPE   rt_size_t
    #endif
#endif

rt_err_t rt_hw_bmi088_init(const char *name, struct rt_sensor_config *acc_cfg, struct rt_sensor_config *gyr_cfg);
rt_size_t _bmi088_get_data(rt_sensor_t sensor, struct rt_sensor_data *data);

#endif
