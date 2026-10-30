/* SPDX-License-Identifier: Apache-2.0 */
#include <rtthread.h>
#include <errno.h>
#include <stdbool.h>
#include <rtdevice.h>
#include <dfs_fs.h>
#include <dfs_file.h>
#include "titan_storage.h"
#if defined(BSP_OPENMV_STORAGE_SD)
#include <drivers/mmcsd_core.h>
#include "titan_sdhi.h"
#define STORAGE_DEVICE "sd"
const char titan_storage_backend_name[] = "sd";
static int storage_backend_ready(void) { return titan_sdhi_is_ready(); }
static int storage_backend_writable(void) { return !titan_sdhi_write_protected(); }
static int storage_backend_sync(void) { return titan_sdhi_sync(); }
static int storage_backend_init(rt_int32_t timeout_ms)
{
    if (storage_backend_ready()) { return RT_EOK; }
    int result = titan_sdhi_probe();
    if (result != RT_EOK) { return result; }
    /* A timeout leaves the original enumeration in flight. A second change
     * event could detach its live card/device objects instead of retrying. */
    result = mmcsd_wait_cd_changed(timeout_ms > 0 ? rt_tick_from_millisecond(timeout_ms) : 0);
    if (result == MMCSD_HOST_PLUGED)
    {
        titan_sdhi_probe_complete(1);
        return storage_backend_ready() ? RT_EOK : -ENODEV;
    }
    if (result == MMCSD_HOST_UNPLUGED)
    {
        titan_sdhi_probe_complete(0);
        return -ENODEV;
    }
    return -EAGAIN;
}
#elif defined(BSP_OPENMV_STORAGE_FLASH)
#include "titan_flash_disk.h"
#include "titan_flash_fs.h"
#define STORAGE_DEVICE "qflash"
const char titan_storage_backend_name[] = "spi-flash";
static int storage_backend_ready(void) { return titan_flash_disk_is_ready(); }
static int storage_backend_writable(void) { return storage_backend_ready(); }
static int storage_backend_sync(void) { return titan_flash_disk_sync(); }
static int storage_backend_init(rt_int32_t timeout_ms)
{
    (void)timeout_ms;
    return titan_flash_disk_init();
}
#else
#error "Select an OpenMV storage medium in Kconfig"
#endif

static char mounted_device[RT_NAME_MAX];
static char refresh_device[RT_NAME_MAX];
static rt_bool_t initial_check_done;
#if defined(BSP_OPENMV_STORAGE_FLASH)
static rt_bool_t format_attempted;
#endif
static int last_mount_error;
static int last_init_error;
/* Published only after provisioning makes its one-time format decision.
 * Early USB callbacks never wait behind media or filesystem operations. */
static rt_device_t volatile usb_device;
static struct rt_device_blk_geometry usb_geometry;
static volatile rt_bool_t usb_media_enabled = RT_TRUE;
static rt_bool_t host_write_pending;
static unsigned volume_open_files;
static rt_bool_t storage_initialized;
static uint8_t sector_scratch[512] __attribute__((aligned(32)));

static int storage_init(void)
{
    storage_initialized = RT_TRUE;
    return RT_EOK;
}
INIT_COMPONENT_EXPORT(storage_init);

/* Like official OpenMV, USB exposes the mounted FAT medium automatically.
 * This is not a coherent filesystem shared between two writers: avoid host
 * writes while a script modifies files. The backend lock serializes the
 * hardware transfers; the DFS lock here protects media state and path lookup.
 *
 * A host write can invalidate FatFs's cached directory/FAT sector. Track open
 * FIL/DIR objects so that the next path operation can refresh an idle volume
 * without freeing live handles. The generated DFS views route native calls,
 * FINSH aliases and internal calls through these wrappers. DFS's recursive
 * mutex also supports chdir -> opendir without a lock-order inversion.
 */
int titan_dfs_file_open_impl(struct dfs_file *, const char *, int);
int titan_dfs_file_close_impl(struct dfs_file *);
int titan_dfs_file_stat_impl(const char *, struct stat *);
int titan_dfs_file_unlink_impl(const char *);
int titan_dfs_file_rename_impl(const char *, const char *);
int titan_dfs_statfs_impl(const char *, struct statfs *);
int titan_dfs_mount_impl(const char *, const char *, const char *, unsigned long, const void *);
int titan_dfs_unmount_impl(const char *);
int titan_dfs_mkfs_impl(const char *, const char *);
int titan_chdir_impl(const char *);

