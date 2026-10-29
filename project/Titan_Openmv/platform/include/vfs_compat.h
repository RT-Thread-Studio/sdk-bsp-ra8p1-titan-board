/* RT-Thread's Arm libc stat.st_dev is a device pointer. Its integer identity
 * fits in mp_uint_t on this 32-bit target; keep the conversion in the port. */
#include "py/obj.h"
#define mp_obj_new_int_from_uint(value) mp_obj_new_int_from_uint((mp_uint_t)(uintptr_t)(value))
