/* SPDX-License-Identifier: Apache-2.0 */
#include "framebuffer.h"
#include "ra8_capture.h"
#include "py/runtime.h"
#include "umalloc.h"
static int ra8_framebuffer_resize_upstream(framebuffer_t *, size_t, size_t);
static void ra8_framebuffer_flush_upstream(framebuffer_t *);
static vbuffer_t *ra8_framebuffer_release_upstream(framebuffer_t *, uint32_t);
#define framebuffer_resize ra8_framebuffer_resize_upstream
#define framebuffer_flush ra8_framebuffer_flush_upstream
#define framebuffer_release ra8_framebuffer_release_upstream
#include "../port/titan_framebuffer_core.c"
#undef framebuffer_resize
#undef framebuffer_flush
#undef framebuffer_release

/* Header/wire ABI stays 32-byte aligned. The main pool starts each header
 * 96 bytes into a 128-byte block, putting its data on a VIN-safe boundary. */
_Static_assert(sizeof(vbuffer_t) == 32, "Review VIN framebuffer header padding");
int framebuffer_resize(framebuffer_t *fb, size_t count, size_t frame_size) {
    if (ra8_capture_buffer_change(fb)) { return -1; }
    if (fb != framebuffer_get(FB_MAINFB_ID)) {
        return ra8_framebuffer_resize_upstream(fb, count, frame_size);
    }
    if (count < 5) { count = 5; }
    if (count > UINT32_MAX / 16U || frame_size > SIZE_MAX - sizeof(vbuffer_t) - 127U) { return -1; }
    size_t stride = OMV_ALIGN_TO(frame_size + sizeof(vbuffer_t), 128);
    size_t queue_size = queue_calc_size(count);
    if (queue_size > (SIZE_MAX - 127U) / 2U) { return -1; }
    size_t prefix = OMV_ALIGN_TO(queue_size * 2U, 128) + 128U - sizeof(vbuffer_t);
    if (!stride || count > (SIZE_MAX - prefix) / stride) { return -1; }
    size_t minimum = prefix + stride * count;
    if (fb->dynamic) {
        if (fb->raw_base) { uma_free(fb->raw_base); }
        fb->raw_base = NULL;
        fb->raw_size = fb->buf_size = fb->buf_count = 0;
        fb->free_queue = fb->used_queue = NULL;
        fb->pixfmt = PIXFORMAT_INVALID;
        void *allocation = uma_malign(minimum, 128, UMA_PERSIST | UMA_MAYBE);
        if (!allocation) { return -1; }
        fb->raw_base = allocation;
        fb->raw_size = minimum;
    }
    if (minimum > fb->raw_size || ((uintptr_t)fb->raw_base & 127U)) { return -1; }
    fb->buf_count = count;
    fb->buf_size = stride - sizeof(vbuffer_t);
    queue_init(&fb->free_queue, count, fb->raw_base);
    queue_init(&fb->used_queue, count, fb->raw_base + queue_size);
    ra8_framebuffer_flush_upstream(fb);
    return 0;
}
void framebuffer_flush(framebuffer_t *fb) {
    if (ra8_capture_buffer_change(fb)) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("VIN DMA has not stopped; framebuffer retained"));
    }
    ra8_framebuffer_flush_upstream(fb);
}
vbuffer_t *framebuffer_release(framebuffer_t *fb, uint32_t flags) {
    /* Every buffer returned by the task to VIN's free queue must have no
     * dirty CPU data left. Its header owns a different cache line. */
    if (fb == framebuffer_get(FB_MAINFB_ID) && (flags & FB_FLAG_USED)) {
        flags |= FB_FLAG_INVALIDATE;
    }
    return ra8_framebuffer_release_upstream(fb, flags);
}

/* The official Python image helper ignores resize's integer result. Bind only
 * that translation unit to this checked entry; camera callers keep the normal
 * error-return API so they can retain DMA ownership on failure. */
int ra8_framebuffer_resize_or_raise(framebuffer_t *fb, size_t count, size_t frame_size) {
    int result = framebuffer_resize(fb, count, frame_size);
    if (result != 0) {
        mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("Framebuffer resize failed"));
    }
    return result;
}
