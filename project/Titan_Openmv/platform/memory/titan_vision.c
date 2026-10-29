/* SPDX-License-Identifier: Apache-2.0 */
#include <rtthread.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "bsp_api.h"
#include "titan_memory.h"
#include "titan_storage.h"
#include "titan_vision.h"
#include "led.h"
#include "lib/uzlib/uzlib.h"

#define VISION_PATH "/openmv-vision.bin"
#define VISION_BASE UINT32_C(0x70c00000)
#define VISION_CAPACITY UINT32_C(0x00400000)
#define VISION_HEADER_SIZE 64U
#define VISION_FORMAT_RAW 1U
#define VISION_FORMAT_DEFLATE 2U
#define VISION_CODEC_DEFLATE 1U

extern uint8_t __openmv_vision_start[];
extern uint8_t __openmv_vision_end[];

/* The post-link packer replaces these bytes with SHA256(.openmv_vision).
 * Volatile prevents folding comparisons against this placeholder at compile
 * time. This object must remain in internal MRAM, outside the vision image. */
const volatile uint8_t titan_vision_build_id[32]
    __attribute__((section(".vision_binding"), aligned(4), used)) = {0};

static volatile rt_bool_t vision_ready;
static volatile rt_bool_t service_started;
static struct rt_thread vision_thread;
static uint8_t vision_stack[4096] __attribute__((aligned(RT_ALIGN_SIZE)));
static const char *last_failure;

static int failure(const char *reason, int error)
{
    led_boot_vision(false);
    if (last_failure != reason)
    {
        rt_kprintf("[titan.vision] %s: %s (%d)\n", VISION_PATH, reason, error);
        last_failure = reason;
    }
    return error;
}

