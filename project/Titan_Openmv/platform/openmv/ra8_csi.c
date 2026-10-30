/* SPDX-License-Identifier: Apache-2.0 */
#include <string.h>
#include <arm_mve.h>
#include "py/mphal.h"
#include "py/runtime.h"
#include "omv_csi.h"
#include "board_config.h"
#include "ra8_capture.h"
#include "titan_camera.h"
#include "titan_memory.h"
#include "titan_orientation.h"

static bool configured;
static bool pipeline_enabled = true;
static omv_csi_t *active_csi;
static volatile uint32_t capture_generation;
static bool notifying_frame_callback;
static bool snapshot_in_progress;
static vbuffer_t *volatile direct_ready;
static volatile bool direct_fault;
char *framebuffer_pool_start(framebuffer_t *fb, size_t count);

static int ra8_abort(omv_csi_t *csi, bool flush, bool irq) {
    if (irq) {
        titan_camera_quiesce_irq();
        configured = false;
        capture_generation++;
        /* Keep owner and queue pointers until the task-context drain. */
        return 0;
    }
    int result = titan_camera_close();
    if (result) { return result; }
    configured = false;
    active_csi = NULL;
    direct_ready = NULL;
    direct_fault = false;
    capture_generation++;
    if (flush && csi && csi->fb && csi->fb->free_queue) { framebuffer_flush(csi->fb); }
    return 0;
}
int ra8_capture_buffer_change(framebuffer_t *fb) {
    if (active_csi && active_csi->fb == fb) { return ra8_abort(active_csi, false, false); }
    return 0;
}
int ra8_capture_reset_barrier(void) {
    return ra8_abort(active_csi, false, false);
}
bool ra8_capture_pipeline(int enable) {
    bool old = pipeline_enabled;
    if (enable >= 0) { pipeline_enabled = enable != 0; }
    return old;
}
static int ra8_config(omv_csi_t *csi, omv_csi_config_t config) {
    (void)config;
    /* FSP configuration is deferred until actual framebuffer addresses exist. */
    if (active_csi) { return ra8_abort(active_csi, true, false); }
    configured = false;
    capture_generation++;
    return 0;
}
static bool direct_valid(framebuffer_t *fb, vbuffer_t *buffer) {
    if (!buffer || !fb->raw_base || !fb->buf_count || !fb->buf_size) { return false; }
    uintptr_t first = (uintptr_t)framebuffer_pool_start(fb, fb->buf_count);
    uintptr_t pointer = (uintptr_t)buffer;
    size_t stride = fb->buf_size + sizeof(vbuffer_t);
    return pointer >= first && (pointer - first) % stride == 0 &&
        (pointer - first) / stride < fb->buf_count &&
        !((uintptr_t)buffer->data & 127U) &&
        titan_memory_is_bus_accessible(buffer->data, fb->buf_size);
}
/* Called in the VIN IRQ or before capture starts. Never allocates or blocks. */
static uint8_t *direct_take_buffer(void *context) {
    framebuffer_t *fb = context;
    vbuffer_t *buffer = direct_ready;
    if (buffer) {
        direct_ready = NULL;
    } else {
        buffer = framebuffer_acquire(fb, FB_FLAG_FREE);
    }
    if (!buffer) { return NULL; }
    if (!direct_valid(fb, buffer)) { direct_fault = true; return NULL; }
    framebuffer_reset(buffer);
    /* Initial flush and every task-context USED release invalidate data.
     * A recycled private_ready has never been accessed by the CPU. Avoid
     * cache sweeps inside the bank-swap critical section. */
    return buffer->data;
}
static void direct_complete_buffer(void *context, uint8_t *data, uint32_t sequence) {
    framebuffer_t *fb = context;
    vbuffer_t *buffer = (vbuffer_t *)(data - offsetof(vbuffer_t, data));
    (void)sequence;
    if (!direct_valid(fb, buffer)) { direct_fault = true; return; }
    /* Only this private ready pointer may be recycled. The used queue holds
     * the Python-visible frame and is never reclaimed by the interrupt. */
    if (direct_ready) {
        /* take_buffer must consume a previous private_ready first. Do not
         * add an IRQ producer to the task-produced SPSC free queue. */
        direct_fault = true;
        return;
    }
    framebuffer_reset(buffer);
    direct_ready = buffer;
}
static vbuffer_t *direct_claim(framebuffer_t *fb) {
    rt_base_t level = rt_hw_interrupt_disable();
    vbuffer_t *buffer = direct_ready;
    if (!queue_is_empty(fb->used_queue)) {
        direct_fault = true;
        rt_hw_interrupt_enable(level);
        return NULL;
    }
    if (buffer) {
        direct_ready = NULL;
        /* Protect before re-enabling interrupts; core sets this again later. */
        buffer->flags |= VB_FLAG_USED;
        queue_push(fb->used_queue, buffer);
    }
    rt_hw_interrupt_enable(level);
    return buffer;
}

