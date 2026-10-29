#ifndef RA8_OPENMV_PORT_CONFIG_H
#define RA8_OPENMV_PORT_CONFIG_H
#include <stdint.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <hal_data.h>
typedef const struct ra8_pin *omv_gpio_t;
struct ra8_pin { rt_base_t pin; };
typedef struct rt_i2c_bus_device *omv_i2c_dev_t;
#define OMV_GPIO_MODE_INPUT PIN_MODE_INPUT
#define OMV_GPIO_MODE_OUTPUT PIN_MODE_OUTPUT
#define OMV_GPIO_MODE_ALT (3)
#define OMV_GPIO_PULL_NONE (0)
#define OMV_GPIO_PULL_UP (1)
#define OMV_GPIO_PULL_DOWN (2)
#define OMV_GPIO_SPEED_LOW (0)
#define OMV_GPIO_SPEED_HIGH (1)
#define OMV_GPIO_SPEED_MAX (2)
#define OMV_I2C_PORT_BITS uint8_t pending[8]; uint8_t pending_len;
#endif