static int storage_usb_prepare(void);
static int storage_refresh(void);

static int storage_device_name(const char *name)
{
    return name && !rt_strcmp(name, STORAGE_DEVICE);
}

static int storage_file(const struct dfs_file *file)
{
    return file && file->vnode && file->vnode->fs && file->vnode->fs->dev_id &&
           storage_device_name(file->vnode->fs->dev_id->parent.name);
}

int dfs_file_open(struct dfs_file *file, const char *path, int flags)
{
    if (!storage_initialized) { return titan_dfs_file_open_impl(file, path, flags); }
    dfs_lock();
    int result = storage_refresh();
    if (result == RT_EOK) { result = titan_dfs_file_open_impl(file, path, flags); }
    if (result == RT_EOK && storage_file(file)) { volume_open_files++; }
    dfs_unlock();
    return result;
}

int dfs_file_close(struct dfs_file *file)
{
    if (!storage_initialized) { return titan_dfs_file_close_impl(file); }
    dfs_lock();
    /* dup() shares the descriptor: FatFs closes only the final reference. */
    int closing_volume = storage_file(file) && file->ref_count == 1;
    int result = titan_dfs_file_close_impl(file);
    if (closing_volume && volume_open_files) { volume_open_files--; }
    dfs_unlock();
    return result;
}

