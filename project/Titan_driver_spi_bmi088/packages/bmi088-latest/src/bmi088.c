/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author      Notes
 * 2020-02-28     MyGuo       the first version
 */
 
#include "bmi088.h"
#include <rtdbg.h> 
#include <rtdevice.h> 
#include <board.h>
#include "drv_spi.h"
#include <drv_gpio.h>

#define BMI088_SPI_MAX_SPEED (10 * 1000 * 1000) // M

#define CSB1_Pin     BSP_IO_PORT_06_PIN_12
#define CSB2_Pin     BSP_IO_PORT_07_PIN_04

static rt_err_t _bmi088_spi_read(struct rt_spi_device *dev, rt_uint8_t reg_addr, const rt_uint8_t len, rt_uint8_t *buf)
{
    reg_addr |= 0x80;
    
    dev->bus->owner = dev;
    rt_spi_send_then_recv(dev, &reg_addr, 1, buf, len);    
    
    return RT_EOK;
}

static rt_err_t _bmi088_spi_write(struct rt_spi_device *dev, rt_uint8_t reg_addr, const rt_uint8_t len, rt_uint8_t *buf)
{   
    reg_addr &= 0x7f;
    
    dev->bus->owner = dev;
    rt_spi_send_then_send(dev, &reg_addr, 1, buf, len);
    
    return RT_EOK;
}

static rt_err_t _bmi088_get_accel_raw(struct bmi08x_dev *dev, struct bmi088_3axes *accel)
{
    rt_uint8_t buffer[10];
    uint8_t lsb, msb;
    rt_err_t res;

    struct rt_spi_device *spi_dev = (struct rt_spi_device *)(dev->accel_bus);
    res = _bmi088_spi_read(spi_dev, ACC_X_LSB_REG, 10, buffer);
    if (res != RT_EOK)
    {
        return res;
    }
    lsb = buffer[1];
    msb = buffer[2];
    accel->x = (rt_int16_t)((msb << 8) | lsb); /* X */
    
    lsb = buffer[3];
    msb = buffer[4];
    accel->y = (rt_int16_t)((msb << 8) | lsb);/* Y */

    lsb = buffer[5];
    msb = buffer[6];
    accel->z = (rt_int16_t)((msb << 8) | lsb);/* Z */

    return RT_EOK;
}

static rt_err_t _bmi088_get_gyro_raw(struct bmi08x_dev *dev, struct bmi088_3axes *gyro)
{
    rt_uint8_t buffer[6];
    uint8_t lsb, msb;
    rt_err_t res;
    
    struct rt_spi_device *spi_dev = (struct rt_spi_device *)(dev->gyro_bus);
    res = _bmi088_spi_read(spi_dev, RATE_X_LSB_REG, 6, buffer);
    if (res != RT_EOK)
    {
        return res;
    }
    lsb = buffer[0];
    msb = buffer[1];
    gyro->x = (rt_int16_t)((msb * 256) + lsb); /* X */

    lsb = buffer[2];
    msb = buffer[3];
    gyro->y = (rt_int16_t)((msb * 256) + lsb); /* Y */

    lsb = buffer[4];
    msb = buffer[5];
    gyro->z = (rt_int16_t)((msb * 256) + lsb); /* Z */

    return RT_EOK;
}

/**
* This function gets the data of the accelerometer, unit: m/ss
 *
 * @param dev the pointer of device driver structure
 * @param accel the pointer of 3axes structure for receive data
 *
 * @return the reading number.
 */
rt_size_t bmi088_get_accel(struct bmi08x_dev *dev, struct bmi088_data *buf)
{ 
    struct bmi088_3axes tmp;
    
    _bmi088_get_accel_raw(dev, &tmp);
    buf->x = ((float)tmp.x) /32768.0f * 6 * 9.8f;
    buf->y = ((float)tmp.y) /32768.0f * 6 * 9.8f;
    buf->z = ((float)tmp.z) /32768.0f * 6 * 9.8f;

    return 1;// just support rw mode
}

/**
* This function gets the data of the gyroscope, unit: rad/s
 *
 * @param dev the pointer of device driver structure
 * @param gyro the pointer of 3axes structure for receive data
 *
 * @return the reading number.
 */
