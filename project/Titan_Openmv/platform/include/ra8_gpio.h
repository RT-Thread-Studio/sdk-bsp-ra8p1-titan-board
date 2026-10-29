/* SPDX-License-Identifier: Apache-2.0 */
#ifndef RA8_GPIO_H
#define RA8_GPIO_H
#include <rtthread.h>
rt_base_t ra8_gpio_pin_get(const char *name);
rt_bool_t ra8_gpio_is_ready(void);
rt_err_t ra8_gpio_configure(rt_base_t pin, rt_uint8_t mode, int value, rt_bool_t pullup);
#endif
