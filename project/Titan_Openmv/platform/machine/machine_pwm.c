/* SPDX-License-Identifier: MIT
 * Titan Board U18 GPT outputs. Each GPT owns one selected A/B output at a time.
 */
#include <string.h>
#include <hal_data.h>
#include <r_gpt_cfg.h>
#include <rthw.h>
#include "py/runtime.h"
#include "py/mperrno.h"
#include "modmachine.h"
#include "mphalport.h"
#include "machine_pwm.h"
#include "titan_lcd.h"

#if MICROPY_PY_MACHINE_PWM
#define PWM_MAX_FREQ (1000000U)
#define PWM_TIMER_COUNT (4)
typedef struct {
    uint16_t pin;
    uint8_t timer_index;
    uint8_t output;
} pwm_pin_t;
static const pwm_pin_t pwm_pins[] = {
    {BSP_IO_PORT_06_PIN_01, 0, GPT_IO_PIN_GTIOCA}, /* U18-7, GPT6A */
    {BSP_IO_PORT_06_PIN_02, 1, GPT_IO_PIN_GTIOCB}, /* U18-35, GPT7B */
    {BSP_IO_PORT_06_PIN_03, 1, GPT_IO_PIN_GTIOCA}, /* U18-33, GPT7A */
    {BSP_IO_PORT_06_PIN_05, 2, GPT_IO_PIN_GTIOCA}, /* U18-32, GPT8A */
    {BSP_IO_PORT_06_PIN_04, 2, GPT_IO_PIN_GTIOCB}, /* U18-12, GPT8B */
    {BSP_IO_PORT_07_PIN_15, 3, GPT_IO_PIN_GTIOCA}, /* U18-38, GPT12A */
    {BSP_IO_PORT_07_PIN_14, 3, GPT_IO_PIN_GTIOCB}, /* U18-36, GPT12B */
};
static gpt_instance_ctrl_t *const pwm_ctrls[PWM_TIMER_COUNT] = {
    &g_timer6_ctrl, &g_timer7_ctrl, &g_timer8_ctrl, &g_timer12_ctrl,
};
static const timer_cfg_t *const pwm_defaults[PWM_TIMER_COUNT] = {
    &g_timer6_cfg, &g_timer7_cfg, &g_timer8_cfg, &g_timer12_cfg,
};
/* Other pins selected as GPT1 by the generated BSP. The same GPT signal is
 * broadcast to every pin with its mux enabled, so park these routes before
 * starting a U18 waveform. Existing GPIO/other-peripheral use is left intact.
 */
static const uint16_t pwm_aliases[PWM_TIMER_COUNT][2] = {
    {0xffff, 0xffff},
    {BSP_IO_PORT_03_PIN_03, BSP_IO_PORT_10_PIN_07}, /* GPT7B LCD_BL, GPT7A PHY_RESET */
    {BSP_IO_PORT_01_PIN_06, 0xffff},              /* GPT8B U18 CS3 */
    {BSP_IO_PORT_05_PIN_01, 0xffff},              /* GPT12A camera FSIN */
};
typedef struct {
    mp_obj_base_t base;
    uint32_t requested_freq;
    uint32_t duty_value;
    uint32_t saved_pin_cfg;
    uint32_t saved_alias_cfg[2];
    uint8_t pin_index;
    uint8_t alias_mask;
    bool duty_is_ns;
    bool active;
    bool was_open;
} machine_pwm_obj_t;

MP_REGISTER_ROOT_POINTER(mp_obj_t ra8_pwm_owners[4]);
static timer_cfg_t pwm_cfgs[PWM_TIMER_COUNT];
static gpt_extended_cfg_t pwm_extends[PWM_TIMER_COUNT];

static void pwm_check(fsp_err_t error) {
    if (error != FSP_SUCCESS) { mp_raise_OSError(MP_EIO); }
}

static fsp_err_t pwm_pin_config(uint16_t pin, uint32_t config) {
    rt_base_t level = rt_hw_interrupt_disable();
    fsp_err_t error = R_IOPORT_PinCfg(&g_ioport_ctrl, (bsp_io_port_pin_t)pin, config);
    rt_hw_interrupt_enable(level);
    return error;
}