rt_size_t bmi088_get_gyro(struct bmi08x_dev *dev, struct bmi088_data *buf)
{
    struct bmi088_3axes tmp;
    
    _bmi088_get_gyro_raw(dev, &tmp);
    buf->x = (float)tmp.x / 32767.0f * 2000.0f;
    buf->y = (float)tmp.y / 32767.0f * 2000.0f;
    buf->z = (float)tmp.z / 32767.0f * 2000.0f;   
    
    return 1;
}

/**
 * This function software reset the accelerometer of bmi08x.
 *
 * @param dev the pointer of bmi08x driver structure
 *
 * @return the status of software reset, RT_EOK represents software reset successfully.
 */
static rt_err_t _bmi088a_soft_reset(struct bmi08x_dev *dev)
{
    uint8_t send_cmd = BMI08X_SOFT_RESET_CMD;
    struct rt_spi_device *spi_dev = (struct rt_spi_device *)(dev->accel_bus);
    if (_bmi088_spi_write(spi_dev, ACC_SOFTRESET_REG, 1, &send_cmd) == RT_EOK)
    {
        rt_thread_mdelay(BMI08X_ACCEL_SOFTRESET_DELAY_MS);
        return RT_EOK;
    }
    else
    {
        return RT_ERROR;    
    }
}

/**
 * This function software reset the gyroscope of bmi08x.
 *
 * @param dev the pointer of bmi08x driver structure
 *
 * @return the status of software reset, RT_EOK represents software reset successfully.
 */
static rt_err_t _bmi088g_soft_reset(struct bmi08x_dev *dev)
{
    uint8_t send_cmd = BMI08X_SOFT_RESET_CMD;
    struct rt_spi_device *spi_dev = (struct rt_spi_device *)(dev->gyro_bus);
    if (_bmi088_spi_write(spi_dev, GYRO_SOFTRESET_REG, 1, &send_cmd) == RT_EOK)
    {
        rt_thread_mdelay(BMI08X_GYRO_SOFTRESET_DELAY_MS);
        return RT_EOK;
    }
    else
    {
        return RT_ERROR;    
    }
}

/**
 * This function initialize the accelerometer of bmi08x.
 *
 * @param dev the pointer of bmi08x driver structure
 *
 * @return the status of initialization, RT_EOK represents initialize successfully.
 */
static rt_err_t _bmi088a_init(struct bmi08x_dev *dev)
{
	rt_err_t res = RT_EOK;
    uint8_t chip_acc_id[2] = {0};
    // config acc to spi mode
    rt_pin_write(dev->accel_id, PIN_LOW);
    rt_thread_mdelay(1);
    rt_pin_write(dev->accel_id, PIN_HIGH);
 
    struct rt_spi_device *spi_dev = (struct rt_spi_device *)(dev->accel_bus);
    _bmi088_spi_read(spi_dev, ACC_CHIP_ID_REG, 2, chip_acc_id);    /* Dummy read */
    if (chip_acc_id[1] != dev->accel_chip_id) 
    {
        LOG_E("Fail initialize acc");
        goto __exit;        
    }

    rt_thread_mdelay(10);
    res = _bmi088a_soft_reset(dev);
    
    // config acc to spi mode
    rt_pin_write(dev->accel_id, PIN_LOW);
    rt_thread_mdelay(1);
    rt_pin_write(dev->accel_id, PIN_HIGH);
    
    return res;

__exit:
    return RT_ERROR;    
}

/**
 * This function initialize the gyroscope of bmi08x.
 *
 * @param dev the pointer of bmi08x driver structure
 *
 * @return the status of initialization, RT_EOK represents initialize successfully.
 */
static rt_err_t _bmi088g_init(struct bmi08x_dev *dev)
{
	rt_err_t res = RT_EOK;
    rt_uint8_t id = 0;  
    
    struct rt_spi_device *spi_dev = (struct rt_spi_device *)dev->gyro_bus;
    _bmi088_spi_read(spi_dev, GYRO_CHIP_ID_REG, 1, &id);
    
    if (id != dev->gyro_chip_id) 
    {
        LOG_E("Fail initialize gyro");
        goto __exit;
    }
    rt_thread_mdelay(10);
    res = _bmi088g_soft_reset(dev);
    return res;
    
__exit:
    return RT_ERROR;
}

