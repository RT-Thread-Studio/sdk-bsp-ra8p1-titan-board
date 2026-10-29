/* SPDX-License-Identifier: Apache-2.0 */
#include <stdint.h>
#include <limits.h>
#include <rtthread.h>
#include "bsp_api.h"
#include "ra8_memory.h"
#include "titan_memory.h"

/* FSP POST_C has already initialized HyperRAM and its C runtime sections.
 * Do not repeat the HyperRAM command sequence or allocate VIN/GC/UMA memory. */
#ifdef RT_USING_MEMHEAP_AS_HEAP
static struct rt_memheap hyperram_heap;
static int titan_hyperram_heap_init(void)
{
    return rt_memheap_init(&hyperram_heap, "hyperram", (void *)RA8_RT_HYPERRAM_START,
                          RA8_RT_HYPERRAM_SIZE);
}
INIT_BOARD_EXPORT(titan_hyperram_heap_init);
#endif

static int region_contains(uintptr_t start, size_t length, uintptr_t p, size_t size)
{
    return p >= start && size <= length && p - start <= length - size;
}

int titan_memory_is_bus_accessible(const void *address, size_t size)
{
    uintptr_t p = (uintptr_t)address;
    return region_contains(0x22000000UL, 0x001d4000UL, p, size) ||
           region_contains(RA8_HYPERRAM_START, RA8_HYPERRAM_SIZE, p, size) ||
           region_contains(0x02000000UL, 0x00100000UL, p, size);
}

static void cache_op(const void *address, size_t size, unsigned operation)
{
    uintptr_t p = (uintptr_t)address;
    if (!size || p > UINTPTR_MAX - size || size > INT32_MAX - 64) {
        return;
    }
    if (!titan_memory_is_bus_accessible(address, size)) {
        return;
    }
    uintptr_t start = p & ~(uintptr_t)31;
    uintptr_t end = (p + size + 31) & ~(uintptr_t)31;
    __DSB();
#if BSP_CFG_DCACHE_ENABLED
    if (operation == 0) {
        SCB_CleanDCache_by_Addr((void *)start, (int32_t)(end - start));
    } else if (operation == 1) {
        SCB_InvalidateDCache_by_Addr((void *)start, (int32_t)(end - start));
    } else {
        SCB_CleanInvalidateDCache_by_Addr((void *)start, (int32_t)(end - start));
    }
#else
    (void)start;
    (void)end;
    (void)operation;
#endif
    __DSB();
}

void titan_cache_clean(const void *address, size_t size) { cache_op(address, size, 0); }
void titan_cache_invalidate(void *address, size_t size) { cache_op(address, size, 1); }
void titan_cache_clean_invalidate(void *address, size_t size) { cache_op(address, size, 2); }
