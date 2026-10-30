/* SPDX-License-Identifier: Apache-2.0 */
#include <errno.h>
#include <string.h>
#include <rtthread.h>
#include <rthw.h>
#include "tusb.h"
#include "titan_storage.h"
#include "ra8_usb_msc.h"

static bool prevent_removal;

enum msc_io_state { MSC_IO_IDLE, MSC_IO_QUEUED, MSC_IO_RUNNING, MSC_IO_DONE };
static struct {
    enum msc_io_state state;
    uint32_t generation;
    uint32_t lba, offset, size;
    uint8_t lun;
    bool writing;
    void *usb_buffer;
    int result;
} msc_io;
static uint32_t msc_generation = 1;
static bool worker_started;
static struct rt_semaphore io_ready;
static struct rt_thread io_thread;
static uint8_t io_stack[4096] __attribute__((aligned(RT_ALIGN_SIZE)));
/* The storage driver must never own TinyUSB's endpoint buffer: reset immediately
 * reuses that buffer for a new CBW. This buffer remains reserved even when a
 * reset cancels an operation which is still inside media/DFS. */
static uint8_t io_buffer[CFG_TUD_MSC_EP_BUFSIZE] __attribute__((aligned(32)));

static void msc_io_thread(void *arg)
{
    (void)arg;
    for (;;)
    {
        if (rt_sem_take(&io_ready, RT_WAITING_FOREVER) != RT_EOK) { continue; }
        rt_base_t level = rt_hw_interrupt_disable();
        if (msc_io.state != MSC_IO_QUEUED)
        {
            rt_hw_interrupt_enable(level);
            continue;
        }
        if (msc_io.generation != msc_generation)
        {
            msc_io.state = MSC_IO_DONE;
            rt_hw_interrupt_enable(level);
            continue;
        }
        msc_io.state = MSC_IO_RUNNING;
        rt_hw_interrupt_enable(level);

        /* All media access, filesystem locks, media waits and write sync happen
         * below the USB and VM task priorities, never in a USB callback. */
        int result = msc_io.writing ?
            titan_storage_usb_write(msc_io.lba, msc_io.offset, io_buffer, msc_io.size) :
            titan_storage_usb_read(msc_io.lba, msc_io.offset, io_buffer, msc_io.size);

        level = rt_hw_interrupt_disable();
        msc_io.result = result;
        msc_io.state = MSC_IO_DONE;
        rt_hw_interrupt_enable(level);
    }
}

int ra8_usb_msc_init(void)
{
    if (worker_started) { return RT_EOK; }
    int result = rt_sem_init(&io_ready, "msc_io", 0, RT_IPC_FLAG_PRIO);
    if (result != RT_EOK) { return result; }
    /* The VM uses priority 20; keep media waits below both VM and tusb (15). */
    result = rt_thread_init(&io_thread, "msc_io", msc_io_thread, RT_NULL,
                            io_stack, sizeof(io_stack), 25, 10);
    if (result == RT_EOK) { result = rt_thread_startup(&io_thread); }
    if (result != RT_EOK)
    {
        rt_sem_detach(&io_ready);
        return result;
    }
    worker_started = true;
    return RT_EOK;
}

void ra8_usb_msc_reset(void)
{
    rt_base_t level = rt_hw_interrupt_disable();
    ++msc_generation;
    prevent_removal = false;
    rt_hw_interrupt_enable(level);
}

bool ra8_usb_msc_needs_service(void)
{
    rt_base_t level = rt_hw_interrupt_disable();
    bool active = msc_io.state == MSC_IO_QUEUED || msc_io.state == MSC_IO_RUNNING;
    rt_hw_interrupt_enable(level);
    return active;
}

static bool valid_lun(uint8_t lun)
{
    if (lun == 0) { return true; }
    tud_msc_set_sense(lun, SCSI_SENSE_ILLEGAL_REQUEST, 0x25, 0x00);
    return false;
}

