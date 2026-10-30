/* SPDX-License-Identifier: Apache-2.0 */
#ifndef TITAN_ORIENTATION_H
#define TITAN_ORIENTATION_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum {
    TITAN_ORIENTATION_GRAY8,
    TITAN_ORIENTATION_RGB565,
    TITAN_ORIENTATION_YUYV422,
} titan_orientation_format_t;

/* Map a crop in the oriented full image back to the sensor image. Both
 * dimensions must be nonzero. On failure the output coordinates are untouched.
 * Packed YUYV callers must additionally enforce even frame/crop width and x.
 */
bool titan_orientation_map_roi(uint16_t frame_width, uint16_t frame_height,
                               uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                               bool flip_x, bool flip_y, uint16_t *raw_x, uint16_t *raw_y);

/* Reorder detached, contiguous pixels in place, after grayscale extraction and
 * stride packing. RGB565 words and YUYV chroma values are never converted.
 * YUYV width must be even. Invalid arguments leave the whole buffer untouched.
 * The caller owns the buffer exclusively; DMA must no longer write to it.
 */
bool titan_orientation_apply(uint8_t *pixels, size_t capacity, uint16_t width, uint16_t height,
                             titan_orientation_format_t format, bool flip_x, bool flip_y);

#endif
