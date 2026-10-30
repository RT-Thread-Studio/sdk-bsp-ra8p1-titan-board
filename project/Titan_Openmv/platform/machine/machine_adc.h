/* SPDX-License-Identifier: MIT */
#ifndef TITAN_MACHINE_ADC_H
#define TITAN_MACHINE_ADC_H
#include <stdbool.h>
#include <stdint.h>
#include "py/obj.h"
#if MICROPY_PY_MACHINE_ADC
extern const mp_obj_type_t machine_adc_type;
void machine_adc_deinit_all(void);
bool ra8_adc_pin_owned(uint16_t pin);

#else
static inline void machine_adc_deinit_all(void) {  }
static inline bool ra8_adc_pin_owned(uint16_t pin) { (void)pin; return false; }
#endif
#endif
