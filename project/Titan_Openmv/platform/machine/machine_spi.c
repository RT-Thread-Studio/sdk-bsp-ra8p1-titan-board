/* SPDX-License-Identifier: MIT
 * Titan Board U18 SPI1: SCK=P102, MOSI=P708, MISO=P709.
 * Chip selects (e.g. P105/P106) are controlled by Python Pin objects.
 * Use FSP SPI_B directly: the SDK RT SPI adapter discards configuration and
 * timeout errors, and cannot safely service machine.SPI's mutable settings.
 */
#include <string.h>
#include <rtdevice.h>
#include "hal_data.h"
#include "py/runtime.h"
#include "py/mphal.h"
#include "py/mperrno.h"
#include "extmod/modmachine.h"
#include "machine_spi.h"

#if MICROPY_PY_MACHINE_SPI
extern void ra8_machine_pin_require_available(uint16_t pin);
extern bool ra8_machine_pin_irq_owned(uint16_t pin);
static const uint16_t spi_pins[] = {BSP_IO_PORT_01_PIN_02, BSP_IO_PORT_07_PIN_08, BSP_IO_PORT_07_PIN_09};
typedef struct _machine_spi_obj_t {
    mp_obj_base_t base;
    spi_cfg_t cfg;
    spi_b_extended_cfg_t ext;
    uint32_t saved_pfs[3];
    uint32_t baudrate;
    uint8_t bits;
    volatile bool complete, failed;
    bool active, busy;
} machine_spi_obj_t;

MP_REGISTER_ROOT_POINTER(struct _machine_spi_obj_t *titan_machine_spi);

bool ra8_spi_pin_owned(uint16_t pin) {
    machine_spi_obj_t *self = MP_STATE_PORT(titan_machine_spi);
    return self && self->active && (pin == spi_pins[0] || pin == spi_pins[1] || pin == spi_pins[2]);
}

void spi1_callback(spi_callback_args_t *args) {
    if (!args || !args->p_context) { return; }
    machine_spi_obj_t *self = args->p_context;
    self->failed = args->event != SPI_EVENT_TRANSFER_COMPLETE;
    self->complete = true;
}

static void spi_close(machine_spi_obj_t *self) {
    if (!self->active) { return; }
    /* Also aborts a timed-out transfer before its stack buffers disappear. */
    R_SPI_B_Close(&g_spi1_ctrl);
    self->active = self->busy = false;
    for (size_t i = 0; i < MP_ARRAY_SIZE(spi_pins); ++i) {
        R_IOPORT_PinCfg(&g_ioport_ctrl, spi_pins[i], self->saved_pfs[i]);
    }
}

void ra8_machine_spi_deinit_all(void) {
    machine_spi_obj_t *self = MP_STATE_PORT(titan_machine_spi);
    if (self) { spi_close(self); }
    MP_STATE_PORT(titan_machine_spi) = NULL;
}

enum { BAUD, POLARITY, PHASE, BITS, FIRSTBIT, SCK, MOSI, MISO };
static const mp_arg_t spi_args[] = {
    {MP_QSTR_baudrate, MP_ARG_INT, {.u_int = -1}},
    {MP_QSTR_polarity, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1}},
    {MP_QSTR_phase, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1}},
    {MP_QSTR_bits, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1}},
    {MP_QSTR_firstbit, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1}},
    {MP_QSTR_sck, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL}},
    {MP_QSTR_mosi, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL}},
    {MP_QSTR_miso, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL}},
};

