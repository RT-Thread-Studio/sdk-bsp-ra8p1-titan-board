/* SPDX-License-Identifier: Apache-2.0 */
#ifndef RA8_USB_MSC_H
#define RA8_USB_MSC_H
#include <stdbool.h>
#include <stdint.h>

int ra8_usb_msc_init(void);
/* USB task only, after tud_task_ext() has drained the queued bus events. */
void ra8_usb_msc_poll(void);
/* Bus ISR and TinyUSB class/BOT reset: invalidate, never recycle live I/O. */
void ra8_usb_msc_reset(void);
/* Let a continuously running VM give the lower priority Flash worker a tick. */
bool ra8_usb_msc_needs_service(void);
/* Generated MSC adapter: complete immediately in the USB task. */
void ra8_msc_complete_now(int32_t result);

#endif