/* VIN normalizes memory to Y0 U Y1 V with output byte swap enabled.
 * Startup copies this hot loop from flash to ITCM. */
__attribute__((section(".itcm_code_from_flash"), noinline))
void ra8_yuv422_to_gray(uint8_t *dst, const uint8_t *src, size_t pixels) {
    while (pixels >= 16) {
        uint8x16x2_t pair = vld2q_u8(src);
        vst1q_u8(dst, pair.val[0]);
        src += 32; dst += 16; pixels -= 16;
    }
    while (pixels--) { *dst++ = *src; src += 2; }
}
/* GCC newlib-nano memcpy in this toolchain is a byte loop. Keep this
 * forward image-only copy explicit so it cannot be folded back into
 * that libc call. The VIN source is detached and DMA will not touch it.
 */
__attribute__((section(".itcm_code_from_flash"), noinline,
               optimize("O3,no-tree-loop-distribute-patterns")))
void titan_image_copy(uint8_t *dst, const uint8_t *src, size_t bytes) {
    while (bytes >= 64) {
        uint8x16_t a = vld1q_u8(src);
        uint8x16_t b = vld1q_u8(src + 16);
        uint8x16_t c = vld1q_u8(src + 32);
        uint8x16_t d = vld1q_u8(src + 48);
        vst1q_u8(dst, a);
        vst1q_u8(dst + 16, b);
        vst1q_u8(dst + 32, c);
        vst1q_u8(dst + 48, d);
        src += 64; dst += 64; bytes -= 64;
    }
    while (bytes >= 16) {
        vst1q_u8(dst, vld1q_u8(src));
        src += 16; dst += 16; bytes -= 16;
    }
    while (bytes--) { *dst++ = *src++; }
}

typedef struct { nlr_jump_callback_node_t node; omv_csi_t *csi; } capture_cleanup_t;
static void capture_cleanup(void *arg) {
    capture_cleanup_t *cleanup = arg;
    notifying_frame_callback = false;
    snapshot_in_progress = false;
    (void)ra8_abort(cleanup->csi, true, false);
}
int omv_csi_snapshot_upstream(omv_csi_t *, image_t *, uint32_t);
/* Guard before core previews/releases the previous image, not merely before
 * the hardware callback. Scheduled Python and frame callbacks may recurse. */
