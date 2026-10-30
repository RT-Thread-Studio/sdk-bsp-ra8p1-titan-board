/* SPDX-License-Identifier: Apache-2.0 */
#include "titan_orientation.h"

#if defined(__ARM_FEATURE_MVE) && (__ARM_FEATURE_MVE & 1)
#include <arm_mve.h>
#endif

bool titan_orientation_map_roi(uint16_t frame_width, uint16_t frame_height,
                               uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                               bool flip_x, bool flip_y, uint16_t *raw_x, uint16_t *raw_y) {
    if (!raw_x || !raw_y || !width || !height || x > frame_width || y > frame_height ||
        width > frame_width - x || height > frame_height - y) {
        return false;
    }
    *raw_x = flip_x ? frame_width - x - width : x;
    *raw_y = flip_y ? frame_height - y - height : y;
    return true;
}

static void swap_rows(uint8_t *top, uint8_t *bottom, size_t bytes) {
#if defined(__ARM_FEATURE_MVE) && (__ARM_FEATURE_MVE & 1)
    /* Unaligned byte-vector accesses also cover odd-width grayscale rows. */
    while (bytes >= 16U) {
        uint8x16_t a = vld1q_u8(top);
        uint8x16_t b = vld1q_u8(bottom);
        vst1q_u8(top, b);
        vst1q_u8(bottom, a);
        top += 16; bottom += 16; bytes -= 16;
    }
#endif
    while (bytes--) {
        uint8_t value = *top;
        *top++ = *bottom;
        *bottom++ = value;
    }
}

static void mirror_row(uint8_t *row, uint16_t width, titan_orientation_format_t format) {
    if (format == TITAN_ORIENTATION_YUYV422) {
        /* Reverse complete pairs, then exchange their luma samples. Reversing
         * individual 16-bit words would exchange U and V and change colors.
         */
        unsigned pairs = width / 2U;
        for (unsigned left = 0; left < pairs / 2U; left++) {
            uint8_t *a = row + left * 4U;
            uint8_t *b = row + (pairs - left - 1U) * 4U;
            uint8_t a0 = a[0], au = a[1], a1 = a[2], av = a[3];
            a[0] = b[2]; a[1] = b[1]; a[2] = b[0]; a[3] = b[3];
            b[0] = a1; b[1] = au; b[2] = a0; b[3] = av;
        }
        if (pairs & 1U) {
            uint8_t *middle = row + (pairs / 2U) * 4U;
            uint8_t value = middle[0];
            middle[0] = middle[2]; middle[2] = value;
        }
    } else {
        unsigned bytes_per_pixel = format == TITAN_ORIENTATION_GRAY8 ? 1U : 2U;
        for (unsigned left = 0; left < width / 2U; left++) {
            uint8_t *a = row + left * bytes_per_pixel;
            uint8_t *b = row + (width - left - 1U) * bytes_per_pixel;
            for (unsigned byte = 0; byte < bytes_per_pixel; byte++) {
                uint8_t value = a[byte]; a[byte] = b[byte]; b[byte] = value;
            }
        }
    }
}

bool titan_orientation_apply(uint8_t *pixels, size_t capacity, uint16_t width, uint16_t height,
                             titan_orientation_format_t format, bool flip_x, bool flip_y) {
    if (!pixels || !width || !height ||
        (format != TITAN_ORIENTATION_GRAY8 && format != TITAN_ORIENTATION_RGB565 &&
         format != TITAN_ORIENTATION_YUYV422) ||
        (format == TITAN_ORIENTATION_YUYV422 && (width & 1U))) {
        return false;
    }
    size_t row_bytes = (size_t)width * (format == TITAN_ORIENTATION_GRAY8 ? 1U : 2U);
    /* Division keeps the capacity check safe even on 32-bit size_t targets. */
    if (height > capacity / row_bytes) { return false; }
    if (flip_y) {
        for (unsigned y = 0; y < height / 2U; y++) {
            swap_rows(pixels + y * row_bytes, pixels + (height - y - 1U) * row_bytes, row_bytes);
        }
    }
    if (flip_x) {
        for (unsigned y = 0; y < height; y++) {
            mirror_row(pixels + y * row_bytes, width, format);
        }
    }
    return true;
}