bool ra8_pwm_pin_owned(uint16_t pin) {
    for (unsigned i = 0; i < PWM_TIMER_COUNT; ++i) {
        mp_obj_t owner = MP_STATE_PORT(ra8_pwm_owners[i]);
        if (owner == MP_OBJ_NULL) { continue; }
        machine_pwm_obj_t *self = MP_OBJ_TO_PTR(owner);
        if (pwm_pins[self->pin_index].pin == pin) { return true; }
        for (unsigned j = 0; j < 2; ++j) {
            if ((self->alias_mask & (1U << j)) && pwm_aliases[i][j] == pin) { return true; }
        }
    }
    return false;
}

static void pwm_require_active(machine_pwm_obj_t *self) {
    if (!self->active) { mp_raise_OSError(MP_ENODEV); }
}

static uint32_t pwm_source_clock_hz(void) {
    /* Match FSP's GPT clock selection, including the PCLKD bypass. Read the
     * source before Open(), when the timer instance may not be initialized.
     */
#if BSP_PERIPHERAL_GPT_GTCLK_PRESENT && !GPT_CFG_GPTCLK_BYPASS
    uint32_t divisor = R_FSP_ClockDividerGet(R_SYSTEM->GPTCKDIVCR_b.GPTCKDIV);
    uint32_t clock = R_BSP_SourceClockHzGet((fsp_priv_source_clock_t)R_SYSTEM->GPTCKCR_b.GPTCKSEL) / divisor;
#else
    uint32_t clock = R_FSP_SystemClockHzGet(FSP_PRIV_CLOCK_PCLKD);
#endif
    if (!clock) { mp_raise_OSError(MP_EIO); }
    return clock;
}

static void pwm_calculate(machine_pwm_obj_t *self, uint32_t freq, uint32_t duty,
                          bool duty_is_ns, timer_cfg_t *cfg) {
    uint32_t clock = pwm_source_clock_hz();
    if (!freq || freq > PWM_MAX_FREQ) {
        mp_raise_ValueError(MP_ERROR_TEXT("PWM frequency must be 1..1000000 Hz"));
    }
    /* All RA8P1 GPT channels are 32-bit. GPT source / 1 provides full resolution
     * throughout the exposed 1 Hz..1 MHz range, without changing prescalers.
     */
    uint32_t counts = ((uint64_t)clock + freq / 2U) / freq;
    if (counts < 2) { mp_raise_ValueError(MP_ERROR_TEXT("PWM frequency exceeds timer clock")); }
    uint64_t pulse;
    if (duty_is_ns) {
        if ((uint64_t)duty * clock > (uint64_t)counts * 1000000000ULL) {
            mp_raise_ValueError(MP_ERROR_TEXT("duty_ns exceeds PWM period"));
        }
        pulse = ((uint64_t)duty * clock + 500000000ULL) / 1000000000ULL;
    } else {
        if (duty > 65535) { mp_raise_ValueError(MP_ERROR_TEXT("duty_u16 must be 0..65535")); }
        pulse = ((uint64_t)counts * duty + 32767U) / 65535U;
    }
    unsigned timer = pwm_pins[self->pin_index].timer_index;
    *cfg = *pwm_defaults[timer];
    cfg->mode = TIMER_MODE_PWM;
    cfg->source_div = TIMER_SOURCE_DIV_1;
    cfg->period_counts = counts;
    cfg->duty_cycle_counts = (uint32_t)pulse;
    cfg->p_extend = &pwm_extends[timer];
    cfg->p_callback = NULL;
    cfg->p_context = NULL;
    cfg->cycle_end_irq = FSP_INVALID_VECTOR;
    cfg->cycle_end_ipl = BSP_IRQ_DISABLED;
}

