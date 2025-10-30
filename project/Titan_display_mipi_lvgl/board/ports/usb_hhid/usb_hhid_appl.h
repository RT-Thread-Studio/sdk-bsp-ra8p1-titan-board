/***********************************************************************************************************************
 * File Name    : usb_hhid_appl.h
 * Description  : Contains data structures and functions used in usb_hhid_appl.h.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2020 - 2024 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef USB_HHID_APPL_H_
#define USB_HHID_APPL_H_

#include <Freertos.h>
#include <queue.h>

#include <rtthread.h>

/* macro definition.*/
#define SET_PROTOCOL               (USB_HID_SET_PROTOCOL | USB_HOST_TO_DEV | USB_CLASS | USB_INTERFACE)
#define BOOT_PROTOCOL              (0U)
#define TASK_DELAY                 (1U)
#define NULL_CHAR                  ('\0')
#define ROW_SIZE                   (40U)
#define COL_SIZE                   (25U)
#define SET_ZERO                   (0x0000)
#define ONE                        (1U)
#define INDEX                      (2U)
#define BUFF_SIZE                  (25U)
#define KEY_MOD_LSHIFT             (0x02)
#define KEY_MOD_RSHIFT             (0x20)

extern QueueHandle_t g_event_queue;
extern QueueHandle_t g_data_queue;

#define EP_INFO        "\nThis example project demonstrates basic functionalities of USB Host Human Interface Device \n"

/* Function declaration */
void deinit_usb(void);

void usb_app_thread_entry(void *pvParameters);
void usb_logger_thread_entry(void *pvParameters);

#endif /* USB_HHID_APPL_H_ */
