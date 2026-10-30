/* SPDX-License-Identifier: Apache-2.0 */
/* Included by the generated dfs_elm view, after disk I/O implementations. */
#include <errno.h>
#include "titan_flash_fs.h"

struct titan_elm_volume {
    rt_device_t device;
    FATFS *fat;
    int index;
    char path[3];
    rt_bool_t opened;
};

static int titan_elm_result(FRESULT result)
{
    switch (result)
    {
        case FR_OK: return RT_EOK;
        case FR_NOT_ENOUGH_CORE: return -ENOMEM;
        case FR_TIMEOUT: return -ETIMEDOUT;
        case FR_LOCKED: return -EBUSY;
        case FR_NOT_READY: return -ENODEV;
        case FR_WRITE_PROTECTED:
        case FR_DENIED: return -EROFS;
        case FR_INVALID_PARAMETER:
        case FR_INVALID_NAME:
        case FR_INVALID_DRIVE:
        case FR_NO_FILESYSTEM:
        case FR_MKFS_ABORTED: return -EINVAL;
        default: return -EIO;
    }
}

static int titan_elm_volume_close(struct titan_elm_volume *volume)
{
    int result = RT_EOK;
    if (volume->fat)
    {
        /* Remove FatFs's pointer and its mutex before releasing the object.
         * This port's ff_del_syncobj always succeeds for a valid drive. */
        FRESULT unmount = f_mount(RT_NULL, volume->path, 0);
        if (unmount != FR_OK)
        {
            /* Do not free an object the core may still reference. The drive
             * remains reserved rather than becoming a dangling disk mapping. */
            return titan_elm_result(unmount);
        }
        titan_elm_probe_active[volume->index] = RT_FALSE;
        titan_elm_probe_read_failed[volume->index] = RT_FALSE;
        disk[volume->index] = RT_NULL;
        rt_free(volume->fat);
        volume->fat = RT_NULL;
    }
    if (volume->opened)
    {
        if (rt_device_close(volume->device) != RT_EOK) { result = -EIO; }
        volume->opened = RT_FALSE;
    }
    return result;
}

static int titan_elm_volume_open(const char *name, struct titan_elm_volume *volume)
{
    memset(volume, 0, sizeof(*volume));
    if (!name || !name[0]) { return -EINVAL; }
    volume->device = rt_device_find(name);
    if (!volume->device) { return -ENODEV; }
    /* Never replace the FATFS object or invalidate handles of a mounted disk. */
    if (get_disk(volume->device) >= 0) { return -EBUSY; }
    volume->index = get_disk(RT_NULL);
    if (volume->index < 0 || volume->index > 9) { return -ENOSPC; }

    int result = rt_device_open(volume->device, RT_DEVICE_OFLAG_RDWR);
    if (result != RT_EOK) { return result < 0 ? result : -EIO; }
    volume->opened = RT_TRUE;
    struct rt_device_blk_geometry geometry = {0};
    result = rt_device_control(volume->device, RT_DEVICE_CTRL_BLK_GETGEOME, &geometry);
    if (result != RT_EOK) { result = result < 0 ? result : -EIO; goto fail; }
    /* 2 MiB is reserved before this device; the driver enforces its offset. */
    if (geometry.bytes_per_sector != 512 || geometry.sector_count != 12288 ||
        geometry.block_size != 4096)
    {
        result = -EINVAL;
        goto fail;
    }
    volume->fat = rt_malloc(sizeof(FATFS));
    if (!volume->fat) { result = -ENOMEM; goto fail; }
    memset(volume->fat, 0, sizeof(FATFS));
    volume->path[0] = '0' + volume->index;
    volume->path[1] = ':';
    disk[volume->index] = volume->device;
    return RT_EOK;
fail:
    titan_elm_volume_close(volume);
    return result;
}

int titan_elm_probe(const char *device, bool *needs_format)
{
    if (!needs_format) { return -EINVAL; }
    *needs_format = false;
    struct titan_elm_volume volume;
    int result = titan_elm_volume_open(device, &volume);
    if (result != RT_EOK) { return result; }

    titan_elm_probe_read_failed[volume.index] = RT_FALSE;
    titan_elm_probe_active[volume.index] = RT_TRUE;
    FRESULT mounted = f_mount(volume.fat, volume.path, 1);
    bool io_failed = titan_elm_probe_read_failed[volume.index] != RT_FALSE;
    titan_elm_probe_active[volume.index] = RT_FALSE;
    bool absent = mounted == FR_NO_FILESYSTEM && !io_failed;
    result = io_failed ? -EIO : (absent ? RT_EOK : titan_elm_result(mounted));
    int closed = titan_elm_volume_close(&volume);
    if (result == RT_EOK) { result = closed; }
    if (result == RT_EOK) { *needs_format = absent; }
    return result;
}

int titan_elm_format(const char *device)
{
    struct titan_elm_volume volume;
    int result = titan_elm_volume_open(device, &volume);
    if (result != RT_EOK) { return result; }
    /* A full erase-block work buffer improves multi-sector metadata writes
     * without placing 4 KiB on the loader/msh stack. */
    void *work = rt_malloc(4096);
    if (!work)
    {
        titan_elm_volume_close(&volume);
        return -ENOMEM;
    }
    FRESULT formatted = f_mount(volume.fat, volume.path, 0);
    if (formatted == FR_OK)
    {
        const MKFS_PARM options = {
            .fmt = FM_FAT | FM_SFD, .n_fat = 2, .align = 8,
            .n_root = 512, .au_size = 1024
        };
        formatted = f_mkfs(volume.path, &options, work, 4096);
    }
    result = titan_elm_result(formatted);
    rt_free(work);
    int closed = titan_elm_volume_close(&volume);
    return result == RT_EOK ? closed : result;
}
