/* SPDX-License-Identifier: MIT
 * Titan Board U18 analog inputs, using the RA8P1 ADC_B (not the RA8D1 ADC12).
 */
#include <string.h>
#include <hal_data.h>
#include <rthw.h>
#include "py/runtime.h"
#include "py/mperrno.h"
#include "modmachine.h"
#include "mphalport.h"
#include "machine_adc.h"

#if MICROPY_PY_MACHINE_ADC
#define ADC_INPUT_COUNT (10)
#define ADC_WAIT_MS (250)

/* ANxxx has a flat physical channel number on ADC_B. Converter selection is
 * derived from the MCU's channel masks; AN006, for example, uses converter 1.
 * P003 is the board key, and P007/P010..P013 are not U18 analog inputs.
 */
static const uint8_t adc_channels[ADC_INPUT_COUNT] = {0, 1, 2, 4, 5, 6, 8, 9, 14, 15};
typedef struct {
    mp_obj_base_t base;
    uint32_t saved_pin_cfg;
    uint16_t sample_states;
    uint8_t index;
    bool active;
} machine_adc_obj_t;

MP_REGISTER_ROOT_POINTER(mp_obj_t ra8_adc_owners[10]);
static adc_cfg_t adc_cfg;
static adc_b_extended_cfg_t adc_extend;
static adc_b_virtual_channel_cfg_t adc_channel_cfg;
static adc_b_virtual_channel_cfg_t *adc_virtual_channels[] = {&adc_channel_cfg};
static adc_b_group_cfg_t adc_group_cfg;
static adc_b_group_cfg_t *adc_groups[] = {&adc_group_cfg};
static const adc_b_scan_cfg_t adc_scan_cfg = {.group_count = 1, .p_adc_groups = adc_groups};
static bool adc_taken;
static bool adc_was_open;
static int adc_selected = -1;
static uint16_t adc_selected_states;

static void adc_check(fsp_err_t error) {
    if (error == FSP_SUCCESS) { return; }
    if (error == FSP_ERR_TIMEOUT) { mp_raise_OSError(MP_ETIMEDOUT); }
    if (error == FSP_ERR_IN_USE) { mp_raise_OSError(MP_EBUSY); }
    mp_raise_OSError(MP_EIO);
}

static fsp_err_t adc_pin_config(uint16_t pin, uint32_t config) {
    rt_base_t level = rt_hw_interrupt_disable();
    fsp_err_t error = R_IOPORT_PinCfg(&g_ioport_ctrl, (bsp_io_port_pin_t)pin, config);
    rt_hw_interrupt_enable(level);
    return error;
}

bool ra8_adc_pin_owned(uint16_t pin) {
    for (unsigned i = 0; i < ADC_INPUT_COUNT; ++i) {
        if (pin == adc_channels[i] && MP_STATE_PORT(ra8_adc_owners[i]) != MP_OBJ_NULL) { return true; }
    }
    return false;
}

static bool adc_has_owners(void) {
    for (unsigned i = 0; i < ADC_INPUT_COUNT; ++i) {
        if (MP_STATE_PORT(ra8_adc_owners[i]) != MP_OBJ_NULL) { return true; }
    }
    return false;
}

static fsp_err_t adc_wait_idle(void) {
    rt_tick_t start = rt_tick_get();
    do {
        adc_status_t status;
        fsp_err_t error = R_ADC_B_StatusGet(&g_adc0_ctrl, &status);
        if (error != FSP_SUCCESS) { return error; }
        if (g_adc0_ctrl.adc_state == ADC_B_CONVERTER_STATE_CALIBRATION_FAIL) { return FSP_ERR_INVALID_DATA; }
        if (status.state == ADC_STATE_IDLE) { return FSP_SUCCESS; }
        /* Calibration completion advances through the FSP CALEND interrupts.
         * Keep IRQs enabled and yield without running Python callbacks here.
         */
        rt_thread_mdelay(1);
    } while ((rt_tick_t)(rt_tick_get() - start) < rt_tick_from_millisecond(ADC_WAIT_MS));
    return FSP_ERR_TIMEOUT;
}

