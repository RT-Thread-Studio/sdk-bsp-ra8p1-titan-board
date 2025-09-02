/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-03-06     MYGuo        the first version
 */

#include "sensor_intf_bmi088.h"
#include "bmi088.h"
#include <rtdbg.h>

static struct bmi08x_dev *bmi_dev;

static rt_err_t _bmi088_init(struct rt_sensor_intf *acc_intf, struct rt_sensor_intf *gyr_intf)
{
    bmi_dev = bmi088_init(acc_intf->dev_name, gyr_intf->dev_name);

    if (bmi_dev == RT_NULL)
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}

static rt_err_t _bmi088_set_power_mode(rt_sensor_t sensor, rt_uint8_t power)
{   
    if (sensor->info.type == RT_SENSOR_CLASS_ACCE)
    {
        if (power == RT_SENSOR_POWER_DOWN) 
        {
            bmi_dev->accel_cfg.power = BMI08X_ACCEL_PM_SUSPEND;
        }
        else if (power == RT_SENSOR_POWER_NORMAL)
        {
            bmi_dev->accel_cfg.power = BMI08X_ACCEL_PM_ACTIVE;
        }
        else 
        {
            LOG_E("Unsupported power mode %d", power);
            return -RT_ERROR;        
        }
        
        bmi088a_set_power_mode(bmi_dev);
    }
    else if (sensor->info.type == RT_SENSOR_CLASS_GYRO)
    {
        if (power == RT_SENSOR_POWER_DOWN) 
        {
            bmi_dev->gyro_cfg.power = BMI08X_GYRO_PM_SUSPEND;
        }
        else if (power == RT_SENSOR_POWER_NORMAL)
        {
            bmi_dev->gyro_cfg.power = BMI08X_GYRO_PM_NORMAL;
        }
        else if (power == RT_SENSOR_POWER_NONE)
        {
            bmi_dev->gyro_cfg.power = BMI08X_GYRO_PM_DEEP_SUSPEND;
        }
        else 
        {
            LOG_E("Unsupported power mode %d", power);
            return -RT_ERROR;        
        }
        
        bmi088g_set_power_mode(bmi_dev);
    }
    else 
    {
        LOG_E("Unsupported type %d", sensor->info.type);
        return -RT_ERROR;
    }
    return RT_EOK;
}

/**
* This function get the data of bmi088 sensor, unit: mg, mdps
 *
 * @param sensor the pointer of rt_sensor_device.
 * @param data the pointer of rt_sensor_data
 * 
 * @return the reading number.
 */
rt_size_t _bmi088_get_data(rt_sensor_t sensor, struct rt_sensor_data *data)
{
    rt_size_t len;
    if (sensor->info.type == RT_SENSOR_CLASS_ACCE)
    {
        struct bmi088_data acce_m_ss;
        len =  bmi088_get_accel(bmi_dev, &acce_m_ss);

        data->type = RT_SENSOR_CLASS_ACCE;
        data->data.acce.x = acce_m_ss.x * 1000;
        data->data.acce.y = acce_m_ss.y * 1000;
        data->data.acce.z = acce_m_ss.z * 1000;
        data->timestamp = rt_sensor_get_ts();
    }
    else if (sensor->info.type == RT_SENSOR_CLASS_GYRO)
    {
        struct bmi088_data gyro_rad_s;
        len = bmi088_get_gyro(bmi_dev, &gyro_rad_s);

        data->type = RT_SENSOR_CLASS_GYRO;
        data->data.gyro.x = gyro_rad_s.x * rad2deg * 1000;
        data->data.gyro.y = gyro_rad_s.y * rad2deg * 1000;
        data->data.gyro.z = gyro_rad_s.x * rad2deg * 1000;
        data->timestamp = rt_sensor_get_ts();
    }
    return len;
}

/**
* This function get the data of bmi088 sensor
 *
 * @param sensor the pointer of rt_sensor_device.
 * @param buf the pointer of data buffer.
 * @param len the length of data.
 * 
 * @return the reading number.
 */
static RT_SIZE_TYPE _bmi088_fetch_data(struct rt_sensor_device *sensor, void *buf, rt_size_t len)
{
    if (sensor->config.mode == RT_DEVICE_OFLAG_RDONLY)
    {
        return _bmi088_get_data(sensor, (struct rt_sensor_data *)buf);
    }
    else
    {
        return 0;
    }
}

/**
* This function control the bmi088 sensor
 *
 * @param sensor the pointer of rt_sensor_device.
 * @param cmd the type of command.
 * @param args the null pointer of commmand parameter, notice the pointer is four bytes.
 * 
 * @return the reading number.
 */