static fsp_err_t pwm_restore(machine_pwm_obj_t *self) {
    const pwm_pin_t *pin = &pwm_pins[self->pin_index];
    gpt_instance_ctrl_t *ctrl = pwm_ctrls[pin->timer_index];
    fsp_err_t error = FSP_SUCCESS;
    if (ctrl->open) {
        error = R_GPT_Stop(ctrl);
        fsp_err_t close_error = R_GPT_Close(ctrl);
        if (error == FSP_SUCCESS) { error = close_error; }
    }
    /* Restore the original SDK instance only if it was open on acquisition. */
    if (!ctrl->open && self->was_open) {
        fsp_err_t open_error = R_GPT_Open(ctrl, pwm_defaults[pin->timer_index]);
        if (error == FSP_SUCCESS) { error = open_error; }
    }
    fsp_err_t pin_error = pwm_pin_config(pin->pin, self->saved_pin_cfg);
    if (error == FSP_SUCCESS) { error = pin_error; }
    for (unsigned i = 0; i < 2; ++i) {
        if (self->alias_mask & (1U << i)) {
            fsp_err_t alias_error = pwm_pin_config(pwm_aliases[pin->timer_index][i], self->saved_alias_cfg[i]);
            if (error == FSP_SUCCESS) { error = alias_error; }
        }
    }
    self->alias_mask = 0;
    return error;
}

static void pwm_apply(machine_pwm_obj_t *self, uint32_t freq, uint32_t duty, bool duty_is_ns) {
    const pwm_pin_t *pin = &pwm_pins[self->pin_index];
    unsigned timer = pin->timer_index;
    gpt_instance_ctrl_t *ctrl = pwm_ctrls[timer];
    if (titan_lcd_timer_owned(pwm_defaults[timer]->channel)) { mp_raise_OSError(MP_EBUSY); }
    timer_cfg_t config;
    pwm_calculate(self, freq, duty, duty_is_ns, &config);
    bool first = !self->active;
    if (first) {
        ra8_machine_pin_require_available(pin->pin);
        if (ra8_machine_pin_irq_owned(pin->pin) || MP_STATE_PORT(ra8_pwm_owners[timer]) != MP_OBJ_NULL) {
            mp_raise_OSError(MP_EBUSY);
        }
        if (!g_ioport_ctrl.open || !pwm_defaults[timer]->p_extend) { mp_raise_OSError(MP_ENODEV); }
        if (ctrl->open) {
            timer_status_t status;
            pwm_check(R_GPT_StatusGet(ctrl, &status));
            if (status.state == TIMER_STATE_COUNTING || ctrl->p_cfg != pwm_defaults[timer]) { mp_raise_OSError(MP_EBUSY); }
        }
        self->saved_pin_cfg = R_PFS->PORT[pin->pin >> 8].PIN[pin->pin & 15].PmnPFS & ~R_PFS_PORT_PIN_PmnPFS_PIDR_Msk;
        self->was_open = ctrl->open != 0;
        self->alias_mask = 0;
        for (unsigned i = 0; i < 2; ++i) {
            uint16_t alias = pwm_aliases[timer][i];
            if (alias == 0xffff) { continue; }
            uint32_t old_cfg = R_PFS->PORT[alias >> 8].PIN[alias & 15].PmnPFS;
            uint32_t mux_mask = IOPORT_CFG_PERIPHERAL_PIN | R_PFS_PORT_PIN_PmnPFS_PSEL_Msk;
            if ((old_cfg & mux_mask) == (IOPORT_CFG_PERIPHERAL_PIN | IOPORT_PERIPHERAL_GPT1)) {
                ra8_machine_pin_require_available(alias);
                if (ra8_machine_pin_irq_owned(alias)) { mp_raise_OSError(MP_EBUSY); }
                self->saved_alias_cfg[i] = old_cfg;
                self->alias_mask |= 1U << i;
            }
        }
        pwm_extends[timer] = *(const gpt_extended_cfg_t *)pwm_defaults[timer]->p_extend;
        /* Never enable the other output of this GPT: it may be routed to a
         * board device or another U18 function in the generated pin table.
         */
        pwm_extends[timer].gtioca.output_enabled = pin->output == GPT_IO_PIN_GTIOCA;
        pwm_extends[timer].gtiocb.output_enabled = pin->output == GPT_IO_PIN_GTIOCB;
        pwm_extends[timer].gtioca.stop_level = GPT_PIN_LEVEL_LOW;
        pwm_extends[timer].gtiocb.stop_level = GPT_PIN_LEVEL_LOW;
        pwm_extends[timer].gtior_setting.gtior = 0;
        pwm_extends[timer].p_pwm_cfg = NULL;
        MP_STATE_PORT(ra8_pwm_owners[timer]) = MP_OBJ_FROM_PTR(self);
    }
    bool reconfigure = first || config.period_counts != pwm_cfgs[timer].period_counts;
    fsp_err_t error = FSP_SUCCESS;
    if (first) {
        for (unsigned i = 0; i < 2 && error == FSP_SUCCESS; ++i) {
            if (!(self->alias_mask & (1U << i))) { continue; }
            uint32_t saved = self->saved_alias_cfg[i];
            uint32_t parked = IOPORT_CFG_PORT_DIRECTION_OUTPUT |
                              ((saved & R_PFS_PORT_PIN_PmnPFS_PIDR_Msk) ? IOPORT_CFG_PORT_OUTPUT_HIGH : IOPORT_CFG_PORT_OUTPUT_LOW);
            self->saved_alias_cfg[i] &= ~R_PFS_PORT_PIN_PmnPFS_PIDR_Msk;
            error = pwm_pin_config(pwm_aliases[timer][i], parked);
        }
    }
    if (reconfigure) {
        if (error == FSP_SUCCESS && ctrl->open) {
            error = R_GPT_Stop(ctrl);
            if (error == FSP_SUCCESS) { error = R_GPT_Close(ctrl); }
        }
        pwm_cfgs[timer] = config; /* FSP retains this pointer beyond Open(). */
        if (error == FSP_SUCCESS) { error = R_GPT_Open(ctrl, &pwm_cfgs[timer]); }
        if (error == FSP_SUCCESS) {
            error = pwm_pin_config(pin->pin, IOPORT_CFG_PERIPHERAL_PIN | IOPORT_PERIPHERAL_GPT1);
        }
    }
    if (error == FSP_SUCCESS) { error = R_GPT_DutyCycleSet(ctrl, config.duty_cycle_counts, pin->output); }
    if (error == FSP_SUCCESS && reconfigure) { error = R_GPT_Start(ctrl); }
    if (error != FSP_SUCCESS) {
        pwm_restore(self);
        self->active = false;
        MP_STATE_PORT(ra8_pwm_owners[timer]) = MP_OBJ_NULL;
        pwm_check(error);
    }
    pwm_cfgs[timer].duty_cycle_counts = config.duty_cycle_counts;
    self->requested_freq = freq;
    self->duty_value = duty;
    self->duty_is_ns = duty_is_ns;
    self->active = true;
}

