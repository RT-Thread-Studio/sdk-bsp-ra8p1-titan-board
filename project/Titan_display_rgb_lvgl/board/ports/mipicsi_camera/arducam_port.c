/*
 * Copyright (c) 2006-2025, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author        Notes
 * 2025-06-11     kurisaw       first version
 */

#include <arducam.h>
#include <camera_layer.h>
#include <rtdevice.h>
#include <rtthread.h>

//#define DRV_DEBUG
#define LOG_TAG             "camera.i2c"
#include <drv_log.h>

#define BSP_I2C_SLAVE_ADDR_CAMERA   (0x3C) /* Slave address for OV Camera Module */

/* define your i2c bus name HERE */
#define I2C_BUS_NAME        BSP_USING_MIPI_CSI_CAMERA_I2C
#define CAMERA_I2C_ADDR     BSP_I2C_SLAVE_ADDR_CAMERA

static struct rt_i2c_bus_device *i2c_bus = RT_NULL;

bool rdSensorReg16_Multi(uint16_t regID, uint8_t * regDat, uint32_t len);

/******************************************************************************
 * Initialization Function
 ******************************************************************************/
int camera_i2c_init(void)
{
    if (i2c_bus == RT_NULL)
    {
        i2c_bus = (struct rt_i2c_bus_device *)rt_device_find(I2C_BUS_NAME);
        if (i2c_bus == RT_NULL)
        {
            LOG_E("Failed to find I2C bus: %s", I2C_BUS_NAME);
            return -RT_ERROR;
        }
        LOG_I("I2C bus initialized: %s", I2C_BUS_NAME);
    }
    return RT_EOK;
}

/******************************************************************************
 * Common I2C Transfer Function
 ******************************************************************************/
static bool camera_i2c_transfer(struct rt_i2c_msg *msgs, rt_uint32_t num)
{
    if (i2c_bus == RT_NULL && camera_i2c_init() != RT_EOK)
    {
        return false;
    }

    if (rt_i2c_transfer(i2c_bus, msgs, num) != num)
    {
        LOG_D("I2C transfer failed");
        return false;
    }
    return true;
}

/******************************************************************************
 * Register Access Functions
 ******************************************************************************/
bool wrSensorReg8_8(int regID, int regDat)
{
    uint8_t data[2] = {(uint8_t)regID, (uint8_t)regDat};
    struct rt_i2c_msg msg =
    {
        .addr  = CAMERA_I2C_ADDR,
        .flags = RT_I2C_WR,
        .buf   = data,
        .len   = sizeof(data)
    };

    return camera_i2c_transfer(&msg, 1);
}

bool wrSensorReg16_8(int regID, int regDat)
{
    uint8_t data[3] = {(uint8_t)(regID >> 8), (uint8_t)regID, (uint8_t)regDat};
    struct rt_i2c_msg msg =
    {
        .addr  = CAMERA_I2C_ADDR,
        .flags = RT_I2C_WR,
        .buf   = data,
        .len   = sizeof(data)
    };

    return camera_i2c_transfer(&msg, 1);
}

bool rdSensorReg16_8(uint16_t regID, uint8_t *regDat)
{
    uint8_t reg_addr[2] = {(uint8_t)(regID >> 8), (uint8_t)regID};
    struct rt_i2c_msg msgs[2] =
    {
        {
            .addr  = CAMERA_I2C_ADDR,
            .flags = RT_I2C_WR | RT_I2C_NO_START,
            .buf   = reg_addr,
            .len   = sizeof(reg_addr)
        },
        {
            .addr  = CAMERA_I2C_ADDR,
            .flags = RT_I2C_RD,
            .buf   = regDat,
            .len   = 1
        }
    };

    return camera_i2c_transfer(msgs, 2);
}

bool rdSensorReg16_Multi(uint16_t regID, uint8_t *regDat, uint32_t len)
{
    uint8_t reg_addr[2] = {(uint8_t)(regID >> 8), (uint8_t)regID};
    struct rt_i2c_msg msgs[2] =
    {
        {
            .addr  = CAMERA_I2C_ADDR,
            .flags = RT_I2C_WR | RT_I2C_NO_START,
            .buf   = reg_addr,
            .len   = sizeof(reg_addr)
        },
        {
            .addr  = CAMERA_I2C_ADDR,
            .flags = RT_I2C_RD,
            .buf   = regDat,
            .len   = len
        }
    };

    return camera_i2c_transfer(msgs, 2);
}
