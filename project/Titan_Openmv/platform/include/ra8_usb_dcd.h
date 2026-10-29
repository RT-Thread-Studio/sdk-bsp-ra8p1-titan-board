/* SPDX-License-Identifier: Apache-2.0 */
#ifndef RA8_USB_DCD_H
#define RA8_USB_DCD_H
#include <stdbool.h>
/* Queued SETUP ownership remains visible after the ISR acknowledges VALID. */
bool ra8_usb_setup_pending(void);
#endif