static uint32_t pwm_unsigned(mp_obj_t value, uint32_t max, const char *name) {
    mp_int_t number = mp_obj_get_int(value);
    if (number < 0 || (uint32_t)number > max) {
        mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("invalid %s"), name);
    }
    return number;
}

static void pwm_init_args(machine_pwm_obj_t *self, size_t n_args, const mp_obj_t *pos, mp_map_t *kw, bool legacy) {
    enum { ARG_freq, ARG_duty_u16, ARG_duty_ns, ARG_duty, ARG_channel };
    static const mp_arg_t allowed[] = {
        {MP_QSTR_freq, MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL}},
        {MP_QSTR_duty_u16, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL}},
        {MP_QSTR_duty_ns, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL}},
        {MP_QSTR_duty, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL}},
        {MP_QSTR_channel, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0}},
    };
    mp_obj_t legacy_freq = MP_OBJ_NULL, legacy_duty = MP_OBJ_NULL;
    if (legacy && n_args) {
        if (n_args > 3 || mp_obj_get_int(pos[0]) != 0) {
            mp_raise_ValueError(MP_ERROR_TEXT("RT PWM channel must be 0; select A/B with Pin"));
        }
        if (n_args > 1) { legacy_freq = pos[1]; }
        if (n_args > 2) { legacy_duty = pos[2]; }
        n_args = 0;
    }
    mp_arg_val_t parsed[MP_ARRAY_SIZE(allowed)];
    mp_arg_parse_all(n_args, pos, kw, MP_ARRAY_SIZE(allowed), allowed, parsed);
    if (parsed[ARG_channel].u_int != 0) {
        mp_raise_ValueError(MP_ERROR_TEXT("RT PWM channel must be 0; select A/B with Pin"));
    }
    if (legacy_freq != MP_OBJ_NULL) {
        if (parsed[ARG_freq].u_obj != MP_OBJ_NULL) { mp_raise_TypeError(MP_ERROR_TEXT("duplicate freq")); }
        parsed[ARG_freq].u_obj = legacy_freq;
    }
    if (legacy_duty != MP_OBJ_NULL) {
        if (parsed[ARG_duty].u_obj != MP_OBJ_NULL) { mp_raise_TypeError(MP_ERROR_TEXT("duplicate duty")); }
        parsed[ARG_duty].u_obj = legacy_duty;
    }
    uint32_t freq = self->requested_freq, duty = self->duty_value;
    bool is_ns = self->duty_is_ns;
    if (parsed[ARG_freq].u_obj != MP_OBJ_NULL) { freq = pwm_unsigned(parsed[ARG_freq].u_obj, PWM_MAX_FREQ, "freq"); }
    unsigned specified = 0;
    if (parsed[ARG_duty_u16].u_obj != MP_OBJ_NULL) {
        ++specified; duty = pwm_unsigned(parsed[ARG_duty_u16].u_obj, 65535, "duty_u16"); is_ns = false;
    }
    if (parsed[ARG_duty_ns].u_obj != MP_OBJ_NULL) {
        ++specified; duty = pwm_unsigned(parsed[ARG_duty_ns].u_obj, 1000000000, "duty_ns"); is_ns = true;
    }
    if (parsed[ARG_duty].u_obj != MP_OBJ_NULL) {
        ++specified; duty = pwm_unsigned(parsed[ARG_duty].u_obj, 255, "duty") * 257U; is_ns = false;
    }
    if (specified > 1) { mp_raise_TypeError(MP_ERROR_TEXT("choose one duty format")); }
    pwm_apply(self, freq, duty, is_ns);
}

