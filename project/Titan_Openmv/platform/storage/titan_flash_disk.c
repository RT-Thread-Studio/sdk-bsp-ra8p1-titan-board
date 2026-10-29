/* SPDX-License-Identifier: Apache-2.0 */
#include <rtthread.h>
#include <rtdevice.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include "titan_flash.h"
#include "titan_flash_disk.h"

#define DISK_SECTORS (TITAN_FLASH_STORAGE_SIZE / TITAN_FLASH_DISK_SECTOR_SIZE)
#define SECTORS_PER_ERASE (TITAN_FLASH_ERASE_SIZE / TITAN_FLASH_DISK_SECTOR_SIZE)
#define PROGRAM_BYTES TITAN_FLASH_PROGRAM_SIZE
#define VERIFY_BYTES 256U

_Static_assert(TITAN_FLASH_ERASE_SIZE == 4096U, "Review NOR block staging for a different erase size");
_Static_assert(PROGRAM_BYTES == 8U, "Dirty units must match the manual program command capacity");
_Static_assert(TITAN_FLASH_ERASE_SIZE % (PROGRAM_BYTES * 8U) == 0U,
               "Program bitmap must cover the complete erase unit");
_Static_assert(TITAN_FLASH_STORAGE_SIZE % TITAN_FLASH_ERASE_SIZE == 0U, "NOR volume must end on an erase boundary");
_Static_assert(TITAN_FLASH_STORAGE_OFFSET % TITAN_FLASH_ERASE_SIZE == 0U, "NOR volume must start on an erase boundary");

static struct rt_device flash_disk;
static struct rt_mutex flash_mutex;
static bool mutex_initialized;
static volatile bool registered;
static uint8_t erase_image[TITAN_FLASH_ERASE_SIZE] __attribute__((aligned(32)));
static uint8_t verify_image[VERIFY_BYTES] __attribute__((aligned(32)));
static const struct rt_device_blk_geometry geometry = {
    .bytes_per_sector = TITAN_FLASH_DISK_SECTOR_SIZE,
    .sector_count = DISK_SECTORS,
    .block_size = TITAN_FLASH_ERASE_SIZE,
};

bool titan_flash_disk_is_ready(void)
{
    /* The raw predicate is also metadata-only, including its latched faults. */
    return registered && titan_flash_is_ready();
}

static int disk_lock(void)
{
    if (!registered) { return -ENODEV; }
    return rt_mutex_take(&flash_mutex, RT_WAITING_FOREVER);
}

static rt_ssize_t failed_io(int result, rt_size_t completed)
{
    rt_set_errno(result < 0 ? result : -EIO);
    return (rt_ssize_t)completed;
}

static int check_range(rt_off_t sector, const void *buffer, rt_size_t count)
{
    if (sector < 0 || (uint64_t)sector > DISK_SECTORS ||
        (uint64_t)count > DISK_SECTORS - (uint64_t)sector) {
        return -EINVAL;
    }
    if (count && (!buffer || (uintptr_t)buffer > UINTPTR_MAX -
                                (size_t)count * TITAN_FLASH_DISK_SECTOR_SIZE)) {
        return -EINVAL;
    }
    return RT_EOK;
}

static rt_ssize_t flash_disk_read(rt_device_t device, rt_off_t sector, void *buffer, rt_size_t count)
{
    (void)device;
    int result = check_range(sector, buffer, count);
    if (result != RT_EOK) { return failed_io(result, 0); }
    if (!count) { return 0; }
    result = disk_lock();
    if (result != RT_EOK) { return failed_io(result, 0); }
    rt_size_t completed = 0;
    while (completed < count) {
        uint32_t current = (uint32_t)sector + (uint32_t)completed;
        rt_size_t chunk = SECTORS_PER_ERASE - current % SECTORS_PER_ERASE;
        if (chunk > count - completed) { chunk = count - completed; }
        result = titan_flash_read(current * TITAN_FLASH_DISK_SECTOR_SIZE,
                                  (uint8_t *)buffer + completed * TITAN_FLASH_DISK_SECTOR_SIZE,
                                  chunk * TITAN_FLASH_DISK_SECTOR_SIZE);
        if (result != RT_EOK) { break; }
        completed += chunk;
    }
    rt_mutex_release(&flash_mutex);
    return result == RT_EOK ? (rt_ssize_t)completed : failed_io(result, completed);
}

/* Commit one erase-unit image. No acknowledged write remains only in RAM.
 * The mutex is held across the old-image read, erase, programming and verify.
 * Track 8-byte commands individually so sparse writes do not reprogram neighbours. */