/**
 * This function set the power mode of accelerometer of bmi08x 
 *
 * @param dev the pointer of bmi08x driver structure
 *
 * @return the setting status, RT_EOK represents reading the data successfully.
 */
rt_err_t bmi088a_set_power_mode(struct bmi08x_dev *dev)
{
    uint8_t power_mode = dev->accel_cfg.power;
    uint8_t data[2];
    struct rt_spi_device *spi_dev = (struct rt_spi_device *)(dev->accel_bus);
    
    if (power_mode == BMI08X_ACCEL_PM_ACTIVE) 
    {
        data[0] = BMI08X_ACCEL_PWR_ACTIVE_CMD;
        data[1] = BMI08X_ACCEL_POWER_ENABLE_CMD;
    } 
    else if (power_mode == BMI08X_ACCEL_PM_SUSPEND) 
    {
        data[0] = BMI08X_ACCEL_PWR_SUSPEND_CMD;
        data[1] = BMI08X_ACCEL_POWER_DISABLE_CMD;
    } 
    else 
    {
        LOG_E("Invalid acc power mode!");
        goto __exit;          
    }

    if (_bmi088_spi_write(spi_dev, ACC_PWR_CONF_REG, 1, &data[0]) == RT_EOK)
    {
        rt_thread_mdelay(BMI08X_POWER_CONFIG_DELAY);
        data[1] = BMI08X_ACCEL_POWER_ENABLE_CMD;
        if (_bmi088_spi_write(spi_dev, ACC_PWR_CTRL_REG, 1, &data[1]) == RT_EOK)
        {
            rt_thread_mdelay(BMI08X_POWER_CONFIG_DELAY);
            return RT_EOK;
        }
        else
        {
            LOG_E("Failed write CTRL_REG");
            goto __exit;
        }
    }
    else
    {
        LOG_E("Failed write PWR_REG");
        goto __exit;
    }        
    
__exit:
    return RT_ERROR;
}

/**
 * This function set the power mode of gyroscope of bmi08x 
 *
 * @param dev the pointer of bmi08x driver structure
 *
 * @return the setting status, RT_EOK represents reading the data successfully.
 */
rt_err_t bmi088g_set_power_mode(struct bmi08x_dev *dev)
{
	uint8_t power_mode = dev->gyro_cfg.power;
    uint8_t read_data;
	uint8_t is_power_switching_mode_valid = 1;
    
    struct rt_spi_device *spi_dev = (struct rt_spi_device *)(dev->gyro_bus);
    _bmi088_spi_read(spi_dev, GYRO_LPM1_REG, 1, &read_data);
    
    if (power_mode == read_data) 
    {
        return RT_EOK;
    }
    else 
    {
        // only switching between normal mode and the suspend mode is allowed
        if ((power_mode == BMI08X_GYRO_PM_SUSPEND) && (read_data == BMI08X_GYRO_PM_DEEP_SUSPEND)) 
        {
            is_power_switching_mode_valid = 0;
        }  
        if ((power_mode == BMI08X_GYRO_PM_DEEP_SUSPEND) && (read_data == BMI08X_GYRO_PM_SUSPEND))
        {
            is_power_switching_mode_valid = 0;
        }
        
        if (is_power_switching_mode_valid) 
        {
            if (_bmi088_spi_write(spi_dev, GYRO_LPM1_REG, 1, &power_mode) == RT_EOK)
            {
                rt_thread_mdelay(BMI08X_GYRO_POWER_MODE_CONFIG_DELAY);
            }
        }
        else
        {
            LOG_E("Invalid gyro mode switch");
            goto __exit;        
        }
    
    }
    
__exit:
    return RT_ERROR;   
}

/**
 * This function set the bandwidth(bw), output data rate(odr) and range of accelerometer of bmi08x 
 *
 * @param dev the pointer of bmi08x driver structure
 *
 * @return the setting status, RT_EOK represents  reading the data successfully.
 */
