/*
* Copyright (c) 2006-2025, RT-Thread Development Team
*
* SPDX-License-Identifier: Apache-2.0
*
* Change Logs:
* Date           Author        Notes
* 2025-06-13     kurisaW       first version
*/

#include "ist8310.h"

#define DBG_TAG "ist8310"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

static rt_err_t write_reg(struct rt_i2c_bus_device *bus, rt_uint8_t reg, rt_uint8_t value)
{
    rt_uint8_t buf[2] = {reg, value};

    if (rt_i2c_master_send(bus, IST8310_I2C_ADDR, 0, buf, 2) == 2) {
        return RT_EOK;
    }

    LOG_E("I2C write failed");
    return -RT_ERROR;
}

static rt_err_t read_regs(struct rt_i2c_bus_device *bus, rt_uint8_t reg, rt_uint8_t len, rt_uint8_t *buf)
{
    struct rt_i2c_msg msgs[2];

    /* 先发送寄存器地址 */
    msgs[0].addr  = IST8310_I2C_ADDR;
    msgs[0].flags = RT_I2C_WR;
    msgs[0].buf   = &reg;
    msgs[0].len   = 1;

    /* 然后读取数据 */
    msgs[1].addr  = IST8310_I2C_ADDR;
    msgs[1].flags = RT_I2C_RD;
    msgs[1].buf   = buf;
    msgs[1].len   = len;

    if (rt_i2c_transfer(bus, msgs, 2) == 2) {
        return RT_EOK;
    }

    LOG_E("I2C read failed");
    return -RT_ERROR;
}

static rt_err_t ist8310_soft_reset(ist8310_device_t dev)
{
    rt_err_t ret;
    rt_uint8_t val;

    /* 发送复位命令 */
    ret = write_reg(dev->i2c, IST8310_REG_CNTL2, 0x01);
    if (ret != RT_EOK) {
        return ret;
    }

    /* 等待复位完成 */
    for (int i = 0; i < 10; i++) {
        rt_thread_mdelay(10);

        /* 检查设备ID */
        ret = read_regs(dev->i2c, IST8310_REG_WHO_AM_I, 1, &val);
        if (ret != RT_EOK) {
            continue;
        }

        if (val == IST8310_DEVICE_ID) {
            /* 检查复位位是否清除 */
            ret = read_regs(dev->i2c, IST8310_REG_CNTL2, 1, &val);
            if (ret == RT_EOK && (val & 0x01) == 0) {
                return RT_EOK;
            }
        }
    }

    LOG_E("Reset timeout");
    return -RT_ETIMEOUT;
}

static rt_err_t ist8310_read_raw_data(ist8310_device_t dev, ist8310_data_t *data)
{
    rt_uint8_t buf[6];
    rt_err_t ret;

    /* 启动测量 */
    ret = write_reg(dev->i2c, IST8310_REG_CNTL1, 0x01);
    if (ret != RT_EOK) {
        return ret;
    }

    /* 读取数据 */
    ret = read_regs(dev->i2c, IST8310_REG_DATA_START, 6, buf);
    if (ret != RT_EOK) {
        return ret;
    }

    /* 转换数据 */
    data->x = (float)((int16_t)((buf[0] << 8) | buf[1]));
    data->y = (float)((int16_t)((buf[2] << 8) | buf[3]));
    data->z = (float)((int16_t)((buf[4] << 8) | buf[5]));

    return RT_EOK;
}

#ifdef IST8310_USING_SOFT_FILTER
static void ist8310_filter_entry(void *param)
{
    ist8310_device_t dev = (ist8310_device_t)param;
    ist8310_data_t raw_data;

    while (1) {
        rt_mutex_take(dev->lock, RT_WAITING_FOREVER);

        /* 读取原始数据 */
        if (ist8310_read_raw_data(dev, &raw_data) == RT_EOK) {
            /* 更新X轴滤波数据 */
            dev->mag_filter[0].buf[dev->mag_filter[0].index] = raw_data.x;
            dev->mag_filter[1].buf[dev->mag_filter[1].index] = raw_data.y;
            dev->mag_filter[2].buf[dev->mag_filter[2].index] = raw_data.z;

            /* 更新索引 */
            for (int i = 0; i < 3; i++) {
                if (++dev->mag_filter[i].index >= IST8310_AVERAGE_TIMES) {
                    dev->mag_filter[i].index = 0;
                    dev->mag_filter[i].is_full = RT_TRUE;
                }
            }
        }

        rt_mutex_release(dev->lock);
        rt_thread_mdelay(dev->period);
    }
}

static void ist8310_filter_average(ist8310_device_t dev, ist8310_data_t *data)
{
    rt_mutex_take(dev->lock, RT_WAITING_FOREVER);

    for (int i = 0; i < 3; i++) {
        float sum = 0;
        rt_size_t count = dev->mag_filter[i].is_full ? IST8310_AVERAGE_TIMES : dev->mag_filter[i].index;

        if (count == 0) {
            data->x = data->y = data->z = 0;
            rt_mutex_release(dev->lock);
            return;
        }

        for (rt_size_t j = 0; j < count; j++) {
            sum += dev->mag_filter[i].buf[j];
        }

        switch (i) {
        case 0: data->x = sum / count; break;
        case 1: data->y = sum / count; break;
        case 2: data->z = sum / count; break;
        }
    }

    rt_mutex_release(dev->lock);
}
#endif /* IST8310_USING_SOFT_FILTER */