static int32_t io_result(uint8_t lun, int result, bool writing)
{
    if (result >= 0) { return result; }
    if (result == -EBUSY)
    {
        tud_msc_set_sense(lun, SCSI_SENSE_NOT_READY, 0x04, 0x01);
    }
    else if (result == -ENODEV)
    {
        tud_msc_set_sense(lun, SCSI_SENSE_NOT_READY, 0x3a, 0x00);
    }
    else if (result == -EINVAL)
    {
        tud_msc_set_sense(lun, SCSI_SENSE_ILLEGAL_REQUEST, 0x21, 0x00);
    }
    else if (result == -EROFS)
    {
        tud_msc_set_sense(lun, SCSI_SENSE_DATA_PROTECT, 0x27, 0x00);
    }
    else
    {
        tud_msc_set_sense(lun, SCSI_SENSE_MEDIUM_ERROR, writing ? 0x0c : 0x11, 0x00);
    }
    return -1;
}

static bool io_available(uint8_t lun)
{
    rt_base_t level = rt_hw_interrupt_disable();
    bool available = msc_io.state == MSC_IO_IDLE;
    rt_hw_interrupt_enable(level);
    if (!available) { io_result(lun, -EBUSY, false); }
    return available;
}

void ra8_usb_msc_poll(void)
{
    rt_base_t level = rt_hw_interrupt_disable();
    if (msc_io.state == MSC_IO_DONE)
    {
        if (msc_io.generation == msc_generation)
        {
            int result = msc_io.result;
            /* Storage must consume the entire chunk. Async WRITE completion
             * uses that count to advance BOT, so a partial success is unsafe. */
            if (result >= 0 && (uint32_t)result != msc_io.size) { result = -EIO; }
            if (!msc_io.writing && result > 0)
            {
                memcpy(msc_io.usb_buffer, io_buffer, (uint32_t)result);
            }
            result = io_result(msc_io.lun, result, msc_io.writing);
            msc_io.state = MSC_IO_IDLE;
            /* IRQ lock spans validation, copy and class completion: a bus
             * reset cannot slip between them. No queued completion can later
             * acknowledge a different command after a reset. */
            ra8_msc_complete_now(result);
        }
        else
        {
            msc_io.state = MSC_IO_IDLE;
        }
    }
    rt_hw_interrupt_enable(level);
}

static int32_t submit_io(uint8_t lun, uint32_t lba, uint32_t offset,
                         void *buffer, uint32_t size, bool writing)
{
    if (!valid_lun(lun)) { return -1; }
    if (!buffer || !size || size > sizeof(io_buffer)) { return io_result(lun, -EINVAL, writing); }
    uint32_t blocks;
    uint16_t block_size;
    int result = titan_storage_usb_snapshot(&blocks, &block_size);
    if (result != 0) { return io_result(lun, result, writing); }
    if ((uint64_t)lba * block_size + offset + size > (uint64_t)blocks * block_size)
    {
        return io_result(lun, -EINVAL, writing);
    }
    rt_base_t level = rt_hw_interrupt_disable();
    if (!worker_started || msc_io.state != MSC_IO_IDLE)
    {
        /* A canceled media operation may still own the bounce buffer. Fail this
         * CBW with NOT READY; never busy-spin synthetic USB completion events. */
        rt_hw_interrupt_enable(level);
        return io_result(lun, -EBUSY, writing);
    }
    msc_io.generation = msc_generation;
    msc_io.lba = lba;
    msc_io.offset = offset;
    msc_io.size = size;
    msc_io.lun = lun;
    msc_io.writing = writing;
    msc_io.usb_buffer = buffer;
    if (writing) { memcpy(io_buffer, buffer, size); }
    msc_io.state = MSC_IO_QUEUED;
    rt_hw_interrupt_enable(level);
    rt_sem_release(&io_ready);
    return TUD_MSC_RET_ASYNC;
}