static void spi_apply(machine_spi_obj_t *self, const mp_arg_val_t *args) {
    if (self->busy) { mp_raise_OSError(MP_EBUSY); }
    uint32_t baud = self->baudrate;
    uint8_t bits = self->bits;
    spi_cfg_t cfg = self->cfg;
    spi_b_extended_cfg_t ext = self->ext;
    if (args[BAUD].u_int != -1) {
        if (args[BAUD].u_int <= 0) { mp_raise_ValueError(MP_ERROR_TEXT("baudrate must be positive")); }
        baud = args[BAUD].u_int;
    }
    for (size_t i = POLARITY; i <= PHASE; ++i) {
        if (args[i].u_int != -1 && args[i].u_int != 0 && args[i].u_int != 1) {
            mp_raise_ValueError(MP_ERROR_TEXT("polarity and phase must be 0 or 1"));
        }
    }
    if (args[POLARITY].u_int != -1) {
        cfg.clk_polarity = args[POLARITY].u_int ? SPI_CLK_POLARITY_HIGH : SPI_CLK_POLARITY_LOW;
    }
    if (args[PHASE].u_int != -1) {
        cfg.clk_phase = args[PHASE].u_int ? SPI_CLK_PHASE_EDGE_EVEN : SPI_CLK_PHASE_EDGE_ODD;
    }
    if (args[BITS].u_int != -1) {
        if (args[BITS].u_int != 8 && args[BITS].u_int != 16 && args[BITS].u_int != 32) {
            mp_raise_ValueError(MP_ERROR_TEXT("bits must be 8, 16 or 32"));
        }
        bits = args[BITS].u_int;
    }
    if (args[FIRSTBIT].u_int != -1) {
        if (args[FIRSTBIT].u_int != MICROPY_PY_MACHINE_SPI_MSB && args[FIRSTBIT].u_int != MICROPY_PY_MACHINE_SPI_LSB) {
            mp_raise_ValueError(MP_ERROR_TEXT("invalid firstbit"));
        }
        cfg.bit_order = args[FIRSTBIT].u_int == MICROPY_PY_MACHINE_SPI_MSB ? SPI_BIT_ORDER_MSB_FIRST : SPI_BIT_ORDER_LSB_FIRST;
    }
    for (size_t i = 0; i < MP_ARRAY_SIZE(spi_pins); ++i) {
        mp_obj_t pin = args[SCK + i].u_obj;
        if (pin != MP_OBJ_NULL && pin != mp_const_none && mp_hal_get_pin_obj(pin) != spi_pins[i]) {
            mp_raise_ValueError(MP_ERROR_TEXT("SPI1 pins are fixed: SCK=P102, MOSI=P708, MISO=P709"));
        }
    }
    if (R_SPI_B_CalculateBitrate(baud, SPI_B_CLOCK_SOURCE_PCLK, &ext.spck_div) != FSP_SUCCESS) {
        mp_raise_ValueError(MP_ERROR_TEXT("SPI baudrate cannot be achieved; use SoftSPI for low rates"));
    }
    /* Report and use the actual rate, which is rounded down by the divider. */
    baud = R_FSP_SystemClockHzGet(BSP_FEATURE_SPI_CLOCK) /
        (2U * (ext.spck_div.spbr + 1U) * (1U << ext.spck_div.brdv));
    if (!self->active) {
        /* RT SPI clients have a different configuration/lifetime owner. */
        if (rt_device_find("spi1") || g_spi1_ctrl.open) { mp_raise_OSError(MP_EBUSY); }
        for (size_t i = 0; i < MP_ARRAY_SIZE(spi_pins); ++i) {
            ra8_machine_pin_require_available(spi_pins[i]);
            if (ra8_machine_pin_irq_owned(spi_pins[i])) { mp_raise_OSError(MP_EBUSY); }
        }
    }
    spi_close(self);
    self->cfg = cfg;
    self->ext = ext;
    self->cfg.p_extend = &self->ext;
    self->baudrate = baud;
    self->bits = bits;
    for (size_t i = 0; i < MP_ARRAY_SIZE(spi_pins); ++i) {
        self->saved_pfs[i] = R_PFS->PORT[spi_pins[i] >> 8].PIN[spi_pins[i] & 0xff].PmnPFS;
    }
    fsp_err_t err = R_SPI_B_Open(&g_spi1_ctrl, &self->cfg);
    if (err != FSP_SUCCESS) { mp_raise_OSError(MP_EIO); }
    self->active = true;
    for (size_t i = 0; i < MP_ARRAY_SIZE(spi_pins); ++i) {
        err = R_IOPORT_PinCfg(&g_ioport_ctrl, spi_pins[i], IOPORT_CFG_PERIPHERAL_PIN |
            IOPORT_PERIPHERAL_SPI | IOPORT_CFG_DRIVE_HIGH);
        if (err != FSP_SUCCESS) { spi_close(self); mp_raise_OSError(MP_EIO); }
    }
}