#define STORAGE_PATH_WRAPPER(name, arguments, call) \
    int name arguments { \
        if (!storage_initialized) { return titan_##name##_impl call; } \
        dfs_lock(); \
        int result = storage_refresh(); \
        if (result == RT_EOK) { result = titan_##name##_impl call; } \
        dfs_unlock(); \
        return result; \
    }
STORAGE_PATH_WRAPPER(dfs_file_stat, (const char *path, struct stat *buf), (path, buf))
STORAGE_PATH_WRAPPER(dfs_file_unlink, (const char *path), (path))
STORAGE_PATH_WRAPPER(dfs_file_rename, (const char *oldpath, const char *newpath), (oldpath, newpath))
STORAGE_PATH_WRAPPER(dfs_statfs, (const char *path, struct statfs *buf), (path, buf))
#undef STORAGE_PATH_WRAPPER

int chdir(const char *path)
{
    if (!storage_initialized) { return titan_chdir_impl(path); }
    /* Retain POSIX -1/errno semantics on a failed volume refresh. */
    dfs_lock();
    int result;
    result = storage_refresh();
    if (result < 0) { rt_set_errno(result); result = -1; }
    else { result = titan_chdir_impl(path); }
    dfs_unlock();
    return result;
}

int dfs_mount(const char *device, const char *path, const char *type,
                     unsigned long flags, const void *data)
{
    if (!storage_initialized) { return titan_dfs_mount_impl(device, path, type, flags, data); }
    dfs_lock();
    int result = titan_dfs_mount_impl(device, path, type, flags, data);
    dfs_unlock();
    return result;
}

int dfs_unmount(const char *path)
{
    if (!storage_initialized) { return titan_dfs_unmount_impl(path); }
    dfs_lock();
    struct dfs_filesystem *fs = dfs_filesystem_lookup(path);
    int is_volume = fs && fs->dev_id && storage_device_name(fs->dev_id->parent.name);
    int result = is_volume && volume_open_files ? -EBUSY : titan_dfs_unmount_impl(path);
    if (result == RT_EOK && path && !rt_strcmp(path, "/")) { mounted_device[0] = 0; }
    dfs_unlock();
    return result;
}

int dfs_mkfs(const char *type, const char *device)
{
    if (!storage_initialized) { return titan_dfs_mkfs_impl(type, device); }
    dfs_lock();
    int result = storage_device_name(device) && volume_open_files ? -EBUSY :
                 titan_dfs_mkfs_impl(type, device);
    dfs_unlock();
    return result;
}

/* Called with dfs_lock held. Never refresh underneath an open file/directory.
 * This handles host-side copies between scripts (including recovery assets).
 * It does not make simultaneous host/Python filesystem writes safe. */
static int storage_refresh(void)
{
    if (!host_write_pending || volume_open_files) { return RT_EOK; }
    if (!storage_backend_ready()) { return -ENODEV; }
    if (mounted_device[0])
    {
        rt_strncpy(refresh_device, mounted_device, sizeof(refresh_device));
        int result = titan_dfs_unmount_impl("/");
        if (result != RT_EOK) { return result; }
        mounted_device[0] = 0;
    }
    if (!refresh_device[0]) { return RT_EOK; }
    int result = titan_dfs_mount_impl(refresh_device, "/", "elm", 0, RT_NULL);
    if (result == RT_EOK)
    {
        rt_strncpy(mounted_device, refresh_device, sizeof(mounted_device));
        refresh_device[0] = 0;
        host_write_pending = RT_FALSE;
    }
    else { rt_kprintf("[titan.storage] FAT refresh failed (%d); USB medium remains available\n", result); }
    return result;
}

static int storage_mount_device(const char *name)
{
    const int result = dfs_mount(name, "/", "elm", 0, RT_NULL);
    if (result == RT_EOK)
    {
        rt_strncpy(mounted_device, name, sizeof(mounted_device) - 1);
        refresh_device[0] = 0;
        host_write_pending = RT_FALSE;
        last_mount_error = 0;
        rt_kprintf("[titan.storage] FAT %s mounted at /\n", name);
    }
    else
    {
        if (last_mount_error != result) {
            rt_kprintf("[titan.storage] FAT mount %s failed (%d); media unchanged\n", name, result);
            last_mount_error = result;
        }
    }
    return result;
}

int titan_storage_mount(rt_int32_t timeout_ms)
{
    /* Serialize shell/worker initialization as well as FAT state. USB status
     * callbacks do not take this lock, including while a card is probing. */
    dfs_lock();
    int result = storage_backend_init(timeout_ms);
    if (result != RT_EOK) {
        if (last_init_error != result && result != -ENODEV && result != -EAGAIN) {
            rt_kprintf("[titan.storage] %s initialization failed (%d)\n", titan_storage_backend_name, result);
            last_init_error = result;
        }
        dfs_unlock();
        return result;
    }
    last_init_error = 0;
    if (mounted_device[0])
    {
        result = storage_backend_ready() ? storage_refresh() : -EIO;
        goto done;
    }
    struct dfs_filesystem *existing = dfs_filesystem_lookup("/");
    if (existing)
    {
        const char *name = existing->dev_id ? existing->dev_id->parent.name : "";
        if (existing->ops && !rt_strcmp(existing->ops->name, "elm") && storage_device_name(name))
        {
            rt_strncpy(mounted_device, name, sizeof(mounted_device) - 1);
            initial_check_done = RT_TRUE;
            result = RT_EOK;
        }
        else
        {
            result = -EBUSY;
        }
        goto done;
    }
    if (!initial_check_done && !usb_device)
    {
#if defined(BSP_OPENMV_STORAGE_FLASH)
        bool needs_format = false;
        /* dfs_mount loses the underlying FRESULT. Probe preserves it and
         * tracks read errors, so an I/O/ENOMEM/busy failure never means mkfs. */
        result = titan_elm_probe("qflash", &needs_format);
        if (result != RT_EOK) { goto done; }
        initial_check_done = RT_TRUE;
        if (needs_format && !format_attempted)
        {
            /* Authorised first-use provisioning. Set the latch before the
             * write so a failure cannot repeatedly erase on worker retries. */
            format_attempted = RT_TRUE;
            rt_kprintf("[titan.flash] no FAT volume; initializing filesystem once\n");
            result = titan_elm_format("qflash");
            if (result != RT_EOK)
            {
                rt_kprintf("[titan.flash] initialization failed (%d); no automatic retry\n", result);
                goto done;
            }
        }
#else
        /* Removable cards are never formatted automatically. Export the raw
         * card even when FAT is missing so the user may format it on the PC. */
        initial_check_done = RT_TRUE;
#endif
    }
    result = storage_mount_device(STORAGE_DEVICE);
done:
    /* Only publish after a valid probe and the initial format decision. Once
     * visible, later host formatting/FAT refresh must never trigger mkfs.
     * A failed initial mkfs can still be repaired using the PC's format tool. */
    if (initial_check_done) { storage_usb_prepare(); }
    dfs_unlock();
    return result;
}

static int storage_mount(int argc, char **argv)
{
    (void)argc; (void)argv;
    return titan_storage_mount(0);
}
MSH_CMD_EXPORT(storage_mount, Initialize or mount the selected FAT medium);
#if defined(BSP_OPENMV_STORAGE_SD)
MSH_CMD_EXPORT_ALIAS(storage_mount, sd_mount, Initialize or mount the SD FAT volume);
#else
MSH_CMD_EXPORT_ALIAS(storage_mount, flash_mount, Initialize or mount the onboard QSPI FAT volume);
#endif

int titan_storage_usb_enabled(void)
{
    uint32_t blocks;
    uint16_t size;
    return titan_storage_usb_snapshot(&blocks, &size) == RT_EOK;
}

int titan_storage_usb_writable(void)
{
    return titan_storage_usb_enabled() && storage_backend_writable();
}

int titan_storage_usb_snapshot(uint32_t *blocks, uint16_t *block_size)
{
    /* Immutable geometry and a metadata/pad ready check: no media commands or
     * DFS locks are taken by the USB task's capacity/status callbacks. */
    rt_base_t level = rt_hw_interrupt_disable();
    int ready = usb_device && usb_media_enabled;
    *blocks = ready ? usb_geometry.sector_count : 0;
    *block_size = 512;
    rt_hw_interrupt_enable(level);
    if (!ready || !storage_backend_ready()) { *blocks = 0; return -ENODEV; }
    return RT_EOK;
}

int titan_storage_usb_media_set(int enabled)
{
    /* BOT executes commands serially. WRITE10 completion waits for physical
     * programming in the worker, so LOAD/EJECT need only change visibility.
     * Do not remount FAT here: local pathname hooks own deferred refresh. */
    rt_base_t level = rt_hw_interrupt_disable();
    usb_media_enabled = enabled ? RT_TRUE : RT_FALSE;
    int present = usb_device != RT_NULL;
    rt_hw_interrupt_enable(level);
    return enabled && (!present || !storage_backend_ready()) ? -ENODEV : RT_EOK;
}

/* Called with dfs_lock held. Export the whole SD card or bounded Flash slice. */
static int storage_usb_prepare(void)
{
    int result = -ENODEV;
    if (usb_device) { return RT_EOK; }
    if (!initial_check_done || !storage_backend_ready()) { goto done; }
    rt_device_t raw = rt_device_find(STORAGE_DEVICE);
    if (!raw) { goto done; }
    result = rt_device_control(raw, RT_DEVICE_CTRL_BLK_GETGEOME, &usb_geometry);
    if (result != RT_EOK) { goto done; }
    if (usb_geometry.bytes_per_sector != 512 || !usb_geometry.sector_count ||
        usb_geometry.sector_count > INT32_MAX)
    {
        result = -EINVAL;
        goto done;
    }
    result = rt_device_open(raw, RT_DEVICE_OFLAG_RDWR);
    if (result == RT_EOK)
    {
        rt_base_t level = rt_hw_interrupt_disable();
        usb_device = raw;
        rt_hw_interrupt_enable(level);
        rt_kprintf("[titan.msc] %s USB medium initialized; local FAT remains available\n", titan_storage_backend_name);
    }
done:
    return result;
}

int titan_storage_usb_export(void)
{
    if (!usb_device) { usb_media_enabled = RT_TRUE; return -ENODEV; }
    dfs_lock();
    usb_media_enabled = RT_TRUE;
    int result = storage_usb_prepare();
    if (result == RT_EOK && !storage_backend_ready()) { result = -ENODEV; }
    dfs_unlock();
    return result;
}

int titan_storage_usb_start(void)
{
    /* Boot default is enabled. A later retry must respect an explicit eject. */
    dfs_lock();
    int result = storage_usb_prepare();
    dfs_unlock();
    return result;
}

int titan_storage_usb_release(void)
{
    if (!usb_device) { usb_media_enabled = RT_FALSE; return RT_EOK; }
    int result = RT_EOK;
    dfs_lock();
    if (usb_device)
    {
        result = storage_backend_ready() ? storage_backend_sync() : -ENODEV;
        if (result != RT_EOK) { goto done; }
    }
    usb_media_enabled = RT_FALSE;
    /* Eject only hides the host medium. It does not revoke Python access.
     * A refresh failure must not turn a completed host eject into failure. */
    if (storage_backend_ready()) { storage_refresh(); }
done:
    dfs_unlock();
    return result;
}

int titan_storage_usb_capacity(uint32_t *blocks, uint16_t *block_size)
{
    return titan_storage_usb_snapshot(blocks, block_size);
}

static int storage_usb_transfer(uint32_t lba, uint32_t offset, void *buffer,
                                uint32_t size, int writing)
{
    if (!usb_device) { return -ENODEV; }
    int result = -ENODEV;
    dfs_lock();
    if (!usb_media_enabled || !usb_device || !storage_backend_ready()) { goto done; }
    if (writing && !storage_backend_writable()) { result = -EROFS; goto done; }
    uint64_t address = (uint64_t)lba * 512 + offset;
    uint64_t capacity = (uint64_t)usb_geometry.sector_count * 512;
    if (!buffer || address > capacity || size > capacity - address || size > INT32_MAX)
    {
        result = -EINVAL;
        goto done;
    }
    uint8_t *bytes = buffer;
    uint32_t remaining = size;
    /* Even a partially failed WRITE may have changed FAT/directory sectors. */
    if (writing && size) { host_write_pending = RT_TRUE; }
    while (remaining)
    {
        uint32_t sector = address / 512, within = address % 512;
        uint32_t count = remaining < 512 - within ? remaining : 512 - within;
        if (within || count < 512)
        {
            if (rt_device_read(usb_device, sector, sector_scratch, 1) != 1) { result = -EIO; goto done; }
            if (writing)
            {
                rt_memcpy(sector_scratch + within, bytes, count);
                if (rt_device_write(usb_device, sector, sector_scratch, 1) != 1) { result = -EIO; goto done; }
            }
            else { rt_memcpy(bytes, sector_scratch + within, count); }
        }
        else
        {
            uint32_t sectors = remaining / 512;
            rt_ssize_t transferred = writing ? rt_device_write(usb_device, sector, bytes, sectors) :
                                               rt_device_read(usb_device, sector, bytes, sectors);
            if (transferred != sectors) { result = -EIO; goto done; }
            count = sectors * 512;
        }
        remaining -= count;
        address += count;
        bytes += count;
    }
    /* Do not acknowledge WRITE10 until the medium has finished programming. */
    result = writing && size ? storage_backend_sync() : RT_EOK;
    if (result == RT_EOK) { result = size; }
done:
    dfs_unlock();
    return result;
}

int titan_storage_usb_read(uint32_t lba, uint32_t offset, void *buffer, uint32_t size)
{
    return storage_usb_transfer(lba, offset, buffer, size, 0);
}

int titan_storage_usb_write(uint32_t lba, uint32_t offset, const void *buffer, uint32_t size)
{
    return storage_usb_transfer(lba, offset, (void *)buffer, size, 1);
}

int titan_storage_usb_sync(void)
{
    if (!usb_device) { return -ENODEV; }
    dfs_lock();
    int result = usb_media_enabled && usb_device && storage_backend_ready() ? storage_backend_sync() : -ENODEV;
    dfs_unlock();
    return result;
}

static void usb_msc(int argc, char **argv)
{
    int result = RT_EOK;
    if (argc == 2 && !rt_strcmp(argv[1], "on")) { result = titan_storage_usb_export(); }
    else if (argc == 2 && !rt_strcmp(argv[1], "off")) { result = titan_storage_usb_release(); }
    else if (argc > 2 || (argc == 2 && rt_strcmp(argv[1], "status")))
    {
        rt_kprintf("usb_msc on|off|status (automatic at boot; eject on PC before off)\n");
        return;
    }
    bool ready = storage_backend_ready();
    rt_kprintf("MSC medium=%s backend=%s ready=%d result=%d\n",
               !ready ? "unavailable" : titan_storage_usb_enabled() ? "available" : "ejected",
               titan_storage_backend_name, ready, result);
}
MSH_CMD_EXPORT(usb_msc, Enable or eject USB medium without unmounting local FAT);
