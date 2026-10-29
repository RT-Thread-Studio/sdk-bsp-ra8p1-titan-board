#include "openmv/lib/imlib/apriltag_config.h"
/* The RTOS owns stack allocation; calculate the current thread's actual limit. */
static inline size_t ra8_apriltag_stack_avail(void) {
    volatile char marker;
    uintptr_t used = (uintptr_t)&marker - (uintptr_t)rt_thread_self()->stack_addr;
    return used > 2048 ? used - 2048 : 0;
}
#undef apriltag_stack_avail
#define apriltag_stack_avail() ra8_apriltag_stack_avail()
