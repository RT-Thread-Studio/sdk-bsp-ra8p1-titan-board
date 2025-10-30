# sdk-bsp-ra8p1-titan-board

**English** | [**Chinese**](README_zh.md)

## Introduction

`sdk-bsp-ra8p1-titan-board` is the Board Support Package (BSP) provided by the RT-Thread team for the **Titan Board**. It can also serve as a software SDK for user development, enabling developers to build their own applications more easily and conveniently.

The **Titan Board** is a development board launched by RT-Thread, based on the Renesas R7KA8P1 chip featuring a dual-core architecture with Cortex-M85 and Cortex-M33. It provides engineers with a flexible and comprehensive development platform, helping developers gain deeper insights and experiences in the field of embedded IoT.

![img](figures/big.png)

## Directory Structure

```
$ sdk-bsp-ra8p1-titan-board
├── docs
│   ├── ra8p1-datasheet.pdf
│   ├── ra8p1-users-manual.pdf
│   ├── Titan_Board_schematic.pdf
│   └── source
├── figures
├── libraries
├── rt-thread
├── project
│   ├── Titan_basic_blink_led
│   ├── Titan_basic_buzzer
│   ├── Titan_basic_key_irq
│   ├── Titan_component_flash_fs
│   ├── Titan_component_netutils
│   ├── Titan_display_camera_ceu
│   ├── Titan_display_camera_mipi_csi
│   ├── Titan_display_mipi_lvgl
│   ├── Titan_display_rgb_lvgl
│   ├── Titan_driver_adc
│   ├── Titan_driver_can
│   ├── Titan_driver_eth
│   ├── Titan_driver_gpt
│   ├── Titan_driver_hyperram
│   ├── Titan_driver_iic_ist8310
│   ├── Titan_driver_mipi_lcd
│   ├── Titan_driver_rgb_lcd
│   ├── Titan_driver_rs485
│   ├── Titan_driver_rtc
│   ├── Titan_driver_sdcard
│   ├── Titan_driver_spi_bmi088
│   ├── Titan_driver_usb_hhid
│   ├── Titan_driver_usb_pcdc
│   ├── Titan_driver_wdt
│   ├── Titan_driver_wifi
│   ├── Titan_dual_core
│   │   ├── Titan_dual_core0
│   │   ├── Titan_dual_core1
│   ├── Titan_factory
│   ├── Titan_mlc_rpmsg_lite
│   │   ├── Titan_rpmsg_master
│   │   └── Titan_rpmsg_remote
│   ├── Titan_npu_ai_face_detection
│   └── Titan_template
├── README.md
├── README_zh.md
└── sdk-bsp-ra8p1-titan-board.yaml
```

- `sdk-bsp-ra8p1-titan-board.yaml`: Describes the hardware information of the Titan Board.
- `docs`: Schematics, documents, datasheets, etc., related to the development board.
- `libraries` : General peripheral drivers for Titan Board.
- `project`: Example project folder.
- `rt-thread`:  Source code of RT-Thread.

## Usage

`sdk-bsp-ra8p1-titan-board.yaml` supports **RT-Thread Studio** development methods.

## RT-Thread Studio Development Steps

1. Open **RT-Thread Studio** and install the **Titan Board development board support package** (if a newer version is available, it is recommended to install the latest version; the version shown below is for reference only)

2. Create a new Titan Board project by selecting File -> New -> RT-Thread Project -> Based on Development Board. You can create example projects or template projects.

![image-20250820093045202](figures/image-20250820093045202.png)

3. Compile and download the project:

<img src="figures/image-20250820093329972.png" alt="image-20250820093329972" style="zoom: 200%;" />