rt_err_t bmi088a_set_meas_conf(struct bmi08x_dev *dev)
{
    uint8_t data[2] = {0};
    uint8_t reg_val[3] = {0};
    uint8_t bw = dev->accel_cfg.bw;
    uint8_t range = dev->accel_cfg.range;
    uint8_t odr = dev->accel_cfg.odr;
    uint8_t is_odr_invalid = 0, is_bw_invalid = 0, is_range_invalid = 0;
    
    if ((odr < BMI08X_ACCEL_ODR_12_5_HZ) || (odr > BMI08X_ACCEL_ODR_1600_HZ))
    {
        is_odr_invalid = 1;
    }
    if (bw > BMI08X_ACCEL_BW_NORMAL) 
    {
        is_bw_invalid = 1;
    }
    if (range > BMI088_ACCEL_RANGE_24G) 
    {
        is_range_invalid = 1;
    }
    
    if ((!is_odr_invalid) && (!is_bw_invalid) && (!is_range_invalid)) 
    {
        //dummy read
        struct rt_spi_device *spi_dev = (struct rt_spi_device *)(dev->accel_bus);
        if (_bmi088_spi_read(spi_dev, ACC_CONF_REG, 2, data) == RT_EOK)
        {
            data[0] = (1<<7) | (2<<4) | (0xB<<0);// bwp = normal, odr = 800
            _bmi088_spi_write(spi_dev, ACC_CONF_REG, 1, &data[0]);
            
            data[1] = 0x01;// range = 6G
            _bmi088_spi_write(spi_dev, ACC_RANGE_REG, 1, &data[1]);
            
            rt_thread_mdelay(10);
            _bmi088_spi_read(spi_dev, ACC_CONF_REG, 3, reg_val);// dummy read
            if ((reg_val[1] == 0xAB) && (reg_val[2] == 0x01)) 
            {
                return RT_EOK;
            }
        }
        
    }
    return RT_ERROR;
}

/**
 * This function set the bandwidth(bw), output data rate(odr) and range of gyroscope of bmi08x 
 *
 * @param dev the pointer of bmi08x driver structure
 *
 * @return the setting status, RT_EOK represents reading the data successfully.
 */
rt_err_t bmi088g_set_meas_conf(struct bmi08x_dev *dev)
{
    uint8_t data;
    uint8_t bw_odr = dev->gyro_cfg.bw, range = dev->gyro_cfg.range;
    uint8_t reg_val[2] = {0};
    uint8_t is_range_invalid = 0, is_odr_invalid = 0;

    if (bw_odr > BMI08X_GYRO_BW_32_ODR_100_HZ) 
    {
        is_odr_invalid = 1;
    }
    if (range > BMI08X_GYRO_RANGE_125_DPS) 
    {
        is_range_invalid = 1;
    } 
    if ((!is_odr_invalid) && (!is_range_invalid)) 
    {
//      data = BMI08X_SET_BITS_POS_0(data, BMI08X_GYRO_BW, odr);
        data = 0x01;// ODR = 2000Hz, Filter bandwidth = 230Hz
        struct rt_spi_device *spi_dev = (struct rt_spi_device *)(dev->gyro_bus);
        if (_bmi088_spi_write(spi_dev, GYRO_BANDWIDTH_REG, 1, &data) == RT_EOK)
        {
//          data = BMI08X_SET_BITS_POS_0(data, GYRO_RANGE_REG, range);
            data = 0x00;// range = 2000deg/s
            if (_bmi088_spi_write(spi_dev, GYRO_RANGE_REG, 1, &data) == RT_EOK) 
            {
                rt_thread_mdelay(10);
                _bmi088_spi_read(spi_dev, GYRO_RANGE_REG, 2, reg_val);
                if ((reg_val[0] == 0x00) && (reg_val[1] == 0x81))// 7 bit always 1
                {
                    return RT_EOK;
                }                
            }                
        }
    }
    return RT_ERROR;    
}

