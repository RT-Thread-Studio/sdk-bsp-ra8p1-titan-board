/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Armink (armink.ztl@gmail.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <drivers/pin.h>
#include "py/runtime.h"
#include "py/gc.h"
#include "py/mphal.h"
#include "py/mperrno.h"
#include "py/stream.h"
#include "modmachine.h"
#include "extmod/virtpin.h"
#include "ra8_gpio.h"
#include "ra8_pin_irq.h"
#include "titan_lcd.h"

#if MICROPY_PY_MACHINE_PIN
#include <rtthread.h>

#define GPIO_MODE_IN                           ((uint32_t)0x00000000)   /*!< Input Floating Mode                   */
#define GPIO_MODE_OUT_PP                       ((uint32_t)0x00000001)   /*!< Output Push Pull Mode                 */
#define GPIO_MODE_OUT_OD                       ((uint32_t)0x00000011)   /*!< Output Open Drain Mode                */
#define GPIO_MODE_AF_PP                        ((uint32_t)0x00000002)   /*!< Alternate Function Push Pull Mode     */
#define GPIO_MODE_AF_OD                        ((uint32_t)0x00000012)   /*!< Alternate Function Open Drain Mode    */
#define GPIO_MODE_ANALOG                       ((uint32_t)0x00000003)   /*!< Analog Mode  */
#define GPIO_NOPULL                            ((uint32_t)0x00000000)   /*!< No Pull-up or Pull-down activation  */
#define GPIO_PULLUP                            ((uint32_t)0x00000001)   /*!< Pull-up activation                  */
#define GPIO_PULLDOWN                          ((uint32_t)0x00000002)   /*!< Pull-down activation                */
#define GPIO_MODE_IT_RISING                    ((uint32_t)0x10110000)   /*!< External Interrupt Mode with Rising edge trigger detection          */
#define GPIO_MODE_IT_FALLING                   ((uint32_t)0x10210000)   /*!< External Interrupt Mode with Falling edge trigger detection         */
#define GPIO_MODE_IT_RISING_FALLING            ((uint32_t)0x10310000)   /*!< External Interrupt Mode with Rising/Falling edge trigger detection  */
#define MACHINE_PIN_IRQ_RISING                 (1)
#define MACHINE_PIN_IRQ_FALLING                (2)

const mp_obj_base_t machine_pin_obj_template = {&machine_pin_type};
MP_REGISTER_ROOT_POINTER(mp_obj_t ra8_pin_irqs[32]);

bool ra8_machine_pin_irq_owned(uint16_t pin) {
    for (unsigned i = 0; i < 32; ++i) {
        mp_obj_t owner = MP_STATE_VM(ra8_pin_irqs)[i];
        if (owner != MP_OBJ_NULL && ((machine_pin_obj_t *)MP_OBJ_TO_PTR(owner))->pin == pin) {
            return true;
        }
    }
    return false;
}

void ra8_machine_pin_require_available(uint16_t pin) {
    if (titan_lcd_pin_owned(pin)) { mp_raise_OSError(MP_EBUSY); }
    /* Permanent Titan Board resources: debug, USB, I2C camera, SCI8 console,
     * GPT6 XCLK and OSPI1 CS0 HyperRAM (including its GPIO reset). */
    switch (pin) {
        case 0x010b: case 0x0208: case 0x0209: case 0x020a: case 0x020b:
        case 0x0407: case 0x0408: case 0x0409: case 0x040a: case 0x040b: case 0x040d:
        case 0x0500: case 0x0501: case 0x050b: case 0x050c:
        case 0x0602: case 0x0603: case 0x0604: case 0x0605: case 0x0606: case 0x0607:
        case 0x080e: case 0x080f: case 0x0a0b:
        case 0x0c00: case 0x0c01: case 0x0c02: case 0x0c03: case 0x0c04: case 0x0c07: case 0x0c08:
        case 0x0d01: case 0x0d02: case 0x0d03: case 0x0d04: case 0x0d05: case 0x0d07:
            mp_raise_OSError(MP_EBUSY);
    }
    uint32_t cfg = R_PFS->PORT[pin >> 8].PIN[pin & 15].PmnPFS;
    uint32_t function = cfg & (0x1fUL << 24);
    if ((cfg & IOPORT_CFG_PERIPHERAL_PIN) &&
        (function == IOPORT_PERIPHERAL_BUS || function == IOPORT_PERIPHERAL_OSPI ||
         function == IOPORT_PERIPHERAL_SDHI_MMC)) {
        mp_raise_OSError(MP_EBUSY);
    }
    if (ra8_adc_pin_owned(pin) || ra8_pwm_pin_owned(pin) ||
        ra8_uart_pin_owned(pin) || ra8_spi_pin_owned(pin)) {
        mp_raise_OSError(MP_EBUSY);
    }
}

static void machine_pin_require_available(mp_hal_pin_obj_t pin) {
    ra8_machine_pin_require_available((uint16_t)pin);
}

void ra8_mp_hal_pin_write(mp_hal_pin_obj_t pin, int value) {
    machine_pin_require_available(pin);
    /* FSP covers PC/PD too; the SDK rt_pin_write still caps ports at PB. */
    if (R_IOPORT_PinWrite(&g_ioport_ctrl, (bsp_io_port_pin_t)pin,
                        value ? BSP_IO_LEVEL_HIGH : BSP_IO_LEVEL_LOW) != FSP_SUCCESS) {
        mp_raise_OSError(MP_EIO);
    }
}

int ra8_mp_hal_pin_read(mp_hal_pin_obj_t pin) {
    bsp_io_level_t level;
    if (R_IOPORT_PinRead(&g_ioport_ctrl, (bsp_io_port_pin_t)pin, &level) != FSP_SUCCESS) {
        mp_raise_OSError(MP_EIO);
    }
    return level == BSP_IO_LEVEL_HIGH;
}

mp_hal_pin_obj_t mp_hal_get_pin_obj(mp_obj_t obj) {
    mp_int_t pin;
    if (mp_obj_is_type(obj, &machine_pin_type)) {
        pin = ((machine_pin_obj_t *)MP_OBJ_TO_PTR(obj))->pin;
    } else if (mp_obj_is_str(obj)) {
        pin = ra8_gpio_pin_get(mp_obj_str_get_str(obj));
    } else if (mp_obj_is_type(obj, &mp_type_tuple)) {
        /* Retain the old (driver_name, encoded_pin) constructor form. */
        mp_obj_t *items;
        mp_obj_get_array_fixed_n(obj, 2, &items);
        pin = mp_obj_get_int(items[1]);
    } else {
        pin = mp_obj_get_int(obj);
    }
    if (pin < 0 || pin > 0x0d0f || (pin & 0xff) > 15) {
        mp_raise_ValueError(MP_ERROR_TEXT("invalid RA8 pin; use P000..PD15 or an encoded integer"));
    }
    if (!ra8_gpio_is_ready()) { mp_raise_OSError(MP_ENODEV); }
    return pin;
}

void ra8_mp_hal_pin_mode(mp_hal_pin_obj_t pin, unsigned mode) {
    machine_pin_require_available(pin);
    if (ra8_machine_pin_irq_owned(pin)) { mp_raise_OSError(MP_EBUSY); }
    /* Open drain starts released, before SoftI2C issues its initial STOP. */
    if (ra8_gpio_configure(pin, mode, mode == PIN_MODE_OUTPUT_OD ? 1 : -1, RT_FALSE) != RT_EOK) {
        mp_raise_OSError(MP_EIO);
    }
}

static mp_obj_t machine_pin_obj_init_helper(machine_pin_obj_t *self, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args);

static void machine_pin_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{
    machine_pin_obj_t *self = self_in;
    mp_printf(print, "Pin('%s')", self->name);
}

// constructor(drv_name, pin, ...)
mp_obj_t mp_pin_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args)
{
    mp_arg_check_num(n_args, n_kw, 1, MP_OBJ_FUN_ARGS_MAX, true);

    mp_hal_pin_obj_t wanted_pin = mp_hal_get_pin_obj(args[0]);
    machine_pin_obj_t *pin = mp_obj_malloc(machine_pin_obj_t, type);
    unsigned port = wanted_pin >> 8;
    snprintf(pin->name, sizeof(pin->name), "P%c%02u", (int)(port < 10 ? '0' + port : 'A' + port - 10),
             (unsigned)(wanted_pin & 15));
    pin->pin_isr_cb = mp_const_none;
    pin->base = machine_pin_obj_template;
    pin->pin = wanted_pin;

    if (n_args > 1 || n_kw > 0)
    {
        // pin mode given, so configure this GPIO
        mp_map_t kw_args;
        mp_map_init_fixed_table(&kw_args, n_kw, args + n_args);
        machine_pin_obj_init_helper(pin, n_args - 1, args + 1, &kw_args);
    }

    return (mp_obj_t)pin;
}

