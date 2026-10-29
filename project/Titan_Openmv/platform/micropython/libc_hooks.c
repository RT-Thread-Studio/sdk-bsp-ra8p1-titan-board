/* SPDX-License-Identifier: Apache-2.0 */
#ifdef __ARMCC_VERSION
#include <rt_misc.h>
#include "bsp_api.h"
extern uint8_t g_heap[BSP_CFG_HEAP_BYTES];
extern uint8_t g_main_stack[];
__asm(".global __use_two_region_memory");
__value_in_regs struct __initial_stackheap __user_initial_stackheap(
    unsigned heap_base, unsigned stack_base, unsigned heap_limit, unsigned stack_limit) {
    struct __initial_stackheap result;
    result.heap_base = (unsigned)g_heap;
    result.heap_limit = (unsigned)(g_heap + BSP_CFG_HEAP_BYTES);
    result.stack_base = (unsigned)(g_main_stack + BSP_CFG_STACK_MAIN_BYTES);
    result.stack_limit = (unsigned)g_main_stack;
    return result;
}
#endif