static int write_erase_unit(uint32_t base, uint32_t within, const uint8_t *data, size_t size)
{
    uint8_t dirty[TITAN_FLASH_ERASE_SIZE / PROGRAM_BYTES / 8U] = {0};
    int result = titan_flash_read(base, erase_image, sizeof(erase_image));
    if (result != RT_EOK) { return result; }
    bool changed = false;
    bool erase_needed = false;
    for (size_t i = 0; i < size; ++i) {
        uint8_t before = erase_image[within + i];
        uint8_t after = data[i];
        if (before != after) {
            changed = true;
            uint32_t unit = (within + (uint32_t)i) / PROGRAM_BYTES;
            dirty[unit / 8U] |= (uint8_t)(1U << (unit % 8U));
            if ((before & after) != after) { erase_needed = true; }
        }
    }
    if (!changed) { return RT_EOK; }
    memcpy(erase_image + within, data, size);
    if (erase_needed) {
        result = titan_flash_erase_sector(base);
        if (result != RT_EOK) { return result; }
    }
    for (uint32_t offset = 0; offset < TITAN_FLASH_ERASE_SIZE; offset += PROGRAM_BYTES) {
        uint32_t unit = offset / PROGRAM_BYTES;
        bool program = (dirty[unit / 8U] & (1U << (unit % 8U))) != 0;
        if (erase_needed) {
            program = false;
            for (unsigned i = 0; i < PROGRAM_BYTES; ++i) {
                if (erase_image[offset + i] != 0xffU) { program = true; break; }
            }
        }
        if (program) {
            result = titan_flash_program(base + offset, erase_image + offset, PROGRAM_BYTES);
            if (result != RT_EOK) { return result; }
        }
    }
    result = titan_flash_sync();
    if (result != RT_EOK) { return result; }
    for (uint32_t offset = 0; offset < TITAN_FLASH_ERASE_SIZE; offset += VERIFY_BYTES) {
        result = titan_flash_read(base + offset, verify_image, sizeof(verify_image));
        if (result != RT_EOK) { return result; }
        if (memcmp(verify_image, erase_image + offset, sizeof(verify_image))) { return -EIO; }
    }
    return RT_EOK;
}

static rt_ssize_t flash_disk_write(rt_device_t device, rt_off_t sector, const void *buffer, rt_size_t count)
{
    (void)device;
    int result = check_range(sector, buffer, count);
    if (result != RT_EOK) { return failed_io(result, 0); }
    if (!count) { return 0; }
    result = disk_lock();
    if (result != RT_EOK) { return failed_io(result, 0); }
    rt_size_t completed = 0;
    while (completed < count) {
        uint32_t current = (uint32_t)sector + (uint32_t)completed;
        uint32_t first = current - current % SECTORS_PER_ERASE;
        rt_size_t chunk = SECTORS_PER_ERASE - current % SECTORS_PER_ERASE;
        if (chunk > count - completed) { chunk = count - completed; }
        result = write_erase_unit(first * TITAN_FLASH_DISK_SECTOR_SIZE,
                                  (current - first) * TITAN_FLASH_DISK_SECTOR_SIZE,
                                  (const uint8_t *)buffer + completed * TITAN_FLASH_DISK_SECTOR_SIZE,
                                  chunk * TITAN_FLASH_DISK_SECTOR_SIZE);
        if (result != RT_EOK) { break; }
        completed += chunk;
    }
    rt_mutex_release(&flash_mutex);
    return result == RT_EOK ? (rt_ssize_t)completed : failed_io(result, completed);
}

int titan_flash_disk_sync(void)
{
    int result = disk_lock();
    if (result != RT_EOK) { return result; }
    result = titan_flash_sync();
    rt_mutex_release(&flash_mutex);
    return result;
}

static rt_err_t flash_disk_control(rt_device_t device, int command, void *arguments)
{
    (void)device;
    switch (command) {
    case RT_DEVICE_CTRL_BLK_GETGEOME:
        if (!arguments) { return -EINVAL; }
        memcpy(arguments, &geometry, sizeof(geometry));
        return RT_EOK;
    case RT_DEVICE_CTRL_BLK_SYNC:
        return titan_flash_disk_sync();
    case RT_DEVICE_CTRL_BLK_ERASE:
        /* FatFs TRIM is advisory. Erasing a 512-byte range would also destroy
         * its live neighbours in the same NOR erase unit; safely ignore it. */
        return RT_EOK;
    default:
        return -ENOSYS;
    }
}

#ifdef RT_USING_DEVICE_OPS
static const struct rt_device_ops flash_disk_ops = {
    .read = flash_disk_read,
    .write = flash_disk_write,
    .control = flash_disk_control,
};
#endif

int titan_flash_disk_init(void)
{
    /* Initialization is task-only. Serialize creation of the static mutex
     * without making any flash call or waiting while scheduling is disabled. */
    int result = RT_EOK;
    rt_enter_critical();
    if (!mutex_initialized) {
        result = rt_mutex_init(&flash_mutex, "qflash", RT_IPC_FLAG_PRIO);
        if (result == RT_EOK) { mutex_initialized = true; }
    }
    rt_exit_critical();
    if (result != RT_EOK) { return result; }
    result = rt_mutex_take(&flash_mutex, RT_WAITING_FOREVER);
    if (result != RT_EOK) { return result; }
    if (registered && titan_flash_is_ready()) { goto done; }
    rt_device_t existing = rt_device_find(TITAN_FLASH_DISK_NAME);
    if (existing && existing != &flash_disk) { result = -EBUSY; goto done; }
    result = titan_flash_init();
    if (result != RT_EOK || registered) { goto done; }
    flash_disk.type = RT_Device_Class_Block;
#ifdef RT_USING_DEVICE_OPS
    flash_disk.ops = &flash_disk_ops;
#else
    flash_disk.read = flash_disk_read;
    flash_disk.write = flash_disk_write;
    flash_disk.control = flash_disk_control;
#endif
    /* FAT and MSC share the volume, so do not use STANDALONE/exclusive-open. */
    result = rt_device_register(&flash_disk, TITAN_FLASH_DISK_NAME, RT_DEVICE_FLAG_RDWR);
    if (result == RT_EOK) { registered = true; }
done:
    rt_mutex_release(&flash_mutex);
    return result;
}
