/* SPDX-License-Identifier: Apache-2.0 */
/* Reuse OpenMV's display argument handling and expose explicit deinit(). */
#include "py/runtime.h"

#define py_display_locals_dict py_display_upstream_locals_dict
#include "../../ThirdParty/openmv/modules/py_display.c"
#undef py_display_locals_dict

#if MICROPY_PY_DISPLAY
static const mp_rom_map_elem_t titan_display_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__),      MP_ROM_QSTR(MP_QSTR_display) },
    { MP_ROM_QSTR(MP_QSTR___del__),       MP_ROM_PTR(&py_display_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit),        MP_ROM_PTR(&py_display_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_width),         MP_ROM_PTR(&py_display_width_obj) },
    { MP_ROM_QSTR(MP_QSTR_height),        MP_ROM_PTR(&py_display_height_obj) },
    { MP_ROM_QSTR(MP_QSTR_triple_buffer), MP_ROM_PTR(&py_display_triple_buffer_obj) },
    { MP_ROM_QSTR(MP_QSTR_bgr),           MP_ROM_PTR(&py_display_bgr_obj) },
    { MP_ROM_QSTR(MP_QSTR_byte_swap),     MP_ROM_PTR(&py_display_byte_swap_obj) },
    { MP_ROM_QSTR(MP_QSTR_framesize),     MP_ROM_PTR(&py_display_framesize_obj) },
    { MP_ROM_QSTR(MP_QSTR_refresh),       MP_ROM_PTR(&py_display_refresh_obj) },
    { MP_ROM_QSTR(MP_QSTR_clear),         MP_ROM_PTR(&py_display_clear_obj) },
    { MP_ROM_QSTR(MP_QSTR_backlight),     MP_ROM_PTR(&py_display_backlight_obj) },
    { MP_ROM_QSTR(MP_QSTR_write),         MP_ROM_PTR(&py_display_write_obj) },
    { MP_ROM_QSTR(MP_QSTR_bus_write),     MP_ROM_PTR(&py_display_bus_write_obj) },
    { MP_ROM_QSTR(MP_QSTR_bus_read),      MP_ROM_PTR(&py_display_bus_read_obj) },
    { MP_ROM_QSTR(MP_QSTR_ioctl),         MP_ROM_PTR(&py_display_ioctl_obj) },
};
MP_DEFINE_CONST_DICT(py_display_locals_dict, titan_display_locals_dict_table);
#endif