/**
 * This function initialize the bmi088 device.
 *
 * @param acc_spi_name the name of spi device(Accelerometer)
 * @param gyro_spi_name the name of spi device(Gyroscope)
 *
 * @return the pointer of bmi08x driver structure, RT_NULL represents initialization failed.
 */
struct bmi08x_dev *bmi088_init(const char *acc_spi_name, const char *gyro_spi_name)
{
    struct bmi08x_dev *dev = RT_NULL;
    rt_uint8_t res = RT_EOK;

    RT_ASSERT(acc_spi_name);
    RT_ASSERT(gyro_spi_name);

    dev = rt_calloc(1, sizeof(struct bmi08x_dev));
    if (dev == RT_NULL)
    {
        LOG_E("Can't allocate memory for bmi08x device on '%s' and '%s' ", acc_spi_name, gyro_spi_name);
        goto __exit;
    }
    
    dev->accel_bus = rt_device_find(acc_spi_name);
    dev->gyro_bus = rt_device_find(gyro_spi_name);
    
    
    if ((dev->accel_bus == RT_NULL) || (dev->gyro_bus == RT_NULL))
    {
        LOG_E("Can't find device:'%s' of '%s'", acc_spi_name, gyro_spi_name);
        goto __exit;
    }
    
    if (dev->accel_bus->type != dev->gyro_bus->type)
    {
        LOG_E("The bus type of '%s' and '%s' should same", acc_spi_name, gyro_spi_name);
        goto __exit;    
    }

    if (dev->accel_bus->type == RT_Device_Class_I2CBUS)
    {
        LOG_E("Bmi08x not support I2C temporarily");
        goto __exit;      
    }
    else if (dev->accel_bus->type == RT_Device_Class_SPIDevice)
    {
//#ifdef RT_USING_SPI
        struct rt_spi_configuration cfg;

        cfg.data_width = 8;
        cfg.mode = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB;
        cfg.max_hz = BMI088_SPI_MAX_SPEED; /* Set spi max speed */
        struct rt_spi_device *spi_dev = (struct rt_spi_device *)dev->accel_bus;
        spi_dev->bus->owner = spi_dev;
        rt_spi_configure(spi_dev, &cfg);
//#endif
    }
    else
    {
        LOG_E("Unsupported bus type:'%s'!", acc_spi_name);
        goto __exit;
    }
    
    // acc init
    {
        dev->accel_id = CSB1_Pin;
        dev->accel_chip_id = 0x1E;
        dev->accel_cfg.bw = BMI08X_ACCEL_BW_NORMAL;
        dev->accel_cfg.odr = BMI08X_ACCEL_ODR_800_HZ;
        dev->accel_cfg.power = BMI08X_ACCEL_PM_ACTIVE; 
        dev->accel_cfg.range = BMI088_ACCEL_RANGE_6G;
        res += _bmi088a_init(dev);
        res += bmi088a_set_power_mode(dev);
        res += bmi088a_set_meas_conf(dev);        
    }
    
    // gyro init
    {
        dev->gyro_id = CSB2_Pin;
        dev->gyro_chip_id = 0x0F;
        dev->gyro_cfg.bw = BMI08X_GYRO_BW_230_ODR_2000_HZ;
        dev->gyro_cfg.odr = BMI08X_GYRO_BW_230_ODR_2000_HZ;
        dev->gyro_cfg.power = BMI08X_GYRO_PM_NORMAL;
        dev->gyro_cfg.range = BMI08X_GYRO_RANGE_2000_DPS;
        res += _bmi088g_init(dev);
        res += bmi088g_set_power_mode(dev);
        res += bmi088g_set_meas_conf(dev);
    }
    
    rt_thread_mdelay(20);
    
    if (res == RT_EOK)
    {
        LOG_I("Device init succeed!");
    }
    else
    {
        goto __exit;
    }

    return dev;

__exit:
    if (dev != RT_NULL)
    {
        rt_free(dev);
    }
    return RT_NULL;

}

/**
 * This function releases memory
 *
 * @param dev the pointer of bmi08x driver structure
 */
void bmi088_deinit(struct bmi08x_dev *dev)
{
    RT_ASSERT(dev);

    rt_free(dev);
}


