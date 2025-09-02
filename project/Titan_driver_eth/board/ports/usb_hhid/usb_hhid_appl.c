/***********************************************************************************************************************
 * File Name    : usb_app_thread_entry.c
 * Description  : Contains data structures and functions used in usb_app_thread_entry.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2020 - 2024 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "usb_hhid_appl.h"
#include "common_utils.h"

extern QueueHandle_t g_event_queue;
extern QueueHandle_t g_data_queue;

/*******************************************************************************************************************//**
 * @addtogroup usb_hhid_ep
 * @{
 **********************************************************************************************************************/

/******************************************************************************
 * Private global variables
 ******************************************************************************/
static uint8_t  g_buf[8] = {RESET_VALUE,};              /* USB Receive data */
static uint32_t g_setup_data   = RESET_VALUE;
static volatile bool g_err_flag = false;
uint16_t g_mxps                 = RESET_VALUE;                /* Max Packet Size*/

/* Private function.*/
static fsp_err_t set_protocol(usb_instance_ctrl_t * p_ctrl, uint8_t protocol, uint8_t device_address);

/* App Thread entry function */
/* pvParameters contains TaskHandle_t */
void usb_app_thread_entry(void *pvParameters)
{
    FSP_PARAMETER_NOT_USED (pvParameters);

    fsp_err_t err                    = FSP_SUCCESS;
    /* Variable to capture USB event. */
    usb_event_info_t   * p_usb_event = NULL ;
    BaseType_t         err_queue     = pdFALSE;

    fsp_pack_version_t version       = {RESET_VALUE};

    /* version get API for FLEX pack information */
    R_FSP_VersionGet(&version);

    /* Example Project information printed on the Console */
    rt_kprintf(BANNER_INFO, EP_VERSION, version.version_id_b.major, version.version_id_b.minor, version.version_id_b.patch);
    rt_kprintf(EP_INFO);

    /* open basic USB driver.*/
    err = R_USB_Open (&g_usb_basic_ctrl, &g_usb_basic_cfg);
    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        rt_kprintf ("** R_USB_Open API FAILED **\n");
    }

    while (true)
    {
        /* Handle error if queue send fails*/
        if (true == g_err_flag)
        {
            rt_kprintf ("\nError in sending usb event through queue\n");
        }

        /* Receive message from queue */
        err_queue = xQueueReceive(g_event_queue, &p_usb_event,(portMAX_DELAY));
        /* Handle error */
        if(pdTRUE != err_queue)
        {
            rt_kprintf ("\nError in receiving event through queue\n");
        }

        switch(p_usb_event->event)
        {
            case USB_STATUS_CONFIGURED:
            {
                rt_kprintf("USB Configured Successfully\n");

                /* Get max packet size for the connected HID device. The max packet size is set to the area.
                 * Set the direction (USB_HID_IN/USB_HID_OUT).*/
                err = R_USB_HHID_MaxPacketSizeGet (&g_usb_basic_ctrl, &g_mxps, USB_HID_IN, p_usb_event->device_address);
                /* Handle error */
                if (FSP_SUCCESS != err)
                {
                    rt_kprintf ("** R_USB_HHID_MaxPacketSizeGet API FAILED **\n");
                }

                /* Send the HID request(SetProtocol) to HID device */
                err = set_protocol (&g_usb_basic_ctrl, BOOT_PROTOCOL, p_usb_event->device_address);
                /* Handle error */
                if (FSP_SUCCESS != err)
                {
                    rt_kprintf ("** Send the HID request(SetProtocol) to HID device FAILED **\n");
                }

                break;
            }

            case USB_STATUS_READ_COMPLETE:
            {
                /* Send data received to queue */
                err_queue = xQueueSend(g_data_queue, &g_buf[RESET_VALUE], portMAX_DELAY);
                /* Handle error */
                if (pdTRUE != err_queue)
                {
                    rt_kprintf ("\nError in sending usb data through queue\n");
                }

                /* Send data receive request to hhid device.*/
                err = R_USB_Read (&g_usb_basic_ctrl, g_buf, (uint32_t) g_mxps, p_usb_event->device_address);
                /* Handle error */
                if (FSP_SUCCESS != err)
                {
                    rt_kprintf ("** R_USB_Read API FAILED **\n");
                }

                break;
            }

            case USB_STATUS_REQUEST_COMPLETE:
            {
                rt_kprintf("USB Status Request Completed Successfully\n");

                /* Check for USB class request type is USB_HID_SET_PROTOCOL.*/
                if (USB_HID_SET_PROTOCOL == (p_usb_event->setup.request_type & USB_BREQUEST))
                {
                    /* Send data receive request to hhid device.*/
                    err = R_USB_Read (&g_usb_basic_ctrl, g_buf, (uint32_t) g_mxps, p_usb_event->device_address);
                    /* Handle error */
                    if (FSP_SUCCESS != err)
                    {
                        rt_kprintf ("** R_USB_Read API FAILED **\n");
                    }
                }
                rt_kprintf("\nEnter any key as specified from a to z, A to Z, 0 to 9 and any special character on keyboard\n\n");
                break;
            }

            case USB_STATUS_DETACH:
            {
                rt_kprintf("USB Removed Successfully\n");
                break;
            }

            default:
            {
                break;
            }
        }
    }
}