// pin.init(mode, pull=None, *, value)
static mp_obj_t machine_pin_obj_init_helper(machine_pin_obj_t *self, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_mode, ARG_pull, ARG_value };
    static const mp_arg_t allowed_args[] =
    {
        { MP_QSTR_mode, MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_pull, MP_ARG_OBJ, {.u_obj = mp_const_none}},
        { MP_QSTR_value, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL}},
    };

    // parse args
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // get io mode
    mp_int_t mode = args[ARG_mode].u_int;

    // get pull mode
    uint pull = GPIO_NOPULL;

    if (args[ARG_pull].u_obj != mp_const_none)
    {
        pull = mp_obj_get_int(args[ARG_pull].u_obj);
    }
    if (pull == GPIO_PULLDOWN) {
        mp_raise_NotImplementedError(MP_ERROR_TEXT("RA8 GPIO has no internal pull-down"));
    }
    if (pull != GPIO_NOPULL && pull != GPIO_PULLUP) {
        mp_raise_ValueError(MP_ERROR_TEXT("invalid pull"));
    }
    int value = args[ARG_value].u_obj == MP_OBJ_NULL ? -1 : mp_obj_is_true(args[ARG_value].u_obj);
    if (mode == -1) {
        if (pull != GPIO_NOPULL) { mp_raise_ValueError(MP_ERROR_TEXT("mode is required when changing pull")); }
        if (value >= 0) { mp_hal_pin_write(self->pin, value); }
        return mp_const_none;
    }

    switch (mode)
    {
    case GPIO_MODE_IN:
    {
        if (pull == GPIO_PULLUP)
        {
            mode = PIN_MODE_INPUT_PULLUP;
        }
        else
        {
            mode = PIN_MODE_INPUT;
        }
        break;
    }
    case GPIO_MODE_OUT_PP :
    {
        mode = PIN_MODE_OUTPUT;
        break;
    }
    case GPIO_MODE_OUT_OD :
    {
        mode = PIN_MODE_OUTPUT_OD;
        break;
    }
    case GPIO_MODE_AF_PP :
    case GPIO_MODE_AF_OD :
    case GPIO_MODE_ANALOG :
        mp_raise_NotImplementedError(MP_ERROR_TEXT("alternate functions are configured in RASC"));
    default:
        mp_raise_ValueError(MP_ERROR_TEXT("invalid pin mode"));
    }
    /* Configure output latch and direction together, without an initial low
     * glitch on active-low chip selects. No global IOPORT reinitialization.
     */
    machine_pin_require_available(self->pin);
    if (ra8_machine_pin_irq_owned(self->pin)) { mp_raise_OSError(MP_EBUSY); }
    if (ra8_gpio_configure(self->pin, mode, value, pull == GPIO_PULLUP) != RT_EOK) {
        mp_raise_OSError(MP_EIO);
    }

    return mp_const_none;
}