int omv_csi_snapshot(omv_csi_t *csi, image_t *image, uint32_t flags) {
    if (snapshot_in_progress) { return OMV_CSI_ERROR_CTL_UNSUPPORTED; }
    capture_cleanup_t guard = {.csi=csi};
    nlr_push_jump_callback(&guard.node, capture_cleanup);
    snapshot_in_progress = true;
    int result = omv_csi_snapshot_upstream(csi, image, flags);
    snapshot_in_progress = false;
    nlr_pop_jump_callback(false);
    return result;
}
static int ra8_snapshot(omv_csi_t *csi, image_t *image, uint32_t flags) {
    if (notifying_frame_callback || (flags & OMV_CSI_FLAG_NON_BLOCK) || csi->transpose) {
        return OMV_CSI_ERROR_CTL_UNSUPPORTED;
    }
    if (csi->pixformat != PIXFORMAT_RGB565 && csi->pixformat != PIXFORMAT_GRAYSCALE &&
        csi->pixformat != PIXFORMAT_YUV422) { return OMV_CSI_ERROR_PIXFORMAT_UNSUPPORTED; }
    if (csi->fb->u <= 0 || csi->fb->v <= 0 ||
        csi->fb->x + csi->fb->u > csi->resolution[csi->framesize][0] ||
        csi->fb->y + csi->fb->v > csi->resolution[csi->framesize][1]) {
        return OMV_CSI_ERROR_CTL_UNSUPPORTED;
    }
    /* The SDK's color-correct sensor output is vertically reflected relative
     * to OpenMV's default. Fix geometry after VIN has completed color
     * conversion, without changing the sensor's Bayer/ISP phase. */
    const bool flip_x = csi->hmirror;
    const bool flip_y = !csi->vflip;
    uint16_t raw_x, raw_y;
    if (!titan_orientation_map_roi(csi->resolution[csi->framesize][0],
                                  csi->resolution[csi->framesize][1],
                                  csi->fb->x, csi->fb->y, csi->fb->u, csi->fb->v,
                                  flip_x, flip_y, &raw_x, &raw_y)) {
        return OMV_CSI_ERROR_CTL_UNSUPPORTED;
    }
    size_t bpp = csi->pixformat == PIXFORMAT_GRAYSCALE ? 1U : 2U;
    size_t stride = ((size_t)csi->fb->u + 15U) & ~(size_t)15U;
    stride *= 2U;
    size_t raw_bytes = stride * csi->fb->v;
    if (csi->fb->buf_size < raw_bytes || csi->fb->buf_count < 5) {
        size_t count = csi->fb->buf_count < 5 ? 5 : csi->fb->buf_count;
        if (framebuffer_resize(csi->fb, count, raw_bytes)) {
            return OMV_CSI_ERROR_FRAMEBUFFER_OVERFLOW;
        }
    }
    if (!configured || active_csi != csi) {
        if (active_csi && ra8_abort(active_csi, true, false)) {
            return OMV_CSI_ERROR_CAPTURE_FAILED;
        }
        framebuffer_flush(csi->fb);
        direct_ready = NULL;
        direct_fault = false;

        active_csi = csi;
        titan_camera_config_t config = {
            .x=raw_x, .y=raw_y, .width=csi->fb->u, .height=csi->fb->v,
            .rgb565=csi->pixformat == PIXFORMAT_RGB565
        };
        titan_camera_sink_t sink = {
            .context=csi->fb, .take_buffer=direct_take_buffer,
            .complete_buffer=direct_complete_buffer
        };
        if (titan_camera_open(&config, &sink, csi->fb->buf_size)) {
            if (!ra8_abort(csi, false, false)) { framebuffer_flush(csi->fb); }
            return OMV_CSI_ERROR_CSI_INIT_FAILED;
        }
        configured = true;
    }
    if (titan_camera_start()) { ra8_abort(csi, true, false); return OMV_CSI_ERROR_CAPTURE_FAILED; }
    uint32_t generation = capture_generation;
    uint32_t start_ms = mp_hal_ticks_ms();
    vbuffer_t *buffer = NULL;
    capture_cleanup_t cleanup = {.csi=csi};
    nlr_push_jump_callback(&cleanup.node, capture_cleanup);

    while (!(buffer = direct_claim(csi->fb))) {
        if (direct_fault || titan_camera_error()) { break; }
        mp_event_handle_nowait();
        if (!configured || active_csi != csi || capture_generation != generation) { break; }
        if ((uint32_t)(mp_hal_ticks_ms() - start_ms) >= OMV_CSI_TIMEOUT_MS) { break; }
        titan_camera_wait(1);
    }
    nlr_pop_jump_callback(false);

    if (!buffer || direct_fault || titan_camera_error() || capture_generation != generation) {
        int error = OMV_CSI_ERROR_CAPTURE_TIMEOUT;
        const char *reason = "frame timeout";
        if (direct_fault) {
            error = OMV_CSI_ERROR_FRAMEBUFFER_ERROR;
            reason = "framebuffer ownership error";
        } else if (titan_camera_error()) {
            error = OMV_CSI_ERROR_CAPTURE_FAILED;
            reason = "capture hardware error";
        } else if (!configured || active_csi != csi || capture_generation != generation) {
            error = OMV_CSI_ERROR_CAPTURE_FAILED;
            reason = "capture configuration changed";
        }
        titan_camera_report_error(reason, (unsigned)(mp_hal_ticks_ms() - start_ms));
        if (ra8_abort(csi, true, false)) { return OMV_CSI_ERROR_CAPTURE_FAILED; }
        return error;
    }

    /* DMA has detached this buffer. Invalidate before the CPU touches pixels.
     * Its header and neighbouring buffers own separate cache lines. */

    titan_cache_invalidate(buffer->data, csi->fb->buf_size);

    size_t row_bytes = (size_t)csi->fb->u * bpp;
    if (bpp == 1 || stride != row_bytes) {
        for (unsigned y = 0; y < (unsigned)csi->fb->v; y++) {
            uint8_t *dst = buffer->data + y * row_bytes;
            const uint8_t *src = buffer->data + y * stride;
            if (bpp == 1) { ra8_yuv422_to_gray(dst, src, csi->fb->u); }
            else if (dst != src) { titan_image_copy(dst, src, row_bytes); }
        }
    } else {
        /* The DMA rows are already packed; no stride compaction needed. */
    }
    /* Gray must be compacted first: reversing while consuming the original
     * 2-byte Y/C rows could overwrite input rows which have not been read.
     * This buffer is detached from every VIN mailbox and CPU-owned here. */
    titan_orientation_format_t orientation_format =
        csi->pixformat == PIXFORMAT_GRAYSCALE ? TITAN_ORIENTATION_GRAY8 :
        (csi->pixformat == PIXFORMAT_RGB565 ? TITAN_ORIENTATION_RGB565 : TITAN_ORIENTATION_YUYV422);
    if (!titan_orientation_apply(buffer->data, csi->fb->buf_size,
                                 csi->fb->u, csi->fb->v, orientation_format, flip_x, flip_y)) {
        ra8_abort(csi, true, false);
        return OMV_CSI_ERROR_FRAMEBUFFER_ERROR;
    }
    csi->fb->w = csi->fb->u; csi->fb->h = csi->fb->v;
    csi->fb->pixfmt = csi->pixformat;
    if (csi->pixformat == PIXFORMAT_YUV422) { csi->fb->subfmt_id = csi->yuv_format; }
    framebuffer_to_image(csi->fb, image);
    if (!pipeline_enabled && ra8_abort(csi, false, false)) { return OMV_CSI_ERROR_CAPTURE_FAILED; }
    omv_csi_cb_t callback = csi->frame_cb;
    if (callback.fun) {
        uint32_t callback_generation = capture_generation;
        nlr_push_jump_callback(&cleanup.node, capture_cleanup);
        notifying_frame_callback = true;
        callback.fun(callback.arg);
        notifying_frame_callback = false;
        nlr_pop_jump_callback(false);
        if (capture_generation != callback_generation) { return OMV_CSI_ERROR_FRAMEBUFFER_ERROR; }
    }

    return 0;
}
static int ra8_clock_set(omv_clk_t *clk, uint32_t frequency) {
    (void)clk;
    return titan_camera_clock_start(frequency);
}
int omv_csi_ops_init(omv_csi_t *csi) {
    csi->config = ra8_config; csi->abort = ra8_abort; csi->snapshot = ra8_snapshot;
    csi->clk->set_freq = ra8_clock_set;
    return 0;
}
/* CSI-2 has no external VSYNC GPIO in this port. The upstream Python setter
 * ignores integer error returns, so reject registration explicitly.
 */
int omv_csi_set_vsync_callback(omv_csi_t *csi, omv_csi_cb_t cb) {
    if (cb.fun) {
        mp_raise_msg(&mp_type_NotImplementedError, MP_ERROR_TEXT("MIPI VSYNC callback is not implemented"));
    }
    csi->vsync_cb = cb;
    return 0;
}
