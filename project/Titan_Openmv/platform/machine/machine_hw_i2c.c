/* SPDX-License-Identifier: MIT
 * MicroPython I2C protocol over the RT-Thread board buses.
 * The upstream extmod implementation owns the Python API and memory addressing.
 */
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <rtdevice.h>
#include "py/runtime.h"
#include "py/mperrno.h"
#include "py/mphal.h"
#include "extmod/modmachine.h"

#if MICROPY_PY_MACHINE_I2C
#if !MICROPY_PY_MACHINE_I2C_TRANSFER_WRITE1
#error "RT-Thread I2C requires atomic WRITE1 transfers for memory reads"
#endif

typedef struct {
    mp_obj_base_t base;
    struct rt_i2c_bus_device *bus;
    rt_base_t scl, sda;
    uint32_t freq, timeout_us;
} machine_hard_i2c_obj_t;

extern const mp_obj_type_t machine_i2c_type;

static const mp_arg_t i2c_args[] = {
    { MP_QSTR_freq, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
    { MP_QSTR_timeout, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
    { MP_QSTR_scl, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    { MP_QSTR_sda, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
};

static void i2c_apply_args(machine_hard_i2c_obj_t *self, const mp_arg_val_t *args) {
    uint32_t freq = self->freq;
    uint32_t timeout_us = self->timeout_us;
    if (args[0].u_int != -1) {
        if (args[0].u_int <= 0 || args[0].u_int > 400000) {
            mp_raise_ValueError(MP_ERROR_TEXT("freq must be 1..400000"));
        }
        freq = args[0].u_int;
    }
    if (args[1].u_int != -1) {
        if (args[1].u_int <= 0 || args[1].u_int > 10000000) {
            mp_raise_ValueError(MP_ERROR_TEXT("timeout must be 1..10000000 us"));
        }
        timeout_us = args[1].u_int;
    }
    for (unsigned i = 2; i < 4; ++i) {
        if (args[i].u_obj != MP_OBJ_NULL && args[i].u_obj != mp_const_none &&
            mp_hal_get_pin_obj(args[i].u_obj) != (i == 2 ? self->scl : self->sda)) {
            mp_raise_ValueError(MP_ERROR_TEXT("I2C pins are fixed by the board; use SoftI2C"));
        }
    }
    if (freq != 400000 || timeout_us != 100000) {
        mp_raise_NotImplementedError(MP_ERROR_TEXT("hardware I2C uses the board 400 kHz / 100 ms configuration"));
    }
    self->freq = freq;
    self->timeout_us = timeout_us;
}

static void machine_hard_i2c_init(mp_obj_base_t *self_in, size_t n_args,
                                 const mp_obj_t *pos_args, mp_map_t *kw_args) {
    mp_arg_val_t args[MP_ARRAY_SIZE(i2c_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(i2c_args), i2c_args, args);
    i2c_apply_args((machine_hard_i2c_obj_t *)self_in, args);
}

static void machine_hard_i2c_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    (void)kind;
    machine_hard_i2c_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "I2C('%s', freq=%u, timeout=%u)",
              self->bus->parent.parent.name, (unsigned)self->freq, (unsigned)self->timeout_us);
}

static int machine_hard_i2c_transfer(mp_obj_base_t *self_in, uint16_t addr, size_t n,
                                    mp_machine_i2c_buf_t *bufs, unsigned flags) {
    machine_hard_i2c_obj_t *self = (machine_hard_i2c_obj_t *)self_in;
    if (addr > 0x7f || n == 0 || n > UINT16_MAX) {
        return -MP_EINVAL;
    }
    /* Do not leave the shared camera bus active across arbitrary Python code.
     * Memory reads instead submit WRITE1 + READ atomically under one bus lock.
     * SoftI2C is available for applications requiring explicit stop=False.
     */
    if (!(flags & MP_MACHINE_I2C_FLAG_STOP)) {
        return -MP_EOPNOTSUPP;
    }
    bool write1 = flags & MP_MACHINE_I2C_FLAG_WRITE1;
    bool read = flags & MP_MACHINE_I2C_FLAG_READ;
    if (write1 && (!read || n != 2)) {
        return -MP_EINVAL;
    }
    size_t bytes = 0;
    for (size_t i = 0; i < n; ++i) {
        if (bufs[i].len > UINT16_MAX || bufs[i].len > INT_MAX - bytes) {
            return -MP_EINVAL;
        }
        bytes += bufs[i].len;
    }
    /* FSP supports a zero-byte WRITE (address + ACK + STOP), which is what
     * upstream scan() uses. Zero-byte READ is not supported by FSP. */
    if ((read && !bytes) || (write1 && (!bufs[0].len || !bufs[1].len))) {
        return -MP_EINVAL;
    }
    if (bytes > UINT16_MAX || (read && !write1 && n != 1)) { return -MP_EINVAL; }
    struct rt_i2c_msg msgs[2];
    uint8_t *joined = NULL;
    unsigned count = write1 ? 2 : 1;
    if (!read && n > 1 && bytes) {
        /* FSP has no RT_I2C_NO_START byte continuation. Join writev buffers
         * into one address phase, including writeto_mem's register prefix.
         */
        joined = m_new(uint8_t, bytes);
        size_t offset = 0;
        for (size_t i = 0; i < n; ++i) {
            memcpy(joined + offset, bufs[i].buf, bufs[i].len);
            offset += bufs[i].len;
        }
    }
    msgs[0] = (struct rt_i2c_msg){.addr = addr,
        .flags = write1 ? RT_I2C_WR | RT_I2C_NO_STOP : (read ? RT_I2C_RD : RT_I2C_WR),
        .len = write1 ? bufs[0].len : bytes,
        .buf = joined ? joined : bufs[0].buf};
    if (write1) {
        msgs[1] = (struct rt_i2c_msg){.addr = addr, .flags = RT_I2C_RD,
            .len = bufs[1].len, .buf = bufs[1].buf};
    }
    /* RT-Thread serialises the entire message array with one bus lock. */
    rt_ssize_t result = rt_i2c_transfer(self->bus, msgs, count);
    if (joined) { m_del(uint8_t, joined, bytes); }
    return result == count ? (read ? 0 : (int)bytes) : -MP_EIO;
}

mp_obj_t machine_hard_i2c_make_new(const mp_obj_type_t *type, size_t n_args,
                                  size_t n_kw, const mp_obj_t *all_args) {
    mp_arg_check_num(n_args, n_kw, 1, 1, true);
    mp_arg_val_t args[MP_ARRAY_SIZE(i2c_args)];
    mp_arg_parse_all_kw_array(n_args - 1, n_kw, all_args + 1,
                             MP_ARRAY_SIZE(i2c_args), i2c_args, args);
    char bus_name[RT_NAME_MAX];
    if (mp_obj_is_str(all_args[0])) {
        const char *name = mp_obj_str_get_str(all_args[0]);
        if (strlen(name) >= sizeof(bus_name)) {
            mp_raise_OSError(MP_ENODEV);
        }
        strcpy(bus_name, name);
    } else {
        mp_int_t id = mp_obj_get_int(all_args[0]);
        if (id < 0 || id > 255) {
            mp_raise_OSError(MP_ENODEV);
        }
        snprintf(bus_name, sizeof(bus_name), "i2c%u", (unsigned)id);
    }
    rt_device_t dev = rt_device_find(bus_name);
    if (!dev || dev->type != RT_Device_Class_I2CBUS) {
        mp_raise_OSError(MP_ENODEV);
    }
    rt_base_t scl = PIN_NONE, sda = PIN_NONE;
#ifdef BSP_USING_HW_I2C0
    if (!strcmp(bus_name, "i2c0")) {
        scl = 0x040a; sda = 0x0409; /* Titan schematic SCL0/SDA0. */
    }
#endif
#ifdef BSP_USING_HW_I2C1
    if (!strcmp(bus_name, "i2c1")) {
        scl = 0x050c; sda = 0x050b; /* IMU and audio control bus. */
    }
#endif
    if (scl == PIN_NONE) {
        mp_raise_NotImplementedError(MP_ERROR_TEXT("only board I2C0/I2C1 are exposed; use SoftI2C for other pins"));
    }
    machine_hard_i2c_obj_t *self = mp_obj_malloc(machine_hard_i2c_obj_t, type);
    self->bus = (struct rt_i2c_bus_device *)dev;
    self->scl = scl;
    self->sda = sda;
    self->freq = 400000;
    self->timeout_us = 100000;
    i2c_apply_args(self, args);
    return MP_OBJ_FROM_PTR(self);
}

static const mp_machine_i2c_p_t machine_hard_i2c_p = {
    .transfer_supports_write1 = true,
    .init = machine_hard_i2c_init,
    .transfer = machine_hard_i2c_transfer,
};

MP_DEFINE_CONST_OBJ_TYPE(
    machine_i2c_type,
    MP_QSTR_I2C,
    MP_TYPE_FLAG_NONE,
    make_new, machine_hard_i2c_make_new,
    print, machine_hard_i2c_print,
    protocol, &machine_hard_i2c_p,
    locals_dict, &mp_machine_i2c_locals_dict
    );
#endif