ist8310_device_t ist8310_init(const char *i2c_bus_name)
{
    ist8310_device_t dev;

    RT_ASSERT(i2c_bus_name);

    dev = rt_calloc(1, sizeof(struct ist8310_device));
    if (dev == RT_NULL) {
        LOG_E("No memory for ist8310 device");
        return RT_NULL;
    }

    /* 查找I2C总线 */
    dev->i2c = rt_i2c_bus_device_find(i2c_bus_name);
    if (dev->i2c == RT_NULL) {
        LOG_E("Can't find I2C bus %s", i2c_bus_name);
        rt_free(dev);
        return RT_NULL;
    }

    /* 创建互斥锁 */
    dev->lock = rt_mutex_create("ist8310", RT_IPC_FLAG_FIFO);
    if (dev->lock == RT_NULL) {
        LOG_E("Can't create mutex for ist8310");
        rt_free(dev);
        return RT_NULL;
    }

    /* 初始化默认配置 */
    dev->flip_x_y = RT_FALSE;
    dev->declination_offset = 0.0f;

#ifdef IST8310_USING_SOFT_FILTER
    /* 初始化滤波 */
    for (int i = 0; i < 3; i++) {
        dev->mag_filter[i].index = 0;
        dev->mag_filter[i].is_full = RT_FALSE;
    }

    dev->period = IST8310_SAMPLE_PERIOD;

    /* 创建滤波线程 */
    dev->thread = rt_thread_create("ist8310", ist8310_filter_entry, dev,
                                 1024, 15, 10);
    if (dev->thread == RT_NULL) {
        LOG_E("Can't create filter thread");
        rt_mutex_delete(dev->lock);
        rt_free(dev);
        return RT_NULL;
    }
#endif /* IST8310_USING_SOFT_FILTER */

    /* 复位设备 */
    if (ist8310_soft_reset(dev) != RT_EOK) {
        LOG_E("Reset failed");
#ifdef IST8310_USING_SOFT_FILTER
        rt_thread_delete(dev->thread);
#endif
        rt_mutex_delete(dev->lock);
        rt_free(dev);
        return RT_NULL;
    }

#ifdef IST8310_USING_SOFT_FILTER
    rt_thread_startup(dev->thread);
#endif

    return dev;
}

void ist8310_deinit(ist8310_device_t dev)
{
    if (dev == RT_NULL) return;

#ifdef IST8310_USING_SOFT_FILTER
    if (dev->thread) {
        rt_thread_delete(dev->thread);
    }
#endif

    if (dev->lock) {
        rt_mutex_delete(dev->lock);
    }

    rt_free(dev);
}

rt_err_t ist8310_read_magnetometer(ist8310_device_t dev, ist8310_data_t *data)
{
    RT_ASSERT(dev);
    RT_ASSERT(data);

#ifdef IST8310_USING_SOFT_FILTER
    ist8310_filter_average(dev, data);
#else
    rt_mutex_take(dev->lock, RT_WAITING_FOREVER);
    rt_err_t ret = ist8310_read_raw_data(dev, data);
    rt_mutex_release(dev->lock);

    if (ret != RT_EOK) {
        return ret;
    }
#endif

    /* 处理XY翻转 */
    if (dev->flip_x_y) {
        float temp = data->x;
        data->x = data->y;
        data->y = temp;
    }

    return RT_EOK;
}

float ist8310_read_heading(ist8310_device_t dev)
{
    ist8310_data_t data;
    float heading;

    if (ist8310_read_magnetometer(dev, &data) != RT_EOK) {
        return 0.0f;
    }

    /* 计算航向角 */
    heading = atan2f(-data.x, data.y);  /* 弧度值 */
    heading += dev->declination_offset; /* 磁偏角修正 */

    /* 规范化到0-2π范围 */
    if (heading < 0) heading += 2 * PI;
    if (heading > 2 * PI) heading -= 2 * PI;

    /* 转换为角度值 */
    return heading * 180.0f / PI;
}

rt_err_t ist8310_set_flip_xy(ist8310_device_t dev, rt_bool_t flip)
{
    RT_ASSERT(dev);

    rt_mutex_take(dev->lock, RT_WAITING_FOREVER);
    dev->flip_x_y = flip;
    rt_mutex_release(dev->lock);

    return RT_EOK;
}

rt_err_t ist8310_set_declination(ist8310_device_t dev, float declination)
{
    RT_ASSERT(dev);

    rt_mutex_take(dev->lock, RT_WAITING_FOREVER);
    dev->declination_offset = declination;
    rt_mutex_release(dev->lock);

    return RT_EOK;
}