static fsp_err_t adc_restore(void) {
    if (!adc_taken) { return FSP_SUCCESS; }
    fsp_err_t error = FSP_SUCCESS;
    if (g_adc0_ctrl.opened) { error = R_ADC_B_Close(&g_adc0_ctrl); }
    if (error == FSP_SUCCESS && adc_was_open) {
        error = R_ADC_B_Open(&g_adc0_ctrl, &g_adc0_cfg);
        if (error == FSP_SUCCESS) { error = R_ADC_B_ScanCfg(&g_adc0_ctrl, &g_adc0_scan_cfg); }
    }
    adc_taken = false;
    adc_selected = -1;
    return error;
}

static fsp_err_t adc_take(void) {
    if (adc_taken) { return FSP_SUCCESS; }
    if (!g_ioport_ctrl.open || !g_adc0_cfg.p_extend) { return FSP_ERR_NOT_OPEN; }
    adc_was_open = g_adc0_ctrl.opened != 0;
    if (adc_was_open) {
        adc_status_t status;
        fsp_err_t error = R_ADC_B_StatusGet(&g_adc0_ctrl, &status);
        if (error != FSP_SUCCESS) { return error; }
        if (status.state != ADC_STATE_IDLE || g_adc0_ctrl.p_cfg != &g_adc0_cfg || R_ADC_B->ADTRGENR) { return FSP_ERR_IN_USE; }
        error = R_ADC_B_Close(&g_adc0_ctrl);
        if (error != FSP_SUCCESS) { return error; }
    }
    adc_cfg = g_adc0_cfg;
    adc_extend = *(const adc_b_extended_cfg_t *)g_adc0_cfg.p_extend;
    adc_extend.adc_b_converter_mode[0].mode = ADC_B_CONVERTER_MODE_SINGLE_SCAN;
    adc_extend.adc_b_converter_mode[1].mode = ADC_B_CONVERTER_MODE_SINGLE_SCAN;
    adc_cfg.p_extend = &adc_extend;
    adc_cfg.p_callback = NULL;
    adc_taken = true;
    fsp_err_t error = R_ADC_B_Open(&g_adc0_ctrl, &adc_cfg);
    if (error != FSP_SUCCESS) { adc_restore(); }
    return error;
}

static uint16_t adc_sample_states(mp_int_t sample_ns) {
    if (sample_ns < 0) { mp_raise_ValueError(MP_ERROR_TEXT("sample_ns must be nonnegative")); }
    const adc_b_extended_cfg_t *extend = g_adc0_cfg.p_extend;
    if (extend->clock_control_bits.source_selection != ADC_B_CLOCK_SOURCE_ADC) {
        mp_raise_OSError(MP_ENODEV);
    }
    uint32_t clock = R_BSP_SourceClockHzGet((fsp_priv_source_clock_t)(R_SYSTEM->ADCCKCR & 0x0fU));
    clock /= R_FSP_ClockDividerGet(R_SYSTEM->ADCCKDIVCR);
    clock /= extend->clock_control_bits.divider + 1U;
    if (!clock) { mp_raise_OSError(MP_EIO); }
    uint64_t states = ((uint64_t)sample_ns * clock + 999999999ULL) / 1000000000ULL;
    /* Retain the conservative 95-state minimum used by this board's FSP
     * configuration, including sample_ns=0 (use the board default).
     */
    if (states < 95) { states = 95; }
    if (states > 65535) { mp_raise_ValueError(MP_ERROR_TEXT("sample_ns exceeds ADC sampling range")); }
    return (uint16_t)states;
}