/*******************************************************************************************************************//**
 * @brief       Sending SetProtocol request to HID device.
 * @param[IN] : usb_ctrl_t   *p_ctrl : Pointer to usb_instance_ctrl_t structure.
 *            : uint8_t      protocol: Protocol Type
 *            : uint8_t      device_address: Device address that sends this request
 * @retval      FSP_SUCCESS  Upon successful Sending SetProtocol request to HID device.
 * @retval      Any Other Error code apart from FSP_SUCCES on Unsuccessful Sending SetProtocol request to HID device.
 **********************************************************************************************************************/
static fsp_err_t set_protocol(usb_instance_ctrl_t *p_ctrl, uint8_t protocol, uint8_t device_address)
{
    usb_setup_t setup;
    fsp_err_t err = FSP_SUCCESS;

    setup.request_type   = SET_PROTOCOL;  /* bRequestCode:SET_PROTOCOL, bmRequestType */
    setup.request_value  = protocol;      /* wValue: Protocol Type */
    setup.request_index  = SET_ZERO;      /* wIndex:Interface */
    setup.request_length = SET_ZERO;      /* wLength:Zero */

    err = R_USB_HostControlTransfer (p_ctrl, &setup, (uint8_t *) &g_setup_data, device_address); /* Request Control transfer */
    if (FSP_SUCCESS != err)
    {
        rt_kprintf ("** R_USB_HostControlTransfer API FAILED **\n");
    }
    return err;
}

/*******************************************************************************************************************//**
 * @brief     This function is callback for FreeRTOS+HHID.
 * @param[IN]   usb_event_info_t  *p_event_info
 * @param[IN]   usb_hdl_t         handler
 * @param[IN]   usb_onoff_t       on_off
 * @retval      None.
 ***********************************************************************************************************************/
void usb_apl_callback(usb_event_info_t * p_event_info, usb_hdl_t handler, usb_onoff_t on_off)
{

    FSP_PARAMETER_NOT_USED (handler);
    FSP_PARAMETER_NOT_USED (on_off);

    rt_interrupt_enter();

    /* Send event received to queue */
    if (pdTRUE != (xQueueSend(g_event_queue, (const void *)&p_event_info, (TickType_t)(RESET_VALUE))))
    {
        g_err_flag = true;
    }

    rt_interrupt_leave();
}

/*******************************************************************************************************************//**
 * This function is called to do closing of usb module using its HAL level API.
 * @brief     Close the usb module. Handle the Error internally with Proper Message.
 *            Application handles the rest.
 * @param[IN] None
 * @retval    None
 **********************************************************************************************************************/
void deinit_usb(void)
{
    fsp_err_t err = FSP_SUCCESS;

    /* close opened USB module */
    err = R_USB_Close(&g_usb_basic_ctrl);
    /* Handle error */
    if(FSP_SUCCESS != err)
    {
        rt_kprintf ("** R_USB_Close API FAILED **\n");
    }
}

/*******************************************************************************************************************//**
 * @} (end addtogroup usb_hhid_ep)
 **********************************************************************************************************************/
