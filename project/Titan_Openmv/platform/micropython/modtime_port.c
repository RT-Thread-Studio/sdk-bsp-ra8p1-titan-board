/* Included by upstream extmod/modtime.c. */
#include <time.h>
#include "shared/timeutils/timeutils.h"
static void mp_time_localtime_get(timeutils_struct_time_t *tm) {
    timeutils_seconds_since_epoch_to_struct_time(time(NULL), tm);
}
static mp_obj_t mp_time_time_get(void) {
    return mp_obj_new_int_from_ll(time(NULL));
}
static mp_obj_t mp_time_time_ns_get(void) {
    return mp_obj_new_int_from_ll((long long)time(NULL) * 1000000000LL);
}
