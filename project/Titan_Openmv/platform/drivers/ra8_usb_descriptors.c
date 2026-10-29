/* SPDX-License-Identifier: MIT
 * CDC + SD mass-storage descriptors for high and full speed.
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 */
#include <rtthread.h>
#include "tusb.h"
#include <string.h>
#include "hal_data.h"

#if CFG_TUD_CDC != 1 || CFG_TUD_MSC != 1 || CFG_TUD_HID || CFG_TUD_VIDEO
#error "Review the platform USB descriptors when adding device classes"
#endif
#if PKG_TINYUSB_DEVICE_MSC_EPNUM == PKG_TINYUSB_DEVICE_CDC_EPNUM || \
    PKG_TINYUSB_DEVICE_MSC_EPNUM == PKG_TINYUSB_DEVICE_CDC_EPNUM_NOTIF || \
    PKG_TINYUSB_DEVICE_MSC_EPNUM < 1 || PKG_TINYUSB_DEVICE_MSC_EPNUM > 15
#error "MSC requires a distinct, nonzero USB endpoint number"
#endif
#if CFG_TUD_CDC_RX_BUFSIZE < 512 || CFG_TUD_CDC_TX_BUFSIZE < 512
#error "HS CDC FIFOs must accommodate a 512-byte bulk packet"
#endif

static const tusb_desc_device_t desc_device = {
    .bLength = sizeof(tusb_desc_device_t), .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200, .bDeviceClass = TUSB_CLASS_MISC,
    .bDeviceSubClass = MISC_SUBCLASS_COMMON, .bDeviceProtocol = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor = PKG_TINYUSB_DEVICE_VID, .idProduct = PKG_TINYUSB_DEVICE_PID,
    .bcdDevice = 0x0200, .iManufacturer = 1, .iProduct = 2, .iSerialNumber = 3,
    .bNumConfigurations = 1,
};
const uint8_t *tud_descriptor_device_cb(void) { return (const uint8_t *)&desc_device; }

#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN + TUD_MSC_DESC_LEN)
#define COMPOSITE_CONFIG(packet_size) \
    TUD_CONFIG_DESCRIPTOR(1, 3, 0, CONFIG_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, PKG_TINYUSB_DEVICE_CURRENT), \
    TUD_CDC_DESCRIPTOR(0, 4, 0x80 | PKG_TINYUSB_DEVICE_CDC_EPNUM_NOTIF, 8, \
        PKG_TINYUSB_DEVICE_CDC_EPNUM, 0x80 | PKG_TINYUSB_DEVICE_CDC_EPNUM, packet_size), \
    TUD_MSC_DESCRIPTOR(2, 5, PKG_TINYUSB_DEVICE_MSC_EPNUM, \
        0x80 | PKG_TINYUSB_DEVICE_MSC_EPNUM, packet_size)
static const uint8_t desc_fs_configuration[] = {COMPOSITE_CONFIG(64)};
#if TUD_OPT_HIGH_SPEED
static const uint8_t desc_hs_configuration[] = {COMPOSITE_CONFIG(512)};
static uint8_t desc_other_speed_config[CONFIG_TOTAL_LEN];
static const tusb_desc_device_qualifier_t desc_device_qualifier = {
    .bLength = sizeof(tusb_desc_device_qualifier_t), .bDescriptorType = TUSB_DESC_DEVICE_QUALIFIER,
    .bcdUSB = 0x0200, .bDeviceClass = TUSB_CLASS_MISC,
    .bDeviceSubClass = MISC_SUBCLASS_COMMON, .bDeviceProtocol = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE, .bNumConfigurations = 1, .bReserved = 0,
};
const uint8_t *tud_descriptor_device_qualifier_cb(void) { return (const uint8_t *)&desc_device_qualifier; }
const uint8_t *tud_descriptor_other_speed_configuration_cb(uint8_t index)
{
    if (index != 0) { return NULL; }
    memcpy(desc_other_speed_config, tud_speed_get() == TUSB_SPEED_HIGH ? desc_fs_configuration : desc_hs_configuration,
           sizeof(desc_other_speed_config));
    desc_other_speed_config[1] = TUSB_DESC_OTHER_SPEED_CONFIG;
    return desc_other_speed_config;
}
#endif
const uint8_t *tud_descriptor_configuration_cb(uint8_t index)
{
    if (index != 0) { return NULL; }
#if TUD_OPT_HIGH_SPEED
    if (tud_speed_get() == TUSB_SPEED_HIGH) { return desc_hs_configuration; }
#endif
    return desc_fs_configuration;
}

static const char *const strings[] = {
    NULL, PKG_TINYUSB_DEVICE_MANUFACTURER, PKG_TINYUSB_DEVICE_PRODUCT,
    "00000000000000000000", PKG_TINYUSB_DEVICE_CDC_STRING, PKG_TINYUSB_DEVICE_MSC_STRING,
};
static uint16_t desc_string[32];
const uint16_t *tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    (void)langid;
    size_t length;
    if (index == 0) { desc_string[1] = 0x0409; length = 1; }
    else if (index == 3) {
        /* Stable per-board serial from the MCU UID, shared with machine.unique_id.
         * A constant serial makes Windows cache multiple boards as one device.
         */
        static const char hex[] = "0123456789ABCDEF";
        const uint8_t *uid = (const uint8_t *)R_BSP_UniqueIdGet();
        length = 24;
        for (size_t i = 0; i < length / 2; ++i) {
            desc_string[1 + 2 * i] = hex[uid[i] >> 4];
            desc_string[2 + 2 * i] = hex[uid[i] & 15];
        }
    }
    else {
        if (index >= sizeof(strings) / sizeof(strings[0]) || !strings[index]) { return NULL; }
        length = strlen(strings[index]);
        if (length > 31) { length = 31; }
        for (size_t i = 0; i < length; ++i) { desc_string[i + 1] = (uint8_t)strings[index][i]; }
    }
    desc_string[0] = (TUSB_DESC_STRING << 8) | (2 * length + 2);
    return desc_string;
}