void tud_msc_inquiry_cb(uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16], uint8_t revision[4])
{
    (void)lun;
    memcpy(vendor_id, "OpenMV  ", 8);
#if defined(BSP_OPENMV_STORAGE_SD)
    memcpy(product_id, "Titan SD Card   ", 16);
#else
    memcpy(product_id, "Titan QSPI Flash", 16);
#endif
    memcpy(revision, "2.00", 4);
}

bool tud_msc_test_unit_ready_cb(uint8_t lun)
{
    if (!valid_lun(lun)) { return false; }
    if (!io_available(lun)) { return false; }
    uint32_t blocks;
    uint16_t size;
    int result = titan_storage_usb_snapshot(&blocks, &size);
    if (result != 0)
    {
        prevent_removal = false;
        io_result(lun, result, false);
        return false;
    }
    return true;
}

void tud_msc_capacity_cb(uint8_t lun, uint32_t *blocks, uint16_t *size)
{
    *blocks = 0;
    *size = 512;
    if (valid_lun(lun) && io_available(lun))
    {
        io_result(lun, titan_storage_usb_snapshot(blocks, size), false);
    }
}

bool tud_msc_is_writable_cb(uint8_t lun)
{
    return valid_lun(lun) && titan_storage_usb_writable();
}

int32_t tud_msc_read10_cb(uint8_t lun, uint32_t lba, uint32_t offset, void *buffer, uint32_t size)
{
    return submit_io(lun, lba, offset, buffer, size, false);
}

int32_t tud_msc_write10_cb(uint8_t lun, uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t size)
{
    return submit_io(lun, lba, offset, buffer, size, true);
}

bool tud_msc_prevent_allow_medium_removal_cb(uint8_t lun, uint8_t prohibit_removal, uint8_t control)
{
    (void)control;
    if (!valid_lun(lun)) { return false; }
    prevent_removal = prohibit_removal != 0;
    return true;
}

bool tud_msc_start_stop_cb(uint8_t lun, uint8_t power_condition, bool start, bool load_eject)
{
    (void)power_condition;
    if (!valid_lun(lun)) { return false; }
    /* A BOT reset can cancel a command while its media write still runs. Do not
     * report safe eject/load/stop until the worker has released that job. */
    if (!io_available(lun)) { return false; }
    if (!load_eject) { return !start || tud_msc_test_unit_ready_cb(lun); }
    if (start)
    {
        /* Official OpenMV uses START to reload an ejected host medium.
         * The local FAT mount remains available throughout. */
        return io_result(lun, titan_storage_usb_media_set(true), false) >= 0;
    }
    if (prevent_removal)
    {
        tud_msc_set_sense(lun, SCSI_SENSE_ILLEGAL_REQUEST, 0x53, 0x02);
        return false;
    }
    /* WRITE10 is completed only after the medium reaches its ready state. BOT
     * serializes commands, so eject has no outstanding cached writes to drain. */
    int result = titan_storage_usb_media_set(false);
    return io_result(lun, result, true) >= 0;
}

int32_t tud_msc_scsi_cb(uint8_t lun, const uint8_t command[16], void *buffer, uint16_t size)
{
    (void)buffer;
    (void)size;
    if (!valid_lun(lun)) { return -1; }
    if (command[0] == 0x35) /* SYNCHRONIZE CACHE(10) */
    {
        /* No write-back cache: the worker synchronizes every successful write
         * before CSW. Waiting on the medium again here would stall CDC unnecessarily. */
        uint32_t blocks;
        uint16_t block_size;
        if (size != 0) { return io_result(lun, -EINVAL, true); }
        if (!io_available(lun)) { return -1; }
        return io_result(lun, titan_storage_usb_snapshot(&blocks, &block_size), true);
    }
    tud_msc_set_sense(lun, SCSI_SENSE_ILLEGAL_REQUEST, 0x20, 0x00);
    return -1;
}
