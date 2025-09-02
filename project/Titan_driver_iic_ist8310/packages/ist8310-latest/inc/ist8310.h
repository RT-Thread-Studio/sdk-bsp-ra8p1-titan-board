/*
* Copyright (c) 2006-2025, RT-Thread Development Team
*
* SPDX-License-Identifier: Apache-2.0
*
* Change Logs:
* Date           Author        Notes
* 2025-06-13     kurisaW       first version
*/

#ifndef __IST8310_H__
#define __IST8310_H__

#include <rtthread.h>
#include <rtdevice.h>
#include <rtdbg.h>

#define IST8310_I2C_ADDR         0x0E
#define IST8310_DEVICE_ID        0x10

/* Register addresses */
#define IST8310_REG_WHO_AM_I     0x00
#define IST8310_REG_CNTL1        0x0A
#define IST8310_REG_CNTL2        0x0B
#define IST8310_REG_DATA_START   0x03

#ifdef IST8310_USING_SOFT_FILTER

typedef struct {
    float buf[IST8310_AVERAGE_TIMES];
    rt_off_t index;
    rt_bool_t is_full;
} ist8310_filter_t;
#endif

#define PI      3.14159265358979323846

typedef struct {
    float x;
    float y;
    float z;
} ist8310_data_t;

struct ist8310_device {
    struct rt_i2c_bus_device *i2c;
    rt_mutex_t lock;

#ifdef IST8310_USING_SOFT_FILTER
    ist8310_filter_t mag_filter[3];  /* for x, y, z */
    rt_thread_t thread;
    rt_uint32_t period;
#endif

    rt_bool_t flip_x_y;
    float declination_offset;
};

typedef struct ist8310_device *ist8310_device_t;

/* Device initialization and deinitialization */
ist8310_device_t ist8310_init(const char *i2c_bus_name);
void ist8310_deinit(ist8310_device_t dev);

/* Data reading interface */
rt_err_t ist8310_read_magnetometer(ist8310_device_t dev, ist8310_data_t *data);
float ist8310_read_heading(ist8310_device_t dev);

/* Configuration interface */
rt_err_t ist8310_set_flip_xy(ist8310_device_t dev, rt_bool_t flip);
rt_err_t ist8310_set_declination(ist8310_device_t dev, float declination);

#endif /* __IST8310_H__ */
