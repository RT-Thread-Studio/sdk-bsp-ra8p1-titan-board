/* SPDX-License-Identifier: Apache-2.0 */
#ifndef TITAN_ML_MEMORY_H
#define TITAN_ML_MEMORY_H
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
/* The returned model storage remains owned and traced by MicroPython GC. */
void *titan_ml_model_alloc(size_t size);
/* Always use the registered HyperRAM GC areas, including small persistent state.
 * Returns NULL on OOM/locked GC and preserves automatic-collection policy. */
void *titan_ml_hyperram_alloc(size_t size);
/* Prefer an SRAM arena, reclaim dead objects once before falling back to HyperRAM.
 * Preserves GC ownership and gc.disable(); returns NULL on OOM/locked GC. */
void *titan_ml_arena_alloc(size_t size);
#ifdef __cplusplus
}
#endif
#endif