static rt_err_t _bmi088_control(struct rt_sensor_device *sensor, int cmd, void *args)//args��32λ(ָ�붼��4���ֽ�)
{
    rt_err_t result = RT_EOK;

    switch (cmd)
    {
    case RT_SENSOR_CTRL_GET_ID:
        if (sensor->info.type == RT_SENSOR_CLASS_ACCE) 
        {
            *(rt_uint8_t *)args = 0x1E;
        }
        else if (sensor->info.type == RT_SENSOR_CLASS_GYRO)
        {
            *(rt_uint8_t *)args = 0x0F;
        }
        break;
    case RT_SENSOR_CTRL_SET_ODR:
    case RT_SENSOR_CTRL_SET_RANGE:
        if (sensor->info.type == RT_SENSOR_CLASS_ACCE) 
        {
            result = bmi088a_set_meas_conf(bmi_dev);
        }
        else if (sensor->info.type == RT_SENSOR_CLASS_GYRO)
        {
            result = bmi088g_set_meas_conf(bmi_dev);
        }
        break;
    case RT_SENSOR_CTRL_SET_POWER:
        _bmi088_set_power_mode(sensor, (rt_uint32_t)args & 0xff);
        break;
    case RT_SENSOR_CTRL_SET_MODE:
        break;
    case RT_SENSOR_CTRL_SELF_TEST:
        /* TODO */
        result = -RT_EINVAL;
        break;
    default:
        return -RT_EINVAL;
    }
    return result;
}

static struct rt_sensor_ops sensor_ops =
{
    _bmi088_fetch_data, 
    _bmi088_control
};

/**
* This function initialize the bmi088
 *
 * @param name the name of bmi088, just first three characters will be used.
 * @param acc_cfg the pointer of configuration structure for accelarometer.
 * @param gyr_cfg the pointer of configuration structure for gyroscope.
 * 
 * @return the reading number.
 */
rt_err_t rt_hw_bmi088_init(const char *name, struct rt_sensor_config *acc_cfg, struct rt_sensor_config *gyr_cfg)
{   
    rt_int8_t result;
    rt_sensor_t sensor_acce = RT_NULL, sensor_gyro = RT_NULL;

//#ifdef PKG_USING_BMI088_ACCE
    /* accelerometer sensor register */
    {
        sensor_acce = rt_calloc(1, sizeof(struct rt_sensor_device));
        if (sensor_acce == RT_NULL)
        {
            return -1;
        }
            
        sensor_acce->info.type       = RT_SENSOR_CLASS_ACCE;
        sensor_acce->info.vendor     = RT_SENSOR_VENDOR_BOSCH;
        sensor_acce->info.model      = "bmi088_acc";
        sensor_acce->info.unit       = RT_SENSOR_UNIT_MG;
        sensor_acce->info.intf_type  = RT_SENSOR_INTF_SPI;
        sensor_acce->info.range_max  = 16000;
        sensor_acce->info.range_min  = 2000;
        sensor_acce->info.period_min = 5;

        rt_memcpy(&sensor_acce->config, acc_cfg, sizeof(struct rt_sensor_config));
        sensor_acce->ops = &sensor_ops;

        result = rt_hw_sensor_register(sensor_acce, name, RT_DEVICE_FLAG_RDWR, RT_NULL);
        if (result != RT_EOK)
        {
            LOG_E("device register err code: %d", result);
            goto __exit;
        }
    }
//#endif
//#ifdef PKG_USING_BMI088_GYRO
    /* gyroscope sensor register */
    {
        sensor_gyro = rt_calloc(1, sizeof(struct rt_sensor_device));
        if (sensor_gyro == RT_NULL)
        {
            goto __exit;
        }
            
        sensor_gyro->info.type       = RT_SENSOR_CLASS_GYRO;
        sensor_gyro->info.vendor     = RT_SENSOR_VENDOR_BOSCH;
        sensor_gyro->info.model      = "bmi088_gyro";
        sensor_gyro->info.unit       = RT_SENSOR_UNIT_MDPS;
        sensor_gyro->info.intf_type  = RT_SENSOR_INTF_SPI;
        sensor_gyro->info.range_max  = 2000000;
        sensor_gyro->info.range_min  = 250000;
        sensor_gyro->info.period_min = 5;

        rt_memcpy(&sensor_gyro->config, gyr_cfg, sizeof(struct rt_sensor_config));
        sensor_gyro->ops = &sensor_ops;

        result = rt_hw_sensor_register(sensor_gyro, name, RT_DEVICE_FLAG_RDWR, RT_NULL);
        if (result != RT_EOK)
        {
            LOG_E("device register err code: %d", result);
            goto __exit;
        }
    }
//#endif

    result = _bmi088_init(&acc_cfg->intf, &gyr_cfg->intf);
    if (result != RT_EOK)
    {
        LOG_E("_bmi088_init err code: %d", result);
        goto __exit;
    }

    LOG_I("sensor init success");
    return RT_EOK;

__exit:
    if (sensor_acce)
    {
        rt_free(sensor_acce);
    } 
    if (sensor_gyro)
    {
        rt_free(sensor_gyro);
    }  
    if (bmi_dev)
    {
        bmi088_deinit(bmi_dev);
    }
    return -RT_ERROR;
}
