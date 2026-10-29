/* SPDX-License-Identifier: Apache-2.0 */
#ifndef TITAN_MEMORY_H
#define TITAN_MEMORY_H
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
/* DMA buffers must own full 32-byte cache lines. TCM is CPU-only. */
void titan_cache_clean(const void *address, size_t size);
void titan_cache_invalidate(void *address, size_t size);
void titan_cache_clean_invalidate(void *address, size_t size);
int titan_memory_is_bus_accessible(const void *address, size_t size);
#ifdef __cplusplus
}
#endif
#endif