static fsp_err_t adc_select(machine_adc_obj_t *self) {
    if (adc_selected == self->index && adc_selected_states == self->sample_states) { return FSP_SUCCESS; }
    if (!g_adc0_ctrl.opened) {
        fsp_err_t open_error = R_ADC_B_Open(&g_adc0_ctrl, &adc_cfg);
        if (open_error != FSP_SUCCESS) { return open_error; }
    }
    uint8_t channel = adc_channels[self->index];
    unsigned converter = (BSP_FEATURE_ADC_B_UNIT_0_CHANNELS_MASK & (1ULL << channel)) ? 0 : 1;
    adc_selected = -1;
    memset(&adc_channel_cfg, 0, sizeof(adc_channel_cfg));
    adc_channel_cfg.channel_id = ADC_B_VIRTUAL_CHANNEL_0;
    adc_channel_cfg.channel_cfg_bits.group = 1;
    adc_channel_cfg.channel_cfg_bits.channel = channel;
    adc_channel_cfg.channel_control_c_bits.channel_data_format = ADC_B_DATA_FORMAT_16_BIT;
    adc_channel_cfg.channel_control_c_bits.data_is_unsigned = true;
    memset(&adc_group_cfg, 0, sizeof(adc_group_cfg));
    adc_group_cfg.scan_group_id = ADC_GROUP_ID_0;
    adc_group_cfg.converter_selection = (adc_b_unit_id_t)converter;
    adc_group_cfg.scan_group_enable = true;
    adc_group_cfg.virtual_channel_count = 1;
    adc_group_cfg.p_virtual_channels = adc_virtual_channels;
    /* Poll scan completion; retain CALEND interrupts for FSP calibration. */
    adc_extend.converter_selection_0 = converter << R_ADC_B0_ADSGCR0_SGADS0_Pos;
    adc_extend.sampling_state_table[0] = self->sample_states;
    fsp_err_t error = R_ADC_B_ScanCfg(&g_adc0_ctrl, &adc_scan_cfg);
    if (error == FSP_SUCCESS) { error = R_ADC_B_Calibrate(&g_adc0_ctrl, NULL); }
    if (error == FSP_SUCCESS) { error = adc_wait_idle(); }
    if (error == FSP_SUCCESS && g_adc0_ctrl.adc_state != ADC_B_CONVERTER_STATE_READY) { error = FSP_ERR_INVALID_DATA; }
    if (error == FSP_SUCCESS) {
        adc_selected = self->index;
        adc_selected_states = self->sample_states;
    } else {
        /* A timed-out calibration must not keep firing interrupts while a
         * later object changes the scan configuration. Reopen on the next read.
         */
        R_ADC_B_Close(&g_adc0_ctrl);
    }
    return error;
}

static void adc_start(machine_adc_obj_t *self) {
    if (self->active) { return; }
    uint16_t pin = adc_channels[self->index];
    ra8_machine_pin_require_available(pin);
    if (ra8_machine_pin_irq_owned(pin)) { mp_raise_OSError(MP_EBUSY); }
    adc_check(adc_take());
    self->saved_pin_cfg = R_PFS->PORT[0].PIN[pin].PmnPFS & ~R_PFS_PORT_PIN_PmnPFS_PIDR_Msk;
    fsp_err_t error = adc_pin_config(pin, IOPORT_CFG_ANALOG_ENABLE);
    if (error == FSP_SUCCESS) { error = adc_select(self); }
    if (error != FSP_SUCCESS) {
        adc_pin_config(pin, self->saved_pin_cfg);
        if (!adc_has_owners()) { adc_restore(); }
        adc_check(error);
    }
    self->active = true;
    MP_STATE_PORT(ra8_adc_owners[self->index]) = MP_OBJ_FROM_PTR(self);
}

static mp_obj_t machine_adc_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 1, 2, true);
    static const mp_arg_t allowed[] = {
        {MP_QSTR_sample_ns, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0}},
    };
    mp_map_t kw;
    mp_map_init_fixed_table(&kw, n_kw, args + n_args);
    mp_arg_val_t parsed[MP_ARRAY_SIZE(allowed)];
    mp_arg_parse_all(0, NULL, &kw, MP_ARRAY_SIZE(allowed), allowed, parsed);
    mp_int_t channel;
    if (n_args == 2) {
        bool valid = mp_obj_is_int(args[0]) && mp_obj_get_int(args[0]) == 0;
        if (mp_obj_is_str(args[0])) { valid = strcmp(mp_obj_str_get_str(args[0]), "adc0") == 0; }
        if (!valid) { mp_raise_ValueError(MP_ERROR_TEXT("ADC device is adc0")); }
        channel = mp_obj_get_int(args[1]);
    } else if (mp_obj_is_int(args[0])) {
        channel = mp_obj_get_int(args[0]);
    } else {
        channel = mp_hal_get_pin_obj(args[0]);
    }
    int index = -1;
    for (unsigned i = 0; i < ADC_INPUT_COUNT; ++i) {
        if (channel == adc_channels[i]) { index = i; break; }
    }
    if (index < 0) { mp_raise_ValueError(MP_ERROR_TEXT("ADC requires a U18 analog pin")); }
    machine_adc_obj_t *self = mp_obj_malloc(machine_adc_obj_t, type);
    self->index = index;
    self->active = false;
    self->sample_states = adc_sample_states(parsed[0].u_int);
    adc_start(self);
    return MP_OBJ_FROM_PTR(self);
}