static uint32_t get_le32(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static int read_exact(int fd, uint8_t *destination, size_t count)
{
    while (count)
    {
        ssize_t result = read(fd, destination, count);
        if (result < 0)
        {
            if (errno == EINTR) { continue; }
            return -EIO;
        }
        if (result == 0) { return -EIO; }
        destination += (size_t)result;
        count -= (size_t)result;
    }
    return RT_EOK;
}

static uint32_t crc32_update(uint32_t crc, const uint8_t *data, size_t length)
{
    while (length--)
    {
        crc ^= *data++;
        for (unsigned bit = 0; bit < 8; ++bit)
        {
            crc = (crc >> 1) ^ (UINT32_C(0xedb88320) & (0U - (crc & 1U)));
        }
    }
    return crc;
}

struct vision_inflate
{
    uzlib_uncomp_t decoder;
    int fd;
    int read_error;
    size_t remaining;
    uint8_t input[4096];
};

static int inflate_read(void *arg)
{
    struct vision_inflate *stream = arg;
    size_t count = stream->remaining;
    if (!count) { return -1; }
    if (count > sizeof(stream->input)) { count = sizeof(stream->input); }
    if (read_exact(stream->fd, stream->input, count) != RT_EOK)
    {
        stream->read_error = -EIO;
        return -1;
    }
    stream->remaining -= count;
    stream->decoder.source = stream->input + 1;
    stream->decoder.source_limit = stream->input + count;
    return stream->input[0];
}

static int load_deflate(int fd, size_t stored_length, size_t length, uint32_t *crc)
{
    /* Keep the worker's 4 KiB stack free for FatFs and decoder call frames.
     * The output itself is the history dictionary; no second image is kept. */
    struct vision_inflate *stream = rt_calloc(1, sizeof(*stream));
    if (!stream) { return -ENOMEM; }
    uzlib_uncomp_t *decoder = &stream->decoder;
    uzlib_uncompress_init(decoder, RT_NULL, 0);
    stream->fd = fd;
    stream->remaining = stored_length;
    decoder->source = stream->input;
    decoder->source_limit = stream->input;
    decoder->source_read_data = stream;
    decoder->source_read_cb = inflate_read;
    decoder->dest_start = __openmv_vision_start;
    decoder->dest = __openmv_vision_start;

    int result = UZLIB_OK;
    uint8_t *end = __openmv_vision_start + length;
    while (decoder->dest < end && result == UZLIB_OK)
    {
        uint8_t *chunk_start = decoder->dest;
        size_t chunk = (size_t)(end - chunk_start);
        if (chunk > 4096) { chunk = 4096; }
        decoder->dest_limit = chunk_start + chunk;
        result = uzlib_uncompress(decoder);
        *crc = crc32_update(*crc, chunk_start, (size_t)(decoder->dest - chunk_start));
    }
    if (result == UZLIB_OK && decoder->dest == end)
    {
        /* The checked uzlib view allows parsing the final block marker with
         * no output space, rejecting any attempt to emit another byte. */
        decoder->dest_limit = end;
        result = uzlib_uncompress(decoder);
    }
    int status = RT_EOK;
    if (result != UZLIB_DONE || decoder->dest != end || decoder->eof ||
        stream->read_error || stream->remaining || decoder->source != decoder->source_limit)
    {
        status = -EIO;
    }
    rt_free(stream);
    return status;
}

static int make_executable(size_t length)
{
#if defined(__MPU_PRESENT) && (__MPU_PRESENT == 1U)
    /* This can walk several MiB; leave USB interrupts active while cleaning. */
    titan_cache_clean(__openmv_vision_start, length);
    /* FSP already owns its non-cacheable DMA regions. Find an unused region
     * and MAIR index instead of changing one of those mappings. */
    rt_base_t level = rt_hw_interrupt_disable();
    uint32_t old_rnr = MPU->RNR;
    uint32_t regions = (MPU->TYPE & MPU_TYPE_DREGION_Msk) >> MPU_TYPE_DREGION_Pos;
    uint32_t free_region = regions;
    uint32_t attributes_used = 0;
    for (uint32_t region = 0; region < regions; ++region)
    {
        MPU->RNR = region;
        uint32_t rlar = MPU->RLAR;
        if (rlar & MPU_RLAR_EN_Msk)
        {
            attributes_used |= 1U << ((rlar & MPU_RLAR_AttrIndx_Msk) >> MPU_RLAR_AttrIndx_Pos);
            uint32_t base = MPU->RBAR & MPU_RBAR_BASE_Msk;
            uint32_t end = (rlar & MPU_RLAR_LIMIT_Msk) | 31U;
            if (base < VISION_BASE + VISION_CAPACITY && end >= VISION_BASE)
            {
                MPU->RNR = old_rnr;
                rt_hw_interrupt_enable(level);
                return -EBUSY;
            }
        }
        else
        {
            free_region = region;
        }
    }
    uint32_t attribute = 0;
    while (attribute < 8 && (attributes_used & (1U << attribute))) { ++attribute; }
    if (free_region == regions || attribute == 8)
    {
        MPU->RNR = old_rnr;
        rt_hw_interrupt_enable(level);
        return -ENOMEM;
    }

    /* File/DMA writes have reached HyperRAM before instruction fetches start. */
    uint32_t control = MPU->CTRL;
    ARM_MPU_Disable();
    const uint8_t normal_wb = ARM_MPU_ATTR_MEMORY_(1U, 1U, 1U, 1U);
    ARM_MPU_SetMemAttr(attribute, ARM_MPU_ATTR(normal_wb, normal_wb));
    ARM_MPU_SetRegion(free_region,
                     ARM_MPU_RBAR(VISION_BASE, ARM_MPU_SH_NON, 1U, 1U, 0U),
                     ARM_MPU_RLAR(VISION_BASE + VISION_CAPACITY - 1U, attribute));
    MPU->RNR = old_rnr;
    ARM_MPU_Enable(control | MPU_CTRL_PRIVDEFENA_Msk);
    __DSB();
#if defined(__ICACHE_PRESENT) && (__ICACHE_PRESENT == 1U)
    SCB_InvalidateICache();
#endif
    __DSB();
    __ISB();
    rt_hw_interrupt_enable(level);
    return RT_EOK;
#else
    (void)length;
    return -ENOSYS;
#endif
}

int titan_vision_load(void)
{
    if (vision_ready) { return RT_EOK; }

    uintptr_t start = (uintptr_t)__openmv_vision_start;
    uintptr_t end = (uintptr_t)__openmv_vision_end;
    if (start != VISION_BASE || end <= start || end - start > VISION_CAPACITY)
    {
        return failure("invalid firmware memory layout", -EINVAL);
    }
    const size_t expected_length = end - start;
    int fd = open(VISION_PATH, O_RDONLY, 0);
    if (fd < 0) { return failure("missing or unreadable; copy the matching asset to the USB storage drive", -ENOENT); }

    uint8_t header[VISION_HEADER_SIZE];
    struct stat info;
    int result = -EINVAL;
    const char *reason = "invalid header or file size";
    if (fstat(fd, &info) != 0 || info.st_size < 0 ||
        (uint64_t)info.st_size < VISION_HEADER_SIZE ||
        read_exact(fd, header, sizeof(header)) != RT_EOK)
    {
        goto done;
    }
    if (memcmp(header, "OMVVISN1", 8) ||
        get_le32(header + 12) != VISION_BASE || get_le32(header + 16) != expected_length)
    {
        goto done;
    }
    uint32_t format = get_le32(header + 8);
    size_t stored_length = expected_length;
    if (format == VISION_FORMAT_RAW)
    {
        for (unsigned i = 56; i < VISION_HEADER_SIZE; ++i)
        {
            if (header[i]) { goto done; }
        }
    }
    else if (format == VISION_FORMAT_DEFLATE)
    {
        stored_length = get_le32(header + 60);
        if (get_le32(header + 56) != VISION_CODEC_DEFLATE ||
            !stored_length || stored_length > VISION_CAPACITY) { goto done; }
    }
    else { goto done; }
    if ((uint64_t)info.st_size != (uint64_t)VISION_HEADER_SIZE + stored_length) { goto done; }
    unsigned digest_nonzero = 0;
    for (unsigned i = 0; i < sizeof(titan_vision_build_id); ++i)
    {
        uint8_t expected = titan_vision_build_id[i];
        digest_nonzero |= expected;
        if (header[24 + i] != expected)
        {
            reason = "asset does not match this firmware; copy the pair from the same build";
            goto done;
        }
    }
    if (!digest_nonzero)
    {
        reason = "firmware was not packaged; use the generated HEX/ELF and vision asset";
        goto done;
    }

    uint32_t crc = UINT32_MAX;
    if (format == VISION_FORMAT_DEFLATE)
    {
        result = load_deflate(fd, stored_length, expected_length, &crc);
        if (result != RT_EOK)
        {
            reason = result == -ENOMEM ? "cannot allocate decompression buffer" :
                     "invalid or incomplete compressed payload; recopy the matching vision asset";
            goto done;
        }
    }
    else
    {
        for (size_t offset = 0; offset < expected_length;)
        {
            size_t chunk = expected_length - offset;
            if (chunk > 4096) { chunk = 4096; }
            if (read_exact(fd, __openmv_vision_start + offset, chunk) != RT_EOK)
            {
                result = -EIO;
                reason = "incomplete payload read";
                goto done;
            }
            crc = crc32_update(crc, __openmv_vision_start + offset, chunk);
            offset += chunk;
        }
    }
    if ((crc ^ UINT32_MAX) != get_le32(header + 20))
    {
        result = -EIO;
        reason = "payload checksum failed; recopy the matching vision asset";
        goto done;
    }
    result = RT_EOK;
done:
    if (close(fd) != 0 && result == RT_EOK)
    {
        result = -EIO;
        reason = "file close failed";
    }
    if (result != RT_EOK) { return failure(reason, result); }
    result = make_executable(expected_length);
    if (result != RT_EOK) { return failure("cannot configure executable HyperRAM", result); }
    vision_ready = RT_TRUE;
    last_failure = RT_NULL;
    led_boot_vision(true);
    rt_kprintf("[titan.vision] verified %u bytes in HyperRAM at 0x%08x\n",
               (unsigned)expected_length, (unsigned)start);
    return RT_EOK;
}

static void vision_entry(void *arg)
{
    (void)arg;
    while (!vision_ready)
    {
        /* Probe and mount the selected storage medium below USB/VM priority.
         * A missing asset is uploaded through MSC while the base VM runs. */
        if (titan_storage_mount(0) == RT_EOK)
        {
            titan_storage_usb_start();
            if (titan_vision_load() == RT_EOK) { return; }
        }
        else { led_boot_vision(false); }
        rt_thread_mdelay(1000);
    }
}

int titan_vision_start(void)
{
    if (service_started || vision_ready) { return RT_EOK; }
    /* Media waits, decompression and CRC run below USB (15) and VM (20). The VM
     * gives this worker service opportunities while the image is not ready. */
    rt_err_t result = rt_thread_init(&vision_thread, "vision", vision_entry, RT_NULL,
                                    vision_stack, sizeof(vision_stack), 25, 10);
    if (result != RT_EOK) { return result; }
    service_started = RT_TRUE;
    result = rt_thread_startup(&vision_thread);
    if (result != RT_EOK)
    {
        service_started = RT_FALSE;
        rt_thread_detach(&vision_thread);
    }
    return result;
}

bool titan_vision_is_ready(void)
{
    return vision_ready != RT_FALSE;
}

bool titan_vision_needs_service(void)
{
    return service_started && !vision_ready;
}
