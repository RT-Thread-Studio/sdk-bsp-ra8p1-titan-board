/* SPDX-License-Identifier: Apache-2.0 */
#ifndef TITAN_OPENMV_STORAGE_H
#define TITAN_OPENMV_STORAGE_H
#include <rtthread.h>
#include <stdint.h>

/* The Kconfig-selected SD card or QSPI volume is the writable root. Only
 * Flash may be provisioned automatically; SD cards are never autoformatted. */
extern const char titan_storage_backend_name[];
int titan_storage_mount(rt_int32_t timeout_ms);

/* USB exposes the selected medium automatically, with local FAT mounted as
 * on official OpenMV. These optional controls load/eject the host medium only;
 * they never grant exclusive ownership. Eject on the PC before disabling.
 * Avoid concurrent host/Python writes to the same FAT filesystem. */
int titan_storage_usb_export(void);
int titan_storage_usb_start(void);
int titan_storage_usb_release(void);
int titan_storage_usb_enabled(void);
int titan_storage_usb_writable(void);
/* USB-task metadata/control: no DFS lock, media wait or FAT refresh. Accepted
 * USB writes are already durable before their asynchronous completion. */
int titan_storage_usb_snapshot(uint32_t *blocks, uint16_t *block_size);
int titan_storage_usb_media_set(int enabled);
int titan_storage_usb_capacity(uint32_t *blocks, uint16_t *block_size);
int titan_storage_usb_read(uint32_t lba, uint32_t offset, void *buffer, uint32_t size);
int titan_storage_usb_write(uint32_t lba, uint32_t offset, const void *buffer, uint32_t size);
int titan_storage_usb_sync(void);

#endif
