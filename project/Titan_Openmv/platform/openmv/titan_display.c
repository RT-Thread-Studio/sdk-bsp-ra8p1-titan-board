/* SPDX-License-Identifier: Apache-2.0 */
/* OpenMV RGBDisplay backend for the Titan Board RGB565 panel. */
#include <string.h>

#include "board_config.h"
#include "py/mperrno.h"
#include "py/runtime.h"
#include "py_display.h"
#include "titan_lcd.h"

#if MICROPY_PY_DISPLAY

enum {
    TITAN_DISPLAY_WIDTH = 800,
    TITAN_DISPLAY_HEIGHT = 480,
    TITAN_DISPLAY_REFRESH = 56,
};

typedef struct {
    py_display_obj_t display;
    uint32_t generation;
} titan_display_obj_t;

/* Hardware owns static framebuffers, never a Python object. An integer token
 * lets GC finalize an abandoned display without permanently rooting it, and
 * prevents an old object's __del__ from closing a subsequently opened display.
 */
static uint32_t display_generation;
static uint32_t active_generation;

static void display_check_result(int result) {
    if (result < 0) {
        mp_raise_OSError(-result);
    }
}

static void display_require_owner(py_display_obj_t *display) {
    titan_display_obj_t *self = (titan_display_obj_t *) display;
    if (!self->generation || self->generation != active_generation || !titan_lcd_is_open()) {
        mp_raise_OSError(MP_ENODEV);
    }
}

static void display_restore_backlight(py_display_obj_t *self) {
    if (!self->display_on) {
        display_check_result(titan_lcd_set_backlight(self->intensity));
        self->display_on = true;
    }
}

static void display_deinit(py_display_obj_t *display) {
    titan_display_obj_t *self = (titan_display_obj_t *) display;
    if (self->generation && self->generation == active_generation) {
        /* Keep ownership on failure so an explicit deinit can be retried. */
        display_check_result(titan_lcd_close());
        active_generation = 0;
        self->generation = 0;
        display->display_on = false;
    }
}

void titan_display_deinit_all(void) {
    /* Called before the VM heap is reset. There is no GC pointer to clear. */
    titan_lcd_deinit_all();
    active_generation = 0;
}

static void display_write(py_display_obj_t *self, image_t *src_img,
                          int dst_x_start, int dst_y_start, float x_scale, float y_scale,
                          rectangle_t *roi, int rgb_channel, int alpha,
                          const uint16_t *color_palette, const uint8_t *alpha_palette,
                          image_hint_t hint) {
    display_require_owner(self);
    uint16_t *buffer = titan_lcd_draw_buffer();
    if (buffer == NULL) {
        mp_raise_OSError(MP_EBUSY);
    }

    image_t dst_img = {
        .w = TITAN_DISPLAY_WIDTH,
        .h = TITAN_DISPLAY_HEIGHT,
        .pixfmt = PIXFORMAT_RGB565,
        .data = (uint8_t *) buffer,
    };
    /* BLACK_BACKGROUND writes all covered pixels (including alpha blending)
     * against black. Clear only the uncovered borders, avoiding a second full
     * write of the image area to the shared HyperRAM on every frame. */
    point_t p0, p1;
    imlib_draw_image_get_bounds(&dst_img, src_img, dst_x_start, dst_y_start,
                               x_scale, y_scale, roi, alpha, alpha_palette, hint, &p0, &p1);
    if (p0.x < 0) {
        memset(buffer, 0, TITAN_DISPLAY_WIDTH * TITAN_DISPLAY_HEIGHT * sizeof(*buffer));
    } else {
        memset(buffer, 0, p0.y * TITAN_DISPLAY_WIDTH * sizeof(*buffer));
        memset(buffer + p1.y * TITAN_DISPLAY_WIDTH, 0,
               (TITAN_DISPLAY_HEIGHT - p1.y) * TITAN_DISPLAY_WIDTH * sizeof(*buffer));
        for (int y = p0.y; y < p1.y; ++y) {
            uint16_t *row = buffer + y * TITAN_DISPLAY_WIDTH;
            memset(row, 0, p0.x * sizeof(*buffer));
            memset(row + p1.x, 0, (TITAN_DISPLAY_WIDTH - p1.x) * sizeof(*buffer));
        }
        imlib_draw_image(&dst_img, src_img, dst_x_start, dst_y_start,
                         x_scale, y_scale, roi, rgb_channel, alpha, color_palette,
                         alpha_palette, hint | IMAGE_HINT_BLACK_BACKGROUND,
                         NULL, NULL, NULL, NULL);
    }

    /* The driver applies a completed-buffer memory barrier and waits for a safe
     * swap. FSP maps these static buffers as non-cacheable. No camera/Python
     * buffer is retained after this synchronous call returns.
     */
    display_check_result(titan_lcd_present());
    display_restore_backlight(self);
}

