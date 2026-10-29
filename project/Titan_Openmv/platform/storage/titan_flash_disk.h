/* SPDX-License-Identifier: Apache-2.0 */
#ifndef TITAN_FLASH_DISK_H
#define TITAN_FLASH_DISK_H
#include <stdbool.h>

#define TITAN_FLASH_DISK_NAME "qflash"
#define TITAN_FLASH_DISK_SECTOR_SIZE 512U

/* Task-context initialization, explicitly called by the storage worker.
 * Registers an existing volume; never erases or formats it. */
int titan_flash_disk_init(void);
/* Metadata only: safe for USB callbacks; no mutex or flash transaction. */
bool titan_flash_disk_is_ready(void);
int titan_flash_disk_sync(void);

#endif
