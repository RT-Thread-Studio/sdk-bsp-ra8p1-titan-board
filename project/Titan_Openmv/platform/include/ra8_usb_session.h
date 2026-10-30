/* SPDX-License-Identifier: Apache-2.0 */
#ifndef RA8_USB_SESSION_H
#define RA8_USB_SESSION_H
#include <stddef.h>
int ra8_usb_serial_active(void);
size_t ra8_usb_serial_write(const char *data, size_t size);
void ra8_usb_session_poll(void);
#endif
