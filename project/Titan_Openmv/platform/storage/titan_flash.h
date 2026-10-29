/* SPDX-License-Identifier: Apache-2.0 */
#ifndef TITAN_FLASH_H
#define TITAN_FLASH_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define TITAN_FLASH_STORAGE_OFFSET UINT32_C(0x00200000)
#define TITAN_FLASH_STORAGE_SIZE   UINT32_C(0x00600000)
#define TITAN_FLASH_ERASE_SIZE     UINT32_C(4096)
/* Payload capacity of one OSPI manual command, within a NOR program page. */
#define TITAN_FLASH_PROGRAM_SIZE   UINT32_C(8)

/* All offsets are relative to the storage volume: the first 2 MiB is never
 * addressed. Success is zero, failure is a negative errno. The block layer
 * serializes all calls and owns the lock across each 4 KiB read/modify/write.
 * Call only from task context with interrupts enabled; is_ready does no I/O.
 * A hardware failure is latched until reset, preventing uncertain reuse. */
int titan_flash_init(void);
bool titan_flash_is_ready(void);
int titan_flash_read(uint32_t volume_offset, void *buffer, size_t length);
int titan_flash_program(uint32_t volume_offset, const void *buffer, size_t length);
int titan_flash_erase_sector(uint32_t volume_offset);
int titan_flash_sync(void);

#endif
