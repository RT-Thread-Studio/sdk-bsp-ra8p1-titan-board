#ifndef RA8_OPENMV_BOARD_CONFIG_H
#define RA8_OPENMV_BOARD_CONFIG_H
#include <rtconfig.h>
#include "port_config.h"
/* OpenMV IDE compatibility identity only. Protocol V2 maps SYS_INFO and
 * runtime USB VID/PID to the AE3 display/example profile. This is still the
 * Titan Board RA8P1 port: CPU/UID, pins, memory and capabilities below are real.
 * AE3 Alif dual-core firmware/DFU is not compatible. The official SYS_BOOT
 * falls back to MCU reset; use the Titan SCons/J-Link firmware workflow.
 */
#define OMV_BOARD_ARCH "OPENMV AE3"
#define OMV_BOARD_TYPE "AE3"
#define OMV_BOARD_LED_PINS {0x000d, 0x060d, 0x000c}
#define OMV_BOARD_LED_COUNT (3)
#define OMV_BOARD_LED_ACTIVE_LOW (1)
#define OMV_USB_VID PKG_TINYUSB_DEVICE_VID
#define OMV_USB_PID PKG_TINYUSB_DEVICE_PID
#define OMV_BOARD_UID_ADDR BSP_FEATURE_BSP_UNIQUE_ID_POINTER
#define OMV_BOARD_UID_SIZE (4)
#define OMV_BOARD_UID_OFFSET (4)
#define OMV_JPEG_CODEC_ENABLE (0)
#define OMV_JPEG_QUALITY_LOW (50)
#define OMV_JPEG_QUALITY_HIGH (90)
#define OMV_JPEG_QUALITY_THRESHOLD (320 * 240 * 2)
#define OMV_GPU_ENABLE (0)
#define OMV_OV2640_ENABLE (0)
#define OMV_OV5640_ENABLE (1)
#define OMV_OV5640_CLK_FREQ (25000000)
#define OMV_OV5640_AF_ENABLE (0)
#define OMV_OV5640_PLL_CTRL2 (0x64)
#define OMV_OV5640_PLL_CTRL3 (0x13)
#define OMV_OV7725_ENABLE (0)
#define OMV_OV7725_PLL_CONFIG (0x01)
#define OMV_OV7725_BANDING (0x7F)
#define OMV_OV7670_ENABLE (0)
#define OMV_OV7670_VERSION (70)
#define OMV_OV7670_CLKRC (0)
#define OMV_OV7670_CLK_FREQ (24000000)
#define OMV_MT9V0XX_ENABLE (0)
#define OMV_MT9V0XX_FSYNC_PIN NULL
#define OMV_CSI_MAX_DEVICES (1)
#define OMV_CSI_I2C_ID (0)
#define OMV_CSI_I2C_SPEED OMV_I2C_SPEED_STANDARD
/* J9 GPIO0 drives active-low RESETB; this connector has no PWDN signal. */
#define OMV_CSI_POLARITY_CONFIG { OMV_CSI_ACTIVE_HIGH, OMV_CSI_ACTIVE_LOW }
#define OMV_OV7725_CLK_FREQ (24000000)
#define OMV_MT9V0XX_CLK_FREQ (24000000)
extern const struct ra8_pin ra8_camera_reset_pin;
#define OMV_CSI_RESET_PIN (&ra8_camera_reset_pin)
#define OMV_CSI_CLK_FREQUENCY (25000000)
#define OMV_PROTOCOL_MAX_BUFFER_SIZE (4096)
/* The port registers official descriptors, adding STDIN lifetime hooks. */
#define OMV_PROTOCOL_DEFAULT_CHANNELS (0)
#define OMV_PROTOCOL_HW_CAPS OMV_PROTOCOL_HW_CAPS_MAKE(HAS_DRAM)
#define OMV_USB_IRQN USBHS_USB_INT_RESUME_IRQn
#define OMV_PROFILE_ENABLE (0)
/* RGBDisplay uses the project GLCDC backend and the shared RASC panel timing. */
#define OMV_RGB_DISPLAY_CONTROLLER (1)
#endif
