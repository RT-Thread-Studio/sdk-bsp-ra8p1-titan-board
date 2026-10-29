/* SPDX-License-Identifier: Apache-2.0 */
#include "py/runtime.h"
#include "titan_vision.h"
#include "titan_vision_guard.h"

#if MICROPY_PY_IMAGE
extern const mp_obj_module_t image_module;
#endif
#if MICROPY_PY_CSI && MICROPY_PY_CSI_NG
extern const mp_obj_module_t csi_module;
#endif
#if MICROPY_PY_GIF
extern const mp_obj_module_t gif_module;
#endif
#if MICROPY_PY_MJPEG
extern const mp_obj_module_t mjpeg_module;
#endif
#if MICROPY_PY_ML
extern const mp_obj_module_t ml_module;
#endif
#if MICROPY_PY_DISPLAY
extern const mp_obj_module_t display_module;
#endif

void titan_vision_module_require(mp_obj_t module)
{
    if (titan_vision_is_ready()) { return; }
    /* Compare the MRAM module objects, never dereference their globals. This
     * covers ml/tf and their u-prefix aliases without changing import rules.
     * Once loaded, the verified HyperRAM image remains valid until hard reset. */
    bool needs_vision = false;
#if MICROPY_PY_IMAGE
    needs_vision |= module == MP_OBJ_FROM_PTR(&image_module);
#endif
#if MICROPY_PY_CSI && MICROPY_PY_CSI_NG
    needs_vision |= module == MP_OBJ_FROM_PTR(&csi_module);
#endif
#if MICROPY_PY_GIF
    needs_vision |= module == MP_OBJ_FROM_PTR(&gif_module);
#endif
#if MICROPY_PY_MJPEG
    needs_vision |= module == MP_OBJ_FROM_PTR(&mjpeg_module);
#endif
#if MICROPY_PY_ML
    needs_vision |= module == MP_OBJ_FROM_PTR(&ml_module);
#endif
#if MICROPY_PY_DISPLAY
    needs_vision |= module == MP_OBJ_FROM_PTR(&display_module);
#endif
    if (needs_vision) {
        mp_raise_msg(&mp_type_OSError,
            MP_ERROR_TEXT("Vision unavailable: copy matching openmv-vision.bin to the USB storage drive"));
    }
}