static mp_obj_t machine_adc_init(size_t n_args, const mp_obj_t *args, mp_map_t *kw) {
    machine_adc_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    static const mp_arg_t allowed[] = {
        {MP_QSTR_sample_ns, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL}},
    };
    mp_arg_val_t parsed[MP_ARRAY_SIZE(allowed)];
    mp_arg_parse_all(n_args - 1, args + 1, kw, MP_ARRAY_SIZE(allowed), allowed, parsed);
    if (parsed[0].u_obj != MP_OBJ_NULL) { self->sample_states = adc_sample_states(mp_obj_get_int(parsed[0].u_obj)); }
    adc_start(self);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_KW(machine_adc_init_obj, 1, machine_adc_init);

static fsp_err_t adc_release(machine_adc_obj_t *self) {
    if (!self->active) { return FSP_SUCCESS; }
    fsp_err_t error = adc_pin_config(adc_channels[self->index], self->saved_pin_cfg);
    self->active = false;
    MP_STATE_PORT(ra8_adc_owners[self->index]) = MP_OBJ_NULL;
    adc_selected = -1;
    if (!adc_has_owners()) {
        fsp_err_t restore_error = adc_restore();
        if (error == FSP_SUCCESS) { error = restore_error; }
    }
    return error;
}

static mp_obj_t machine_adc_deinit(mp_obj_t self_in) {
    adc_check(adc_release(MP_OBJ_TO_PTR(self_in)));
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(machine_adc_deinit_obj, machine_adc_deinit);

void machine_adc_deinit_all(void) {
    for (unsigned i = 0; i < ADC_INPUT_COUNT; ++i) {
        mp_obj_t owner = MP_STATE_PORT(ra8_adc_owners[i]);
        if (owner != MP_OBJ_NULL) { adc_release(MP_OBJ_TO_PTR(owner)); }
    }
}

static mp_obj_t machine_adc_read(mp_obj_t self_in) {
    machine_adc_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if (!self->active) { mp_raise_OSError(MP_ENODEV); }
    fsp_err_t error = adc_select(self);
    if (error == FSP_SUCCESS) { error = R_ADC_B_ScanStart(&g_adc0_ctrl); }
    if (error == FSP_SUCCESS) {
        /* A short hardware delay ensures the scan request is accepted before
         * testing ADSR; otherwise an immediate idle read can return old data.
         */
        R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MICROSECONDS);
        error = adc_wait_idle();
    }
    uint16_t value = 0;
    if (error == FSP_SUCCESS) { error = R_ADC_B_Read(&g_adc0_ctrl, (adc_channel_t)adc_channels[self->index], &value); }
    if (error != FSP_SUCCESS) {
        if (g_adc0_ctrl.opened) { R_ADC_B_Close(&g_adc0_ctrl); }
        adc_selected = -1;
    }
    adc_check(error);
    return MP_OBJ_NEW_SMALL_INT(value);
}
static MP_DEFINE_CONST_FUN_OBJ_1(machine_adc_read_obj, machine_adc_read);

static void machine_adc_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    (void)kind;
    machine_adc_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "ADC(Pin('P0%02u'), bits=16, active=%u)", adc_channels[self->index], self->active);
}

static const mp_rom_map_elem_t machine_adc_locals_dict_table[] = {
    {MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&machine_adc_init_obj)},
    {MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&machine_adc_deinit_obj)},
    {MP_ROM_QSTR(MP_QSTR_read), MP_ROM_PTR(&machine_adc_read_obj)},
    {MP_ROM_QSTR(MP_QSTR_read_u16), MP_ROM_PTR(&machine_adc_read_obj)},
};
static MP_DEFINE_CONST_DICT(machine_adc_locals_dict, machine_adc_locals_dict_table);
MP_DEFINE_CONST_OBJ_TYPE(machine_adc_type, MP_QSTR_ADC, MP_TYPE_FLAG_NONE,
    make_new, machine_adc_make_new, print, machine_adc_print, locals_dict, &machine_adc_locals_dict);
#endif
