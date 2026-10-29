/* SPDX-License-Identifier: Apache-2.0 */
#include <stdint.h>
#include "py/runtime.h"
#include "py/gc.h"
#include "py/nlr.h"
#include "ra8_memory.h"
#include "titan_ml_memory.h"

#if !MICROPY_GC_SPLIT_HEAP || MICROPY_GC_SPLIT_HEAP_AUTO || MICROPY_PY_THREAD
#error "Review model allocation for this GC configuration"
#endif

/* Preserve SRAM for mutable tensors. Small models can still benefit from
 * SRAM weights; both MediaPipe face models are above this boundary. */
#define TITAN_ML_LARGE_MODEL_BYTES (128U * 1024U)
#define TITAN_ML_SRAM_START (0x22000000UL)
#define TITAN_ML_SRAM_END   (0x221d4000UL)

typedef struct {
    nlr_jump_callback_node_t node;
    mp_state_mem_area_t *last_free_area;
    uint16_t auto_collect_enabled;
} model_alloc_guard_t;

static void model_alloc_restore(void *arg)
{
    model_alloc_guard_t *guard = arg;
    MP_STATE_MEM(gc_last_free_area) = guard->last_free_area;
    MP_STATE_MEM(gc_auto_collect_enabled) = guard->auto_collect_enabled;
}

static int model_area_is_hyperram(const mp_state_mem_area_t *area)
{
    uintptr_t start = (uintptr_t)area->gc_pool_start;
    uintptr_t end = (uintptr_t)area->gc_pool_end;
    return start >= RA8_HYPERRAM_START && end >= start &&
           end <= (uintptr_t)RA8_HYPERRAM_START + RA8_HYPERRAM_SIZE;
}

static void *model_alloc_hyperram_try(size_t size)
{
    mp_state_mem_area_t *area = &MP_STATE_MEM(area);
    while (area && !model_area_is_hyperram(area)) { area = area->next; }
    if (!area) { return NULL; }
    /* gc_alloc scans forward. Refuse an unexpected later non-HyperRAM area,
     * rather than allow a model to consume a newly added SRAM/TCM pool. */
    for (mp_state_mem_area_t *next = area; next; next = next->next) {
        if (!model_area_is_hyperram(next)) { return NULL; }
    }
    model_alloc_guard_t guard = {
        .last_free_area = MP_STATE_MEM(gc_last_free_area),
        .auto_collect_enabled = MP_STATE_MEM(gc_auto_collect_enabled),
    };
    nlr_push_jump_callback(&guard.node, model_alloc_restore);
    /* The pinned gc_alloc cannot collect or invoke a finalizer with automatic
     * collection disabled. Restore before any collection, exception or return. */
    MP_STATE_MEM(gc_auto_collect_enabled) = 0;
    MP_STATE_MEM(gc_last_free_area) = area;
    void *result = m_malloc_maybe(size);
    nlr_pop_jump_callback(true);
    return result;
}

void *titan_ml_hyperram_alloc(size_t size)
{
    if (!size || size > SIZE_MAX - MICROPY_BYTES_PER_GC_BLOCK) { return NULL; }
    if (gc_is_locked()) { return NULL; }

    /* Match gc_alloc: honor gc.disable(), collect before allocation when a
     * configured threshold is reached, and collect at most once on failure. */
    int collected = !MP_STATE_MEM(gc_auto_collect_enabled);
#if MICROPY_GC_ALLOC_THRESHOLD
    if (!collected &&
        MP_STATE_MEM(gc_alloc_amount) >= MP_STATE_MEM(gc_alloc_threshold)) {
        gc_collect();
        collected = 1;
    }
#endif
    void *result = model_alloc_hyperram_try(size);
    if (!result && !collected && MP_STATE_MEM(gc_auto_collect_enabled)) {
        gc_collect();
        result = model_alloc_hyperram_try(size);
    }
    return result;
}


void *titan_ml_model_alloc(size_t size)
{
    if (size > SIZE_MAX - MICROPY_BYTES_PER_GC_BLOCK) { return NULL; }
    if (size < TITAN_ML_LARGE_MODEL_BYTES) { return m_malloc_maybe(size); }
    return titan_ml_hyperram_alloc(size);
}

static void *model_alloc_sram_try(size_t size)
{
    if (size > TITAN_ML_SRAM_END - TITAN_ML_SRAM_START) { return NULL; }
    model_alloc_guard_t guard = {
        .last_free_area = MP_STATE_MEM(gc_last_free_area),
        .auto_collect_enabled = MP_STATE_MEM(gc_auto_collect_enabled),
    };
    nlr_push_jump_callback(&guard.node, model_alloc_restore);
    /* A previous small allocation can leave the split-heap cursor in HyperRAM.
     * Scan from the first area, with collection disabled only for this attempt.
     * Do not unlink GC areas or change their allocation tables. */
    MP_STATE_MEM(gc_auto_collect_enabled) = 0;
    MP_STATE_MEM(gc_last_free_area) = &MP_STATE_MEM(area);
    void *result = m_malloc_maybe(size);
    uintptr_t address = (uintptr_t)result;
    if (result && (address < TITAN_ML_SRAM_START || address >= TITAN_ML_SRAM_END ||
                   size > TITAN_ML_SRAM_END - address)) {
        /* gc_alloc falls through to later areas without collecting dead SRAM.
         * Release that tentative allocation before the controlled GC retry. */
        m_free(result);
        result = NULL;
    }
    nlr_pop_jump_callback(true);
    return result;
}

void *titan_ml_arena_alloc(size_t size)
{
    if (!size || size > SIZE_MAX - MICROPY_BYTES_PER_GC_BLOCK || gc_is_locked()) { return NULL; }
    int collected = !MP_STATE_MEM(gc_auto_collect_enabled);
#if MICROPY_GC_ALLOC_THRESHOLD
    if (!collected &&
        MP_STATE_MEM(gc_alloc_amount) >= MP_STATE_MEM(gc_alloc_threshold)) {
        gc_collect();
        collected = 1;
    }
#endif
    void *result = model_alloc_sram_try(size);
    if (!result && !collected && MP_STATE_MEM(gc_auto_collect_enabled)) {
        gc_collect();
        result = model_alloc_sram_try(size);
    }
    /* The fallback uses the same GC-owned, bus-accessible HyperRAM as weights.
     * The try helper avoids a second collection after the SRAM retry. */
    return result ? result : model_alloc_hyperram_try(size);
}