static void spi_init(mp_obj_base_t *self_in, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw) {
    mp_arg_val_t args[MP_ARRAY_SIZE(spi_args)];
    mp_arg_parse_all(n_args, pos_args, kw, MP_ARRAY_SIZE(spi_args), spi_args, args);
    spi_apply((machine_spi_obj_t *)self_in, args);
}

static void spi_deinit(mp_obj_base_t *self_in) {
    machine_spi_obj_t *self = (machine_spi_obj_t *)self_in;
    if (self->busy) { mp_raise_OSError(MP_EBUSY); }
    spi_close(self);
}

static void spi_transfer(mp_obj_base_t *self_in, size_t len, const uint8_t *src, uint8_t *dest) {
    machine_spi_obj_t *self = (machine_spi_obj_t *)self_in;
    if (!self->active) { mp_raise_OSError(MP_ENODEV); }
    if (self->busy) { mp_raise_OSError(MP_EBUSY); }
    size_t word_bytes = self->bits / 8;
    if (len % word_bytes) { mp_raise_ValueError(MP_ERROR_TEXT("buffer length must be a multiple of SPI word size")); }
    if (!len) { return; }
    /* Preserve the source when two memoryviews partially overlap. Exact
     * in-place transfers only require the bounded per-chunk bounce buffer. */
    uint8_t *snapshot = NULL;
    size_t snapshot_len = len;
    if (src && dest && src != dest && (uintptr_t)src < (uintptr_t)dest + len &&
        (uintptr_t)dest < (uintptr_t)src + len) {
        snapshot = m_new(uint8_t, len);
        memcpy(snapshot, src, len);
        src = snapshot;
    }
    /* FSP non-DMA interrupts operate on aligned CPU buffers. This avoids
     * M85 D-cache coherency and allows unaligned/in-place Python buffers. */
    uint32_t tx[64], rx[64];
    self->busy = true;
    nlr_buf_t nlr;
    if (nlr_push(&nlr) == 0) {
        while (len) {
            size_t chunk = MIN(len, sizeof(tx));
            if (src) { memcpy(tx, src, chunk); } else { memset(tx, 0, chunk); }
            self->complete = self->failed = false;
            fsp_err_t err = R_SPI_B_WriteRead(&g_spi1_ctrl, tx, rx, chunk / word_bytes,
                                             (spi_bit_width_t)(self->bits - 1));
            if (err != FSP_SUCCESS) { mp_raise_OSError(err == FSP_ERR_IN_USE ? MP_EBUSY : MP_EIO); }
            uint32_t start = mp_hal_ticks_ms();
            uint32_t timeout = 100U + (uint32_t)(((uint64_t)chunk * 8000U + self->baudrate - 1) / self->baudrate);
            while (!self->complete) {
                if (!self->active) { mp_raise_OSError(MP_ENODEV); }
                if ((uint32_t)(mp_hal_ticks_ms() - start) >= timeout) { mp_raise_OSError(MP_ETIMEDOUT); }
                MICROPY_EVENT_POLL_HOOK
            }
            if (self->failed) { mp_raise_OSError(MP_EIO); }
            if (dest) { memcpy(dest, rx, chunk); dest += chunk; }
            if (src) { src += chunk; }
            len -= chunk;
        }
        nlr_pop();
        self->busy = false;
        if (snapshot) { m_del(uint8_t, snapshot, snapshot_len); }
    } else {
        spi_close(self);
        if (snapshot) { m_del(uint8_t, snapshot, snapshot_len); }
        nlr_jump(nlr.ret_val);
    }
}

static void spi_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    (void)kind;
    machine_spi_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "SPI(1, baudrate=%u, polarity=%u, phase=%u, bits=%u, firstbit=%u)%s",
        (unsigned)self->baudrate, self->cfg.clk_polarity == SPI_CLK_POLARITY_HIGH,
        self->cfg.clk_phase == SPI_CLK_PHASE_EDGE_EVEN, self->bits,
        self->cfg.bit_order == SPI_BIT_ORDER_MSB_FIRST ? MICROPY_PY_MACHINE_SPI_MSB : MICROPY_PY_MACHINE_SPI_LSB,
        self->active ? "" : " [deinitialized]");
}

static mp_obj_t spi_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    #if MICROPY_PY_MACHINE_SOFTSPI
    MP_MACHINE_SPI_CHECK_FOR_LEGACY_SOFTSPI_CONSTRUCTION(n_args, n_kw, args);
    #endif
    mp_arg_check_num(n_args, n_kw, 1, 2, true);
    bool valid_id = mp_obj_is_str(args[0]) ? !strcmp(mp_obj_str_get_str(args[0]), "spi1") : mp_obj_get_int(args[0]) == 1;
    if (!valid_id) { mp_raise_ValueError(MP_ERROR_TEXT("available hardware SPI is SPI(1)")); }
    mp_arg_val_t parsed[MP_ARRAY_SIZE(spi_args)];
    mp_arg_parse_all_kw_array(n_args - 1, n_kw, args + 1, MP_ARRAY_SIZE(spi_args), spi_args, parsed);
    machine_spi_obj_t *self = MP_STATE_PORT(titan_machine_spi);
    if (!self) {
        self = mp_obj_malloc(machine_spi_obj_t, type);
        self->active = self->busy = false;
        self->baudrate = 1000000;
        self->bits = 8;
        self->cfg = g_spi1_cfg;
        self->ext = *(const spi_b_extended_cfg_t *)g_spi1_cfg.p_extend;
        self->cfg.operating_mode = SPI_MODE_MASTER;
        self->cfg.clk_polarity = SPI_CLK_POLARITY_LOW;
        self->cfg.clk_phase = SPI_CLK_PHASE_EDGE_ODD;
        self->cfg.bit_order = SPI_BIT_ORDER_MSB_FIRST;
        self->cfg.p_callback = spi1_callback;
        self->cfg.p_context = self;
        self->cfg.p_transfer_tx = self->cfg.p_transfer_rx = NULL;
        self->cfg.p_extend = &self->ext;
        self->ext.spi_clksyn = SPI_B_SSL_MODE_CLK_SYN;
        self->ext.spi_comm = SPI_B_COMMUNICATION_FULL_DUPLEX;
        self->ext.clock_source = SPI_B_CLOCK_SOURCE_PCLK;
        self->ext.byte_swap = SPI_B_BYTE_SWAP_DISABLE;
        MP_STATE_PORT(titan_machine_spi) = self;
    }
    if (!self->active || n_args > 1 || n_kw) { spi_apply(self, parsed); }
    return MP_OBJ_FROM_PTR(self);
}

static const mp_machine_spi_p_t spi_protocol = {
    .init = spi_init, .deinit = spi_deinit, .transfer = spi_transfer,
};
MP_DEFINE_CONST_OBJ_TYPE(machine_spi_type, MP_QSTR_SPI, MP_TYPE_FLAG_NONE,
    make_new, spi_make_new, print, spi_print, protocol, &spi_protocol, locals_dict, &mp_machine_spi_locals_dict);
#endif
