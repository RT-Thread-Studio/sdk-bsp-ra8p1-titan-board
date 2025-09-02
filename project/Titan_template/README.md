# Titan Board BSP Description

**English** | [**Chinese**](./README_zh.md)

## Introduction

This document provides the BSP (Board Support Package) description for the RT-Thread **Titan Board** development board. By reading the *Quick Start* section, developers can quickly get started with this BSP and run RT-Thread on the development board.

The main contents are as follows:

- Introduction to the development board
- BSP Quick Start Guide

## Development Board Introduction

The **Titan Board** development board is based on the Renesas RA8P1 and is designed for embedded system application development through flexible configuration of software packages and IDEs.

The front view of the development board is shown below:

![big](figures/big.png)

The commonly used **on-board resources** of this development board are as follows:

- **MCU**: R7KA8P1KFLCAC, high-performance dual-core with 1GHz Arm® Cortex®-M85 and 250MHz Arm® Cortex®-M33, 1MB code MRAM, and 2MB ECC-enabled SRAM.
- **Debug interface**: On-board DAP-Link interface
- **Board interfaces**: 40-pin Raspberry Pi interface, MIPI DSI/CSI, CEU Camera, RGB LCD interface, 2 Ethernet interfaces, 4 CAN motor interfaces, 2 RS485 interfaces, 3 UARTs, 1 DBUS interface, 1 USBHS/USBFS interface, 1 RTC battery interface.

**More detailed information and tools**

## Peripheral Support

The current peripheral support status in this BSP is as follows:

| **On-chip Peripheral** | **Support Status** | **Component**               | **Support Status** |
| ---------------------- | ------------------ | --------------------------- | ------------------ |
| UART                   | Supported          | LWIP                        | Supported          |
| GPIO                   | Supported          | TCP/UDP                     | Supported          |
| HWTIMER                | Supported          | MQTT                        | Supported          |
| I2C                    | Supported          | TFTP                        | Supported          |
| WDT                    | Supported          | Telnet                      | Supported          |
| RTC                    | Supported          | **Multicore Communication** | **Support Status** |
| ADC                    | Supported          | RPmsg-Lite                  | Supported          |
| DAC                    | Supported          |                             |                    |
| SPI                    | Supported          |                             |                    |
| RS485                  | Supported          |                             |                    |
| CANFD                  | Supported          |                             |                    |
| SDHI                   | Supported          |                             |                    |

## User Guide

The user guide is divided into the following two sections:

- **Quick Start**

  This section is intended for beginners who are just getting started with RT-Thread. By following simple steps, you can run the RT-Thread operating system on this development board and observe the experimental results.

- **Advanced Usage**

  This section is intended for developers who need to use more board resources on the RT-Thread operating system. By using the FSP and RT-Thread Settings tools to configure the project, more on-board resources can be enabled to achieve advanced functionality.

### Quick Start

This BSP currently provides an RT-Thread Studio project. Below, we use the RT-Thread Studio development environment as an example to illustrate how to get the system running.

**Create a Project**

- Download the Titan Board resource package and **GNU Tools for ARM Embedded Processors v13.2.1** (if the download fails, switch the resource download source of RT-Thread Studio to the GitHub source and try downloading again).

![image-20250820103337128](figures/image-20250820103337128.png)

​		Switch the resource download source

![image-20250820094820852](figures/image-20250820094820852.png)

- Create a new RT-Thread project.

![image-20250819171350806](figures/image-20250819171350806.png)

![image-20250819172332143](figures/image-20250819172332143.png)

**Hardware Connection**

Use a USB cable to connect the development board to the PC, and use the DAP-Link interface to download and debug the program.

**Build and Download**

![image-20250819172013186](figures/image-20250819172013186.png)

**View Running Results**

After the program is successfully downloaded, the system will automatically run and print system information.

Connect the development board’s corresponding serial port to the PC, open the corresponding serial port in a terminal tool (115200-8-1-N), and reset the device. You will then see the RT-Thread output information. Enter the `help` command to view the commands supported in the system.