static void display_clear(py_display_obj_t *self, bool display_off) {
    display_require_owner(self);
    if (display_off) {
        display_check_result(titan_lcd_set_backlight(0));
        self->display_on = false;
        return;
    }

    uint16_t *buffer = titan_lcd_draw_buffer();
    if (buffer == NULL) {
        mp_raise_OSError(MP_EBUSY);
    }
    memset(buffer, 0, TITAN_DISPLAY_WIDTH * TITAN_DISPLAY_HEIGHT * sizeof(*buffer));
    display_check_result(titan_lcd_present());
    display_restore_backlight(self);
}

static void display_set_backlight(py_display_obj_t *self, uint32_t intensity) {
    display_require_owner(self);
    display_check_result(titan_lcd_set_backlight(intensity));
    self->display_on = true;
}

static mp_obj_t display_make_new(const mp_obj_type_t *type, size_t n_args,
                                size_t n_kw, const mp_obj_t *all_args) {
    enum {
        ARG_framesize, ARG_refresh, ARG_display_on, ARG_triple_buffer,
        ARG_portrait, ARG_channel, ARG_controller, ARG_backlight,
    };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_framesize, MP_ARG_INT, {.u_int = DISPLAY_RESOLUTION_FWVGA} },
        { MP_QSTR_refresh, MP_ARG_INT | MP_ARG_KW_ONLY, {.u_int = TITAN_DISPLAY_REFRESH} },
        { MP_QSTR_display_on, MP_ARG_BOOL | MP_ARG_KW_ONLY, {.u_bool = true} },
        { MP_QSTR_triple_buffer, MP_ARG_BOOL | MP_ARG_KW_ONLY, {.u_bool = false} },
        { MP_QSTR_portrait, MP_ARG_BOOL | MP_ARG_KW_ONLY, {.u_bool = false} },
        { MP_QSTR_channel, MP_ARG_INT | MP_ARG_KW_ONLY, {.u_int = 0} },
        { MP_QSTR_controller, MP_ARG_OBJ | MP_ARG_KW_ONLY, {.u_rom_obj = MP_ROM_NONE} },
        { MP_QSTR_backlight, MP_ARG_OBJ | MP_ARG_KW_ONLY, {.u_rom_obj = MP_ROM_NONE} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    if (args[ARG_framesize].u_int != DISPLAY_RESOLUTION_FWVGA || args[ARG_portrait].u_bool) {
        mp_raise_ValueError(MP_ERROR_TEXT("RGB display supports 800x480 landscape only"));
    }
    if (args[ARG_refresh].u_int != TITAN_DISPLAY_REFRESH) {
        mp_raise_ValueError(MP_ERROR_TEXT("RGB display refresh must be 56 Hz"));
    }
    if (args[ARG_triple_buffer].u_bool) {
        mp_raise_ValueError(MP_ERROR_TEXT("RGB display uses two framebuffers"));
    }
    if (args[ARG_channel].u_int != 0 || args[ARG_controller].u_obj != mp_const_none ||
        args[ARG_backlight].u_obj != mp_const_none) {
        mp_raise_ValueError(MP_ERROR_TEXT("RGB display uses the built-in panel and backlight"));
    }
    if (titan_lcd_is_open()) {
        mp_raise_OSError(MP_EBUSY);
    }

    titan_display_obj_t *self = mp_obj_malloc_with_finaliser(titan_display_obj_t, type);
    self->display = (py_display_obj_t) {
        .base = {type},
        .width = TITAN_DISPLAY_WIDTH,
        .height = TITAN_DISPLAY_HEIGHT,
        .framesize = DISPLAY_RESOLUTION_FWVGA,
        .refresh = TITAN_DISPLAY_REFRESH,
        .intensity = 100,
        .display_on = args[ARG_display_on].u_bool,
        .controller = mp_const_none,
        .bl_controller = mp_const_none,
    };
    self->generation = 0;

    display_check_result(titan_lcd_open());
    if (++display_generation == 0) {
        ++display_generation;
    }
    self->generation = display_generation;
    active_generation = self->generation;
    int result = titan_lcd_set_backlight(self->display.display_on ? self->display.intensity : 0);
    if (result < 0) {
        /* This object has not escaped to Python yet. On a failed rollback keep
         * its token so its finalizer can retry; the low-level driver retains
         * pins/buffers, and the VM-reset barrier handles persistent failures.
         */
        if (titan_lcd_close() == 0) {
            active_generation = 0;
            self->generation = 0;
            self->display.display_on = false;
        }
        display_check_result(result);
    }
    return MP_OBJ_FROM_PTR(self);
}

static const py_display_p_t titan_display_protocol = {
    .deinit = display_deinit,
    .clear = display_clear,
    .write = display_write,
    .set_backlight = display_set_backlight,
};

MP_DEFINE_CONST_OBJ_TYPE(
    py_rgb_display_type,
    MP_QSTR_RGBDisplay,
    MP_TYPE_FLAG_NONE,
    make_new, display_make_new,
    protocol, &titan_display_protocol,
    locals_dict, &py_display_locals_dict
    );

#else
void titan_display_deinit_all(void) {
}
#endif // MICROPY_PY_DISPLAY
