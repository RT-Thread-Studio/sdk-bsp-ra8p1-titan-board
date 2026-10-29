/* SPDX-License-Identifier: Apache-2.0 */
#include <stddef.h>
#include "py/runtime.h"
#include "ulab/code/ndarray.h"

#if !defined(MODULE_ULAB_ENABLED) || !MODULE_ULAB_ENABLED
#error OpenMV image-to-tensor conversion requires MODULE_ULAB_ENABLED
#endif
_Static_assert(ULAB_MAX_DIMS == 4, "OpenMV ML requires four-dimensional ulab arrays");
_Static_assert(sizeof(((ndarray_obj_t *)0)->shape) == 4 * sizeof(size_t), "ndarray shape ABI");
_Static_assert(sizeof(((ndarray_obj_t *)0)->strides) == 4 * sizeof(int32_t), "ndarray stride ABI");
_Static_assert(sizeof(ndarray_obj_t) == 52 && offsetof(ndarray_obj_t, array) == 44,
               "Unexpected ARM32 four-dimensional ndarray ABI");
