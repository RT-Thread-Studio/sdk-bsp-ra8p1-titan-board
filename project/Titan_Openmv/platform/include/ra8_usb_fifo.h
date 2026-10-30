/* SPDX-License-Identifier: Apache-2.0 */
#ifndef RA8_USB_FIFO_H
#define RA8_USB_FIFO_H
#include <stdint.h>

/* RUSB2 FIFO RAM is addressed in 64-byte blocks. 0..3 belong to DCP;
 * 4..7 are fixed interrupt pipes 6..9. Reserve non-overlapping double-buffer
 * slots for data pipes: 1/2 may carry 1024-byte ISO, 3..5 at most 512 bytes.
 * End block 120 stays within the RA8 USBHS FIFO RAM. FS uses its fixed layout.
 */
static inline uint16_t ra8_usb_fifo_value(unsigned pipe, unsigned packet_size)
{
    static const uint8_t first_block[6] = {0, 8, 40, 72, 88, 104};
    unsigned limit = pipe <= 2 ? 1024 : 512;
    if (!pipe || pipe > 5 || !packet_size || packet_size > limit) { return UINT16_MAX; }
    unsigned blocks = (packet_size + 63U) / 64U;
    return (uint16_t)(((blocks - 1U) << 10) | first_block[pipe]);
}
#endif
