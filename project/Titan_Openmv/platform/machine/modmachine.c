/* SPDX-License-Identifier: Apache-2.0 */
#include <rtthread.h>
#include "py/runtime.h"
#include "py/mphal.h"
#include "extmod/modmachine.h"
#include "machine_timer.h"
extern const mp_obj_type_t machine_pin_type;
extern const mp_obj_type_t board_led_type;

static mp_obj_t machine_freq(void) { return mp_obj_new_int_from_uint(SystemCoreClock); }
static MP_DEFINE_CONST_FUN_OBJ_0(ra8_machine_freq_obj, machine_freq);
static mp_obj_t machine_uid(void) { return mp_obj_new_bytes((const uint8_t *)R_BSP_UniqueIdGet(), 16); }
static MP_DEFINE_CONST_FUN_OBJ_0(machine_uid_obj, machine_uid);
static mp_obj_t machine_reset(void) { NVIC_SystemReset(); return mp_const_none; }
static MP_DEFINE_CONST_FUN_OBJ_0(ra8_machine_reset_obj, machine_reset);
static mp_obj_t machine_idle(void) { mp_hal_poll(); rt_thread_mdelay(1); return mp_const_none; }
static MP_DEFINE_CONST_FUN_OBJ_0(machine_idle_obj, machine_idle);
static mp_obj_t machine_disable_irq(void) { return mp_obj_new_int(rt_hw_interrupt_disable()); }
static MP_DEFINE_CONST_FUN_OBJ_0(ra8_machine_disable_irq_obj, machine_disable_irq);
static mp_obj_t machine_enable_irq(size_t n_args, const mp_obj_t *args) {
    rt_hw_interrupt_enable(n_args ? mp_obj_get_int(args[0]) : 0);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(ra8_machine_enable_irq_obj, 0, 1, machine_enable_irq);
static const mp_rom_map_elem_t machine_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_machine) },
    { MP_ROM_QSTR(MP_QSTR_Pin), MP_ROM_PTR(&machine_pin_type) },
    { MP_ROM_QSTR(MP_QSTR_LED), MP_ROM_PTR(&board_led_type) },
    { MP_ROM_QSTR(MP_QSTR_Signal), MP_ROM_PTR(&machine_signal_type) },
#if MICROPY_PY_MACHINE_UART
    { MP_ROM_QSTR(MP_QSTR_UART), MP_ROM_PTR(&machine_uart_type) },
#endif
#if MICROPY_PY_MACHINE_PWM
    { MP_ROM_QSTR(MP_QSTR_PWM), MP_ROM_PTR(&machine_pwm_type) },
#endif
#if MICROPY_PY_MACHINE_I2C
    { MP_ROM_QSTR(MP_QSTR_I2C), MP_ROM_PTR(&machine_i2c_type) },
#endif
#if MICROPY_PY_MACHINE_SOFTI2C
    { MP_ROM_QSTR(MP_QSTR_SoftI2C), MP_ROM_PTR(&mp_machine_soft_i2c_type) },
#endif
#if MICROPY_PY_MACHINE_SPI
    { MP_ROM_QSTR(MP_QSTR_SPI), MP_ROM_PTR(&machine_spi_type) },
#endif
#if MICROPY_PY_MACHINE_SOFTSPI
    { MP_ROM_QSTR(MP_QSTR_SoftSPI), MP_ROM_PTR(&mp_machine_soft_spi_type) },
#endif
#if MICROPY_PY_MACHINE_RTC
    { MP_ROM_QSTR(MP_QSTR_RTC), MP_ROM_PTR(&machine_rtc_type) },
#endif
#if MICROPY_PY_MACHINE_TIMER
    { MP_ROM_QSTR(MP_QSTR_Timer), MP_ROM_PTR(&machine_timer_type) },
#endif
    #if MICROPY_PY_MACHINE_ADC
    { MP_ROM_QSTR(MP_QSTR_ADC), MP_ROM_PTR(&machine_adc_type) },
    #endif
    #if MICROPY_PY_MACHINE_WDT
    { MP_ROM_QSTR(MP_QSTR_WDT), MP_ROM_PTR(&machine_wdt_type) },
    #endif
    { MP_ROM_QSTR(MP_QSTR_mem8), MP_ROM_PTR(&machine_mem8_obj) },
    { MP_ROM_QSTR(MP_QSTR_mem16), MP_ROM_PTR(&machine_mem16_obj) },
    { MP_ROM_QSTR(MP_QSTR_mem32), MP_ROM_PTR(&machine_mem32_obj) },
    { MP_ROM_QSTR(MP_QSTR_freq), MP_ROM_PTR(&ra8_machine_freq_obj) },
    { MP_ROM_QSTR(MP_QSTR_unique_id), MP_ROM_PTR(&machine_uid_obj) },
    { MP_ROM_QSTR(MP_QSTR_reset), MP_ROM_PTR(&ra8_machine_reset_obj) },
    { MP_ROM_QSTR(MP_QSTR_idle), MP_ROM_PTR(&machine_idle_obj) },
    { MP_ROM_QSTR(MP_QSTR_disable_irq), MP_ROM_PTR(&ra8_machine_disable_irq_obj) },
    { MP_ROM_QSTR(MP_QSTR_enable_irq), MP_ROM_PTR(&ra8_machine_enable_irq_obj) },
    { MP_ROM_QSTR(MP_QSTR_time_pulse_us), MP_ROM_PTR(&machine_time_pulse_us_obj) },
};
static MP_DEFINE_CONST_DICT(machine_globals, machine_globals_table);
const mp_obj_module_t mp_module_machine = {{&mp_type_module}, (mp_obj_dict_t *)&machine_globals};
MP_REGISTER_MODULE(MP_QSTR_machine, mp_module_machine);
