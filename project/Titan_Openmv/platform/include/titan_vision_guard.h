/* SPDX-License-Identifier: Apache-2.0 */
#ifndef TITAN_VISION_GUARD_H
#define TITAN_VISION_GUARD_H
#include "py/obj.h"

/* Called after built-in alias resolution, before any module initializer. */
void titan_vision_module_require(mp_obj_t module);

#endif