static mp_obj_t machine_pwm_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 1, 4, true);
    bool legacy = mp_obj_is_int(args[0]);
    int timer_id = legacy ? mp_obj_get_int(args[0]) : -1;
    if (mp_obj_is_str(args[0])) {
        const char *name = mp_obj_str_get_str(args[0]);
        if (strncmp(name, "pwm", 3) == 0) {
            legacy = true;
            if (strcmp(name, "pwm6") == 0) { timer_id = 6; }
            if (strcmp(name, "pwm7") == 0) { timer_id = 7; }
            if (strcmp(name, "pwm8") == 0) { timer_id = 8; }
            if (strcmp(name, "pwm12") == 0) { timer_id = 12; }
        }
    }
    int index = -1;
    uint16_t pin = legacy ? 0 : mp_hal_get_pin_obj(args[0]);
    for (unsigned i = 0; i < MP_ARRAY_SIZE(pwm_pins); ++i) {
        if (legacy ? timer_id == pwm_defaults[pwm_pins[i].timer_index]->channel : pin == pwm_pins[i].pin) {
            index = i; break;
        }
    }
    if (index < 0) { mp_raise_ValueError(MP_ERROR_TEXT("PWM requires U18 GPT6/7/8/12 pin")); }
    machine_pwm_obj_t *self = mp_obj_malloc(machine_pwm_obj_t, type);
    self->pin_index = index;
    self->active = false;
    self->requested_freq = 1000;
    self->duty_value = 0;
    self->duty_is_ns = false;
    mp_map_t kw;
    mp_map_init_fixed_table(&kw, n_kw, args + n_args);
    pwm_init_args(self, n_args - 1, args + 1, &kw, legacy);
    return MP_OBJ_FROM_PTR(self);
}

