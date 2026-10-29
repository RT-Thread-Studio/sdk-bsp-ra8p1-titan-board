/* SPDX-License-Identifier: Apache-2.0 */
#include <stddef.h>
#include "py/runtime.h"
#include "py/mperrno.h"
#include "shared/runtime/softtimer.h"

typedef struct {
    mp_obj_base_t base;
    soft_timer_entry_t timer;
    mp_obj_t callback;
    int slot;
    bool active;
} ra8_timer_t;
MP_REGISTER_ROOT_POINTER(mp_obj_t ra8_timers[8]);

static mp_obj_t timer_deinit(mp_obj_t self_in) {
    ra8_timer_t *self = MP_OBJ_TO_PTR(self_in);
    if (self->active) { soft_timer_remove(&self->timer); self->active = false; }
    if (self->slot >= 0 && MP_STATE_VM(ra8_timers)[self->slot] == self_in) {
        MP_STATE_VM(ra8_timers)[self->slot] = MP_OBJ_NULL;
    }
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(timer_deinit_obj, timer_deinit);
static void timer_callback(soft_timer_entry_t *entry) {
    ra8_timer_t *self = (void *)((uint8_t *)entry - offsetof(ra8_timer_t, timer));
    if (self->callback != mp_const_none) { mp_sched_schedule(self->callback, MP_OBJ_FROM_PTR(self)); }
}
static mp_obj_t timer_init(size_t n_args, const mp_obj_t *args, mp_map_t *kw) {
    ra8_timer_t *self = MP_OBJ_TO_PTR(args[0]);
    enum { MODE, PERIOD, CALLBACK };
    static const mp_arg_t allowed[] = {
        { MP_QSTR_mode, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = SOFT_TIMER_MODE_PERIODIC} },
        { MP_QSTR_period, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 1000} },
        { MP_QSTR_callback, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = mp_const_none} },
    };
    mp_arg_val_t values[3];
    mp_arg_parse_all(n_args - 1, args + 1, kw, 3, allowed, values);
    if (values[PERIOD].u_int <= 0 || (values[MODE].u_int != SOFT_TIMER_MODE_PERIODIC && values[MODE].u_int != SOFT_TIMER_MODE_ONE_SHOT)) {
        mp_raise_ValueError(MP_ERROR_TEXT("invalid timer period or mode"));
    }
    if (values[CALLBACK].u_obj != mp_const_none && !mp_obj_is_callable(values[CALLBACK].u_obj)) {
        mp_raise_TypeError(MP_ERROR_TEXT("callback must be callable"));
    }
    timer_deinit(args[0]);
    if (self->slot < 0) {
        for (int i = 0; i < 8; i++) { if (MP_STATE_VM(ra8_timers)[i] == MP_OBJ_NULL) { self->slot = i; break; } }
    }
    if (self->slot < 0 || MP_STATE_VM(ra8_timers)[self->slot] != MP_OBJ_NULL) { mp_raise_OSError(MP_EBUSY); }
    self->callback = values[CALLBACK].u_obj;
    MP_STATE_VM(ra8_timers)[self->slot] = args[0];
    soft_timer_static_init(&self->timer, values[MODE].u_int, values[PERIOD].u_int, timer_callback);
    self->active = true;
    soft_timer_insert(&self->timer, values[PERIOD].u_int);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_KW(timer_init_obj, 1, timer_init);
static mp_obj_t timer_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 0, 1, true);
    int slot = n_args ? mp_obj_get_int(args[0]) : -1;
    if (slot < -1 || slot >= 8) { mp_raise_ValueError(MP_ERROR_TEXT("timer id must be -1 or 0..7")); }
    ra8_timer_t *self = mp_obj_malloc(ra8_timer_t, type);
    self->slot = slot; self->active = false; self->callback = mp_const_none;
    if (n_kw) {
        mp_map_t kw; mp_map_init_fixed_table(&kw, n_kw, args + n_args);
        mp_obj_t obj = MP_OBJ_FROM_PTR(self); timer_init(1, &obj, &kw);
    }
    return MP_OBJ_FROM_PTR(self);
}
void ra8_timers_deinit_all(void) {
    for (int i = 0; i < 8; i++) { if (MP_STATE_VM(ra8_timers)[i] != MP_OBJ_NULL) { timer_deinit(MP_STATE_VM(ra8_timers)[i]); } }
}
static const mp_rom_map_elem_t timer_dict[] = {
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&timer_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&timer_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_PERIODIC), MP_ROM_INT(SOFT_TIMER_MODE_PERIODIC) },
    { MP_ROM_QSTR(MP_QSTR_ONE_SHOT), MP_ROM_INT(SOFT_TIMER_MODE_ONE_SHOT) },
};
static MP_DEFINE_CONST_DICT(timer_locals, timer_dict);
MP_DEFINE_CONST_OBJ_TYPE(machine_timer_type, MP_QSTR_Timer, MP_TYPE_FLAG_NONE,
    make_new, timer_make_new, locals_dict, &timer_locals);