// fast method for getting/setting pin value
static mp_obj_t machine_pin_call(mp_obj_t self_in, size_t n_args, size_t n_kw, const mp_obj_t *args)
{
    mp_arg_check_num(n_args, n_kw, 0, 1, false);
    machine_pin_obj_t *self = self_in;
    if (n_args == 0)
    {
        return mp_obj_new_bool(mp_hal_pin_read(self->pin));
    }
    else
    {
        mp_hal_pin_write(self->pin, mp_obj_is_true(args[0]));
        return mp_const_none;
    }
}

// pin.init(mode, pull)
static mp_obj_t machine_pin_obj_init(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    return machine_pin_obj_init_helper(args[0], n_args - 1, args + 1, kw_args);
}
MP_DEFINE_CONST_FUN_OBJ_KW(machine_pin_init_obj, 1, machine_pin_obj_init);

// pin.value([value])
static mp_obj_t machine_pin_value(size_t n_args, const mp_obj_t *args)
{
    return machine_pin_call(args[0], n_args - 1, 0, args + 1);
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_pin_value_obj, 1, 2, machine_pin_value);

static mp_obj_t machine_pin_on(mp_obj_t self_in) {
    mp_hal_pin_write(((machine_pin_obj_t *)MP_OBJ_TO_PTR(self_in))->pin, PIN_HIGH);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(machine_pin_on_obj, machine_pin_on);

static mp_obj_t machine_pin_off(mp_obj_t self_in) {
    mp_hal_pin_write(((machine_pin_obj_t *)MP_OBJ_TO_PTR(self_in))->pin, PIN_LOW);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(machine_pin_off_obj, machine_pin_off);

static mp_obj_t machine_pin_toggle(mp_obj_t self_in) {
    machine_pin_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_hal_pin_write(self->pin, !mp_hal_pin_read(self->pin));
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(machine_pin_toggle_obj, machine_pin_toggle);

// pin.name()
static mp_obj_t machine_pin_name(size_t n_args, const mp_obj_t *args)
{
    machine_pin_obj_t *self = (machine_pin_obj_t *)args[0];
    return mp_obj_new_str(self->name, strlen(self->name));
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_pin_name_obj, 1, 2, machine_pin_name);

static mp_uint_t machine_pin_ioctl(mp_obj_t self_in, mp_uint_t request, uintptr_t arg, int *errcode)
{
    (void)errcode;
    machine_pin_obj_t *self = self_in;

    switch (request)
    {
    case MP_PIN_READ:
    {
        uint32_t pin_val = mp_hal_pin_read(self->pin);
        return pin_val;
    }
    case MP_PIN_WRITE:
    {
        mp_hal_pin_write(self->pin, arg);
        return 0;
    }
    }

    *errcode = MP_EINVAL;
    return MP_STREAM_ERROR;
}

static void machine_pin_isr_handler(void *arg)
{
    machine_pin_obj_t *self = arg;
    mp_sched_schedule(self->pin_isr_cb, MP_OBJ_FROM_PTR(self));
}

// pin.irq(handler=None, trigger=IRQ_FALLING|IRQ_RISING)
static mp_obj_t machine_pin_irq(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_handler, ARG_trigger };
    static const mp_arg_t allowed_args[] =
    {
        { MP_QSTR_handler, MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_trigger, MP_ARG_INT, {.u_int = MACHINE_PIN_IRQ_RISING | MACHINE_PIN_IRQ_FALLING} },
    };
    machine_pin_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    mp_obj_t handler = args[ARG_handler].u_obj;
    if (handler != mp_const_none) { machine_pin_require_available(self->pin); }
    if (handler != mp_const_none && !mp_obj_is_callable(handler)) {
        mp_raise_TypeError(MP_ERROR_TEXT("handler must be callable"));
    }
    mp_int_t trigger = args[ARG_trigger].u_int;
    if (trigger < 1 || trigger > (MACHINE_PIN_IRQ_RISING | MACHINE_PIN_IRQ_FALLING)) {
        mp_raise_ValueError(MP_ERROR_TEXT("invalid IRQ trigger"));
    }
    rt_uint8_t mode = trigger == MACHINE_PIN_IRQ_RISING ? PIN_IRQ_MODE_RISING :
                      trigger == MACHINE_PIN_IRQ_FALLING ? PIN_IRQ_MODE_FALLING : PIN_IRQ_MODE_RISING_FALLING;

    int free_slot = -1;
    for (int i = 0; i < 32; i++) {
        mp_obj_t obj = MP_STATE_VM(ra8_pin_irqs)[i];
        if (obj != MP_OBJ_NULL && ((machine_pin_obj_t *)MP_OBJ_TO_PTR(obj))->pin == self->pin) {
            ra8_pin_irq_enable(self->pin, PIN_IRQ_DISABLE);
            ra8_pin_irq_detach(self->pin);
            MP_STATE_VM(ra8_pin_irqs)[i] = MP_OBJ_NULL;
        }
        if (MP_STATE_VM(ra8_pin_irqs)[i] == MP_OBJ_NULL) { free_slot = i; }
    }
    self->pin_isr_cb = handler;
    if (self->pin_isr_cb == mp_const_none) { return mp_const_none; }
    if (free_slot < 0) { mp_raise_OSError(MP_ENOMEM); }
    if (ra8_pin_irq_attach(self->pin, mode, machine_pin_isr_handler, self) != RT_EOK) {
        mp_raise_ValueError(MP_ERROR_TEXT("pin has no available interrupt"));
    }
    MP_STATE_VM(ra8_pin_irqs)[free_slot] = pos_args[0];
    if (ra8_pin_irq_enable(self->pin, PIN_IRQ_ENABLE) != RT_EOK) {
        MP_STATE_VM(ra8_pin_irqs)[free_slot] = MP_OBJ_NULL;
        ra8_pin_irq_detach(self->pin);
        mp_raise_OSError(MP_EIO);
    }

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_KW(machine_pin_irq_obj, 1, machine_pin_irq);
void ra8_pin_irqs_deinit_all(void) {
    for (int i = 0; i < 32; i++) {
        mp_obj_t obj = MP_STATE_VM(ra8_pin_irqs)[i];
        if (obj != MP_OBJ_NULL) {
            machine_pin_obj_t *pin = MP_OBJ_TO_PTR(obj);
            ra8_pin_irq_enable(pin->pin, PIN_IRQ_DISABLE);
            ra8_pin_irq_detach(pin->pin);
            MP_STATE_VM(ra8_pin_irqs)[i] = MP_OBJ_NULL;
        }
    }
}

static const mp_rom_map_elem_t machine_pin_locals_dict_table[] =
{
    // instance methods
    { MP_ROM_QSTR(MP_QSTR_init),    MP_ROM_PTR(&machine_pin_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_value),   MP_ROM_PTR(&machine_pin_value_obj) },
    { MP_ROM_QSTR(MP_QSTR_on),      MP_ROM_PTR(&machine_pin_on_obj) },
    { MP_ROM_QSTR(MP_QSTR_high),    MP_ROM_PTR(&machine_pin_on_obj) },
    { MP_ROM_QSTR(MP_QSTR_off),     MP_ROM_PTR(&machine_pin_off_obj) },
    { MP_ROM_QSTR(MP_QSTR_low),     MP_ROM_PTR(&machine_pin_off_obj) },
    { MP_ROM_QSTR(MP_QSTR_toggle),  MP_ROM_PTR(&machine_pin_toggle_obj) },
    { MP_ROM_QSTR(MP_QSTR_name),    MP_ROM_PTR(&machine_pin_name_obj) },
    { MP_ROM_QSTR(MP_QSTR_irq),     MP_ROM_PTR(&machine_pin_irq_obj) },

    // class constants
    { MP_ROM_QSTR(MP_QSTR_ALT_OD),    MP_ROM_INT(GPIO_MODE_AF_OD) },
    { MP_ROM_QSTR(MP_QSTR_ALT_PP),    MP_ROM_INT(GPIO_MODE_AF_PP) },
    { MP_ROM_QSTR(MP_QSTR_ANALOG),    MP_ROM_INT(GPIO_MODE_ANALOG) },
    { MP_ROM_QSTR(MP_QSTR_IN),        MP_ROM_INT(GPIO_MODE_IN) },
    { MP_ROM_QSTR(MP_QSTR_OUT),       MP_ROM_INT(GPIO_MODE_OUT_PP) },
    { MP_ROM_QSTR(MP_QSTR_OPEN_DRAIN), MP_ROM_INT(GPIO_MODE_OUT_OD) },
    { MP_ROM_QSTR(MP_QSTR_OUT_PP),    MP_ROM_INT(GPIO_MODE_OUT_PP) },
    { MP_ROM_QSTR(MP_QSTR_OUT_OD),    MP_ROM_INT(GPIO_MODE_OUT_OD) },
    { MP_ROM_QSTR(MP_QSTR_PULL_DOWN), MP_ROM_INT(GPIO_PULLDOWN) },
    { MP_ROM_QSTR(MP_QSTR_PULL_NONE), MP_ROM_INT(GPIO_NOPULL) },
    { MP_ROM_QSTR(MP_QSTR_PULL_UP),   MP_ROM_INT(GPIO_PULLUP) },
    { MP_ROM_QSTR(MP_QSTR_IRQ_RISING), MP_ROM_INT(MACHINE_PIN_IRQ_RISING) },
    { MP_ROM_QSTR(MP_QSTR_IRQ_FALLING), MP_ROM_INT(MACHINE_PIN_IRQ_FALLING) },
};

static MP_DEFINE_CONST_DICT(machine_pin_locals_dict, machine_pin_locals_dict_table);

static const mp_pin_p_t machine_pin_pin_p =
{
    .ioctl = machine_pin_ioctl,
};

MP_DEFINE_CONST_OBJ_TYPE(
    machine_pin_type,
    MP_QSTR_Pin,
    MP_TYPE_FLAG_NONE,
    make_new, mp_pin_make_new,
    locals_dict, &machine_pin_locals_dict,
    print, machine_pin_print,
    call, machine_pin_call,
    protocol, &machine_pin_pin_p
);

#endif
