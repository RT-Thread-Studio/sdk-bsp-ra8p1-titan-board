/* SPDX-License-Identifier: Apache-2.0 */
#ifndef RA8_PIN_IRQ_H
#define RA8_PIN_IRQ_H

#include <rtdevice.h>

int ra8_pin_irq_channel(rt_base_t pin);
rt_err_t ra8_pin_irq_attach(rt_base_t pin, rt_uint8_t mode, void (*handler)(void *), void *args);
rt_err_t ra8_pin_irq_enable(rt_base_t pin, rt_uint8_t enabled);
rt_err_t ra8_pin_irq_detach(rt_base_t pin);

#endif