static mp_obj_t machine_pwm_init(size_t n_args, const mp_obj_t *args, mp_map_t *kw) {
    pwm_init_args(MP_OBJ_TO_PTR(args[0]), n_args - 1, args + 1, kw, false);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_KW(machine_pwm_init_obj, 1, machine_pwm_init);

static fsp_err_t pwm_release(machine_pwm_obj_t *self) {
    if (!self->active) { return FSP_SUCCESS; }
    fsp_err_t error = pwm_restore(self);
    self->active = false;
    MP_STATE_PORT(ra8_pwm_owners[pwm_pins[self->pin_index].timer_index]) = MP_OBJ_NULL;
    return error;
}
static mp_obj_t machine_pwm_deinit(mp_obj_t self_in) {
    pwm_check(pwm_release(MP_OBJ_TO_PTR(self_in)));
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(machine_pwm_deinit_obj, machine_pwm_deinit);

void machine_pwm_deinit_all(void) {
    for (unsigned i = 0; i < PWM_TIMER_COUNT; ++i) {
        mp_obj_t owner = MP_STATE_PORT(ra8_pwm_owners[i]);
        if (owner != MP_OBJ_NULL) { pwm_release(MP_OBJ_TO_PTR(owner)); }
    }
}

static void machine_pwm_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    (void)kind;
    machine_pwm_obj_t *self = MP_OBJ_TO_PTR(self_in);
    uint16_t pin = pwm_pins[self->pin_index].pin;
    mp_printf(print, "PWM(Pin('P%u%02u'), freq=%u, %s=%u, active=%u)", pin >> 8, pin & 15,
              self->requested_freq, self->duty_is_ns ? "duty_ns" : "duty_u16", self->duty_value, self->active);
}

static mp_obj_t machine_pwm_freq(size_t n_args, const mp_obj_t *args) {
    machine_pwm_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    pwm_require_active(self);
    if (n_args == 1) {
        const timer_cfg_t *cfg = &pwm_cfgs[pwm_pins[self->pin_index].timer_index];
        uint32_t clock = pwm_source_clock_hz();
        return mp_obj_new_int_from_uint(((uint64_t)clock + cfg->period_counts / 2) / cfg->period_counts);
    }
    pwm_apply(self, pwm_unsigned(args[1], PWM_MAX_FREQ, "freq"), self->duty_value, self->duty_is_ns);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_pwm_freq_obj, 1, 2, machine_pwm_freq);

static mp_obj_t pwm_duty_access(size_t n_args, const mp_obj_t *args, unsigned format) {
    machine_pwm_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    pwm_require_active(self);
    if (n_args == 1) {
        const timer_cfg_t *cfg = &pwm_cfgs[pwm_pins[self->pin_index].timer_index];
        uint64_t pulse = cfg->duty_cycle_counts;
        uint32_t value;
        if (format == 1) {
            uint32_t clock = pwm_source_clock_hz();
            value = (pulse * 1000000000ULL + clock / 2) / clock;
        } else {
            unsigned scale = format == 0 ? 65535U : 255U;
            value = (pulse * scale + cfg->period_counts / 2) / cfg->period_counts;
        }
        return mp_obj_new_int_from_uint(value);
    }
    uint32_t value = pwm_unsigned(args[1], format == 1 ? 1000000000U : (format == 0 ? 65535U : 255U), "duty");
    if (format == 2) { value *= 257U; }
    pwm_apply(self, self->requested_freq, value, format == 1);
    return mp_const_none;
}
static mp_obj_t machine_pwm_duty_u16(size_t n, const mp_obj_t *args) { return pwm_duty_access(n, args, 0); }
static mp_obj_t machine_pwm_duty_ns(size_t n, const mp_obj_t *args) { return pwm_duty_access(n, args, 1); }
static mp_obj_t machine_pwm_duty(size_t n, const mp_obj_t *args) { return pwm_duty_access(n, args, 2); }
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_pwm_duty_u16_obj, 1, 2, machine_pwm_duty_u16);
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_pwm_duty_ns_obj, 1, 2, machine_pwm_duty_ns);
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_pwm_duty_obj, 1, 2, machine_pwm_duty);

static const mp_rom_map_elem_t machine_pwm_locals_dict_table[] = {
    {MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&machine_pwm_init_obj)},
    {MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&machine_pwm_deinit_obj)},
    {MP_ROM_QSTR(MP_QSTR_freq), MP_ROM_PTR(&machine_pwm_freq_obj)},
    {MP_ROM_QSTR(MP_QSTR_duty_u16), MP_ROM_PTR(&machine_pwm_duty_u16_obj)},
    {MP_ROM_QSTR(MP_QSTR_duty_ns), MP_ROM_PTR(&machine_pwm_duty_ns_obj)},
    {MP_ROM_QSTR(MP_QSTR_duty), MP_ROM_PTR(&machine_pwm_duty_obj)},
};
static MP_DEFINE_CONST_DICT(machine_pwm_locals_dict, machine_pwm_locals_dict_table);
MP_DEFINE_CONST_OBJ_TYPE(machine_pwm_type, MP_QSTR_PWM, MP_TYPE_FLAG_NONE,
    make_new, machine_pwm_make_new, print, machine_pwm_print, locals_dict, &machine_pwm_locals_dict);
#endif