```bash
 \ | /
- RT -     Thread Operating System
 / | \     5.2.0 build Aug 5 2025 17:24:30
 2006 - 2024 Copyright by RT-Thread team

==================================================
Hello, Titan Board!
==================================================
msh >ps
thread       pri  status      sp     stack size max used left tick   error  tcb addr
------------ ---  ------- ---------- ----------  ------  ---------- ------- ----------
tshell        20  running 0x000000cc 0x00001000    16%   0x0000000a OK      0x22022568
sys workq     23  suspend 0x000000c4 0x00000800    11%   0x0000000a OK      0x22021a60
tidle0        31  ready   0x000000a4 0x00000100    67%   0x00000011 OK      0x220203c8
timer          4  suspend 0x000000a4 0x00000200    38%   0x00000009 EINTRPT 0x22020818
main          10  suspend 0x000000dc 0x00000800    18%   0x00000007 EINTRPT 0x22021128
msh >
```

**Application Entry Function**

The entry function of the application layer is located in **src\hal_entry.c** within `void hal_entry(void)`. User-created source files can be placed directly in the **src** directory.

```c
void hal_entry(void)
{
    rt_kprintf("\n==================================================\n");
    rt_kprintf("Hello, Titan Board!\n");
    rt_kprintf("==================================================\n");

    while (1)
    {
        rt_pin_write(LED_PIN_0, PIN_HIGH);
        rt_thread_mdelay(1000);
        rt_pin_write(LED_PIN_0, PIN_LOW);
        rt_thread_mdelay(1000);
    }
}
```

### Advanced Usage

**Resources and Documentation**

- [Development Board Official Homepage](https://www.renesas.cn/zh/products/ra8p1)
- [Development Board Datasheet](https://www.renesas.cn/zh/document/dst/25574255?r=25574019)
- [Development Board Hardware Manual](https://www.renesas.cn/zh/document/mah/25574257?r=25574019)
- [Dualcore Development Guide](https://www.renesas.cn/zh/document/apn/developing-ra8-dual-core-mcu?r=25574019)
- [Example Project Bundle](https://www.renesas.cn/zh/document/apn/ek-ra8p1-example-project-bundle?r=25574019)
- [Renesas RA8P1 Group](https://www.renesas.cn/zh/document/fly/renesas-ra8p1-group?r=25574019)

**FSP Configuration**

If you need to modify the Renesas BSP peripheral configuration or add new peripheral interfaces, you will need to use the Renesas [Flexible Software Package (FSP)](https://www.renesas.cn/zh/software-tool/flexible-software-package-fsp?queryID=c20a16b5f7f3866713b62c7acb07f2fc) configuration tool. Please make sure to follow the steps below for configuration. If you encounter any issues, you may ask questions in the [RT-Thread Community Forum](https://club.rt-thread.org/).

1. [Download Flexible Software Package (FSP) | Renesas](https://github.com/renesas/fsp/releases/download/v6.0.0/setup_fsp_v6_0_0_rasc_v2025-04.1.exe), please use **FSP version 6.0.0**
2. Refer to the documentation: [Configuring Peripheral Drivers Using FSP for the RA Series](https://www.rt-thread.org/document/site/#/rt-thread-version/rt-thread-standard/tutorial/make-bsp/renesas-ra/RA系列使用FSP配置外设驱动?id=ra系列使用-fsp-配置外设驱动).

**RT-Thread Settings**

In **RT-Thread Settings**, you can configure the RT-Thread kernel, components, software packages, and Titan Board device drivers.

![image-20250819173700386](figures/image-20250819173700386.png)

## Contact Information

If you have any thoughts or suggestions during usage, please feel free to contact us via the [RT-Thread Community Forum](https://club.rt-thread.org/).

## Contribute Code

If you're interested in Titan Board and have some exciting projects you'd like to share, we welcome code contributions. Please refer to [How to Contribute to RT-Thread Code](https://www.rt-thread.org/document/site/#/rt-thread-version/rt-thread-standard/development-guide/github/github).