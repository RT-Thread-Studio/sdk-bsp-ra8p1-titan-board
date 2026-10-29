/* SPDX-License-Identifier: MIT
 * Titan Board H1: SCI2 TX=P801, RX=P802. SCI1 belongs to the RT console.
 * FSP owns RX interrupts; TX uses the SCI_B data register synchronously so
 * no interrupt can retain a Python buffer after write() returns/raises.
 */
#include <string.h>
#include <rtdevice.h>
#include "hal_data.h"
#include "py/runtime.h"
#include "py/mphal.h"
#include "py/mperrno.h"
#include "py/stream.h"
#include "machine_uart.h"

#if MICROPY_PY_MACHINE_UART
extern void ra8_machine_pin_require_available(uint16_t pin);
extern bool ra8_machine_pin_irq_owned(uint16_t pin);

static const uint16_t uart_pins[] = {BSP_IO_PORT_08_PIN_01, BSP_IO_PORT_08_PIN_02};
typedef struct _machine_uart_obj_t {
    mp_obj_base_t base;
    uart_cfg_t cfg;
    sci_b_uart_extended_cfg_t ext;
    sci_b_baud_setting_t baud;
    uint32_t saved_pfs[2];
    uint8_t *rxbuf;
    uint16_t rxlen;
    volatile uint16_t head, tail;
    uint32_t baudrate, timeout, timeout_char;
    bool active;
} machine_uart_obj_t;

MP_REGISTER_ROOT_POINTER(struct _machine_uart_obj_t *titan_machine_uart);

bool ra8_uart_pin_owned(uint16_t pin) {
    machine_uart_obj_t *self = MP_STATE_PORT(titan_machine_uart);
    return self && self->active && (pin == uart_pins[0] || pin == uart_pins[1]);
}

void user_uart2_callback(uart_callback_args_t *args) {
    if (!args || !args->p_context) { return; }
    machine_uart_obj_t *self = args->p_context;
    if (args->event == UART_EVENT_RX_CHAR) {
        uint16_t next = (self->head + 1) % self->rxlen;
        if (next != self->tail) {
            self->rxbuf[self->head] = args->data;
            self->head = next;
        }
    }
}

static void uart_close(machine_uart_obj_t *self) {
    if (!self->active) { return; }
    /* Close disables RX IRQs before the root/ring can be released. */
    R_SCI_B_UART_Close(&g_uart2_ctrl);
    self->active = false;
    for (size_t i = 0; i < MP_ARRAY_SIZE(uart_pins); ++i) {
        R_IOPORT_PinCfg(&g_ioport_ctrl, uart_pins[i], self->saved_pfs[i]);
    }
}

void ra8_machine_uart_deinit_all(void) {
    machine_uart_obj_t *self = MP_STATE_PORT(titan_machine_uart);
    if (self) { uart_close(self); }
    MP_STATE_PORT(titan_machine_uart) = NULL;
}

static void uart_require_active(machine_uart_obj_t *self) {
    if (!self->active) { mp_raise_OSError(MP_ENODEV); }
}

static void uart_init_helper(machine_uart_obj_t *self, size_t n_args,
                             const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { BAUD, BITS, PARITY, STOP, TX, RX, TIMEOUT, TIMEOUT_CHAR, RXBUF, TXBUF, FLOW };
    static const mp_arg_t allowed[] = {
        {MP_QSTR_baudrate, MP_ARG_INT, {.u_int = 115200}},
        {MP_QSTR_bits, MP_ARG_INT, {.u_int = 8}},
        {MP_QSTR_parity, MP_ARG_OBJ, {.u_rom_obj = MP_ROM_NONE}},
        {MP_QSTR_stop, MP_ARG_INT, {.u_int = 1}},
        {MP_QSTR_tx, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL}},
        {MP_QSTR_rx, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL}},
        {MP_QSTR_timeout, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0}},
        {MP_QSTR_timeout_char, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0}},
        {MP_QSTR_rxbuf, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 256}},
        {MP_QSTR_txbuf, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0}},
        {MP_QSTR_flow, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0}},
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed), allowed, args);
    if (args[BAUD].u_int <= 0) { mp_raise_ValueError(MP_ERROR_TEXT("baudrate must be positive")); }
    if (args[BITS].u_int != 7 && args[BITS].u_int != 8) {
        mp_raise_ValueError(MP_ERROR_TEXT("bits must be 7 or 8"));
    }
    if (args[STOP].u_int != 1 && args[STOP].u_int != 2) {
        mp_raise_ValueError(MP_ERROR_TEXT("stop must be 1 or 2"));
    }
    mp_int_t parity = args[PARITY].u_obj == mp_const_none ? -1 : mp_obj_get_int(args[PARITY].u_obj);
    if (parity < -1 || parity > 1 || (parity == -1 && args[PARITY].u_obj != mp_const_none)) {
        mp_raise_ValueError(MP_ERROR_TEXT("parity must be None, 0 or 1"));
    }
    if (args[FLOW].u_int || args[TXBUF].u_int) {
        mp_raise_NotImplementedError(MP_ERROR_TEXT("flow control and buffered TX are unavailable"));
    }
    if (args[TIMEOUT].u_int < 0 || args[TIMEOUT_CHAR].u_int < 0) {
        mp_raise_ValueError(MP_ERROR_TEXT("timeouts must be nonnegative"));
    }
    if (args[RXBUF].u_int < 1 || args[RXBUF].u_int > 32766) {
        mp_raise_ValueError(MP_ERROR_TEXT("rxbuf must be 1..32766"));
    }
    for (size_t i = 0; i < 2; ++i) {
        mp_obj_t pin = args[TX + i].u_obj;
        if (pin != MP_OBJ_NULL && pin != mp_const_none && mp_hal_get_pin_obj(pin) != uart_pins[i]) {
            mp_raise_ValueError(MP_ERROR_TEXT("UART2 pins are fixed: TX=P801, RX=P802"));
        }
    }
    sci_b_baud_setting_t baud;
    if (R_SCI_B_UART_BaudCalculate(args[BAUD].u_int, true, 2000, &baud) != FSP_SUCCESS) {
        mp_raise_ValueError(MP_ERROR_TEXT("baudrate cannot be achieved within 2 percent"));
    }
    if (!self->active) {
        /* A registered RT UART has its own driver and IRQ owner. */
        if (rt_device_find("uart2") || g_uart2_ctrl.open) { mp_raise_OSError(MP_EBUSY); }
        for (size_t i = 0; i < 2; ++i) {
            ra8_machine_pin_require_available(uart_pins[i]);
            if (ra8_machine_pin_irq_owned(uart_pins[i])) { mp_raise_OSError(MP_EBUSY); }
        }
    }
    uint16_t rxlen = args[RXBUF].u_int + 1;
    uint8_t *rxbuf = m_new(uint8_t, rxlen);
    uart_close(self);
    if (self->rxbuf) { m_del(uint8_t, self->rxbuf, self->rxlen); }
    self->rxbuf = rxbuf;
    self->rxlen = rxlen;
    self->head = self->tail = 0;
    self->baud = baud;
    self->cfg = g_uart2_cfg;
    self->ext = *(const sci_b_uart_extended_cfg_t *)g_uart2_cfg.p_extend;
    self->cfg.data_bits = args[BITS].u_int == 7 ? UART_DATA_BITS_7 : UART_DATA_BITS_8;
    self->cfg.stop_bits = args[STOP].u_int == 1 ? UART_STOP_BITS_1 : UART_STOP_BITS_2;
    self->cfg.parity = parity == -1 ? UART_PARITY_OFF : (parity == 0 ? UART_PARITY_EVEN : UART_PARITY_ODD);
    self->cfg.p_callback = user_uart2_callback;
    self->cfg.p_context = self;
    self->cfg.p_extend = &self->ext;
    self->cfg.p_transfer_tx = self->cfg.p_transfer_rx = NULL;
    self->ext.p_baud_setting = &self->baud;
    self->ext.rx_fifo_trigger = SCI_B_UART_RX_FIFO_TRIGGER_1;
    self->baudrate = args[BAUD].u_int;
    self->timeout = args[TIMEOUT].u_int;
    self->timeout_char = MAX((uint32_t)args[TIMEOUT_CHAR].u_int, 13000U / self->baudrate + 2U);
    for (size_t i = 0; i < 2; ++i) {
        self->saved_pfs[i] = R_PFS->PORT[uart_pins[i] >> 8].PIN[uart_pins[i] & 0xff].PmnPFS;
    }
    fsp_err_t err = R_SCI_B_UART_Open(&g_uart2_ctrl, &self->cfg);
    if (err != FSP_SUCCESS) { mp_raise_OSError(MP_EIO); }
    self->active = true;
    for (size_t i = 0; i < 2; ++i) {
        err = R_IOPORT_PinCfg(&g_ioport_ctrl, uart_pins[i], IOPORT_CFG_PERIPHERAL_PIN |
                              IOPORT_PERIPHERAL_SCI0_2_4_6_8);
        if (err != FSP_SUCCESS) { uart_close(self); mp_raise_OSError(MP_EIO); }
    }
}

static mp_obj_t uart_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 1, 5, true);
    mp_int_t id = mp_obj_get_int(args[0]);
    if (id == 1) { mp_raise_ValueError(MP_ERROR_TEXT("UART1 is reserved for the RT console")); }
    if (id != 2) { mp_raise_ValueError(MP_ERROR_TEXT("available UART is UART(2) on H1")); }
    machine_uart_obj_t *self = MP_STATE_PORT(titan_machine_uart);
    if (!self) {
        self = mp_obj_malloc(machine_uart_obj_t, type);
        self->active = false;
        self->rxbuf = NULL;
        MP_STATE_PORT(titan_machine_uart) = self;
    }
    if (!self->active || n_args > 1 || n_kw) {
        mp_map_t kw;
        mp_map_init_fixed_table(&kw, n_kw, args + n_args);
        uart_init_helper(self, n_args - 1, args + 1, &kw);
    }
    return MP_OBJ_FROM_PTR(self);
}

static mp_obj_t uart_init(size_t n_args, const mp_obj_t *args, mp_map_t *kw) {
    uart_init_helper(MP_OBJ_TO_PTR(args[0]), n_args - 1, args + 1, kw);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_KW(uart_init_obj, 1, uart_init);

static mp_obj_t uart_deinit(mp_obj_t self_in) { uart_close(MP_OBJ_TO_PTR(self_in)); return mp_const_none; }
static MP_DEFINE_CONST_FUN_OBJ_1(uart_deinit_obj, uart_deinit);

static uint16_t uart_available(machine_uart_obj_t *self) {
    rt_base_t level = rt_hw_interrupt_disable();
    uint16_t count = (self->head + self->rxlen - self->tail) % self->rxlen;
    rt_hw_interrupt_enable(level);
    return count;
}
static mp_obj_t uart_any(mp_obj_t self_in) {
    machine_uart_obj_t *self = MP_OBJ_TO_PTR(self_in);
    uart_require_active(self);
    return MP_OBJ_NEW_SMALL_INT(uart_available(self));
}
static MP_DEFINE_CONST_FUN_OBJ_1(uart_any_obj, uart_any);

static mp_obj_t uart_txdone(mp_obj_t self_in) {
    uart_require_active(MP_OBJ_TO_PTR(self_in));
    return mp_obj_new_bool(g_uart2_ctrl.p_reg->CSR_b.TEND);
}
static MP_DEFINE_CONST_FUN_OBJ_1(uart_txdone_obj, uart_txdone);

static mp_uint_t uart_read(mp_obj_t self_in, void *buf, mp_uint_t size, int *errcode) {
    machine_uart_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if (!self->active) { *errcode = MP_ENODEV; return MP_STREAM_ERROR; }
    if (!size) { return 0; }
    mp_uint_t received = 0;
    uint32_t start = mp_hal_ticks_ms(), timeout = self->timeout;
    while (received < size) {
        if (!self->active) { *errcode = MP_ENODEV; return received ? received : MP_STREAM_ERROR; }
        rt_base_t level = rt_hw_interrupt_disable();
        bool available = self->head != self->tail;
        if (available) {
            ((uint8_t *)buf)[received++] = self->rxbuf[self->tail];
            self->tail = (self->tail + 1) % self->rxlen;
        }
        rt_hw_interrupt_enable(level);
        if (available) { start = mp_hal_ticks_ms(); timeout = self->timeout_char; }
        else if ((uint32_t)(mp_hal_ticks_ms() - start) >= timeout) { break; }
        else { MICROPY_EVENT_POLL_HOOK rt_thread_mdelay(1); }
    }
    if (!received) { *errcode = MP_EAGAIN; return MP_STREAM_ERROR; }
    return received;
}

static mp_uint_t uart_write(mp_obj_t self_in, const void *buf, mp_uint_t size, int *errcode) {
    machine_uart_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if (!self->active) { *errcode = MP_ENODEV; return MP_STREAM_ERROR; }
    mp_uint_t sent = 0;
    uint32_t start = mp_hal_ticks_ms(), timeout = self->timeout, last_poll = start;
    while (sent < size) {
        if (!self->active) { *errcode = MP_ENODEV; return sent ? sent : MP_STREAM_ERROR; }
        /* Waiting for the shift register to empty also prevents FIFO overflow.
         * No TX IRQ is enabled and no Python memory is handed to FSP. */
        if (g_uart2_ctrl.p_reg->CSR_b.TEND) {
            g_uart2_ctrl.p_reg->TDR_BY = ((const uint8_t *)buf)[sent++];
            if (g_uart2_ctrl.fifo_depth) { g_uart2_ctrl.p_reg->CFCLR = R_SCI_B0_CFCLR_TDREC_Msk; }
            timeout = self->timeout_char;
            start = mp_hal_ticks_ms();
        } else {
            uint32_t now = mp_hal_ticks_ms();
            if ((uint32_t)(now - start) >= timeout) {
                *errcode = MP_EAGAIN; return sent ? sent : MP_STREAM_ERROR;
            }
            if (now != last_poll) { last_poll = now; MICROPY_EVENT_POLL_HOOK }
        }
    }
    return sent;
}

static mp_uint_t uart_ioctl(mp_obj_t self_in, mp_uint_t request, uintptr_t arg, int *errcode) {
    machine_uart_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if (request == MP_STREAM_CLOSE) { uart_close(self); return 0; }
    if (!self->active) { *errcode = MP_ENODEV; return MP_STREAM_ERROR; }
    if (request == MP_STREAM_POLL) {
        return ((arg & MP_STREAM_POLL_RD) && uart_available(self) ? MP_STREAM_POLL_RD : 0) |
               ((arg & MP_STREAM_POLL_WR) && g_uart2_ctrl.p_reg->CSR_b.TEND ? MP_STREAM_POLL_WR : 0);
    }
    if (request == MP_STREAM_FLUSH) {
        uint32_t start = mp_hal_ticks_ms();
        while (self->active && !g_uart2_ctrl.p_reg->CSR_b.TEND) {
            if ((uint32_t)(mp_hal_ticks_ms() - start) >= MAX(self->timeout, self->timeout_char)) {
                *errcode = MP_ETIMEDOUT; return MP_STREAM_ERROR;
            }
            MICROPY_EVENT_POLL_HOOK
        }
        if (!self->active) { *errcode = MP_ENODEV; return MP_STREAM_ERROR; }
        return 0;
    }
    *errcode = MP_EINVAL;
    return MP_STREAM_ERROR;
}

#if MICROPY_PY_MACHINE_UART_READCHAR_WRITECHAR
static mp_obj_t uart_readchar(mp_obj_t self_in) {
    uint8_t byte;
    int error = 0;
    mp_uint_t count = uart_read(self_in, &byte, 1, &error);
    if (count == MP_STREAM_ERROR && error != MP_EAGAIN) { mp_raise_OSError(error); }
    return MP_OBJ_NEW_SMALL_INT(count == 1 ? byte : -1);
}
static MP_DEFINE_CONST_FUN_OBJ_1(uart_readchar_obj, uart_readchar);

static mp_obj_t uart_writechar(mp_obj_t self_in, mp_obj_t char_in) {
    mp_int_t value = mp_obj_get_int(char_in);
    if (value < 0 || value > 255) { mp_raise_ValueError(MP_ERROR_TEXT("character must be 0..255")); }
    uint8_t byte = value;
    int error = 0;
    if (uart_write(self_in, &byte, 1, &error) != 1) {
        mp_raise_OSError(error == MP_EAGAIN ? MP_ETIMEDOUT : error);
    }
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_2(uart_writechar_obj, uart_writechar);
#endif

static void uart_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    (void)kind;
    machine_uart_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if (!self->active) { mp_printf(print, "UART(2) [deinitialized]"); return; }
    mp_printf(print, "UART(2, baudrate=%u, bits=%u, parity=%s, stop=%u, timeout=%u, timeout_char=%u, rxbuf=%u)",
        (unsigned)self->baudrate, self->cfg.data_bits == UART_DATA_BITS_7 ? 7 : 8,
        self->cfg.parity == UART_PARITY_OFF ? "None" : (self->cfg.parity == UART_PARITY_EVEN ? "0" : "1"),
        self->cfg.stop_bits == UART_STOP_BITS_1 ? 1 : 2, (unsigned)self->timeout,
        (unsigned)self->timeout_char, self->rxlen - 1);
}

static const mp_rom_map_elem_t uart_locals_table[] = {
    {MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&uart_init_obj)},
    {MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&uart_deinit_obj)},
    {MP_ROM_QSTR(MP_QSTR_any), MP_ROM_PTR(&uart_any_obj)},
    {MP_ROM_QSTR(MP_QSTR_txdone), MP_ROM_PTR(&uart_txdone_obj)},
    {MP_ROM_QSTR(MP_QSTR_flush), MP_ROM_PTR(&mp_stream_flush_obj)},
    {MP_ROM_QSTR(MP_QSTR_read), MP_ROM_PTR(&mp_stream_read1_obj)},
    {MP_ROM_QSTR(MP_QSTR_readinto), MP_ROM_PTR(&mp_stream_readinto1_obj)},
    {MP_ROM_QSTR(MP_QSTR_readline), MP_ROM_PTR(&mp_stream_unbuffered_readline_obj)},
    {MP_ROM_QSTR(MP_QSTR_write), MP_ROM_PTR(&mp_stream_write1_obj)},
    #if MICROPY_PY_MACHINE_UART_READCHAR_WRITECHAR
    {MP_ROM_QSTR(MP_QSTR_readchar), MP_ROM_PTR(&uart_readchar_obj)},
    {MP_ROM_QSTR(MP_QSTR_writechar), MP_ROM_PTR(&uart_writechar_obj)},
    #endif
};
static MP_DEFINE_CONST_DICT(uart_locals, uart_locals_table);
static const mp_stream_p_t uart_stream = {
    .read = uart_read, .write = uart_write, .ioctl = uart_ioctl, .is_text = false,
};
MP_DEFINE_CONST_OBJ_TYPE(machine_uart_type, MP_QSTR_UART, MP_TYPE_FLAG_ITER_IS_STREAM,
    make_new, uart_make_new, print, uart_print, protocol, &uart_stream, locals_dict, &uart_locals);
#endif
