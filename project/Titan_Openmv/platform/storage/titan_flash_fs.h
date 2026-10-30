/* SPDX-License-Identifier: Apache-2.0 */
#ifndef TITAN_FLASH_FS_H
#define TITAN_FLASH_FS_H
#include <stdbool.h>

/* Caller holds dfs_lock and keeps MSC unpublished (or otherwise quiescent).
 * These APIs require an unmounted 6 MiB block slice, not the whole NOR chip.
 * Probe returns 0 with a definitive needs_format value, or negative errno.
 * Format performs one operation only; the caller owns first-boot policy. */
int titan_elm_probe(const char *device, bool *needs_format);
int titan_elm_format(const char *device);

#endif
