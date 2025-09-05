# Dual-core Project Instructions (Core0)

**English** | [**Chinese**](./README_zh.md)

## Introduction

This project is based on the **Titan Board** development board and runs on the **Cortex-M85 (Core0)**, serving as the primary control template project in a dual-core system.

**Core0 (Cortex-M85 @ 1GHz)**: The high-performance main core, responsible for major system logic, complex computations, and peripheral management.

**Core1 (Cortex-M33 @ 250MHz)**: The low-power auxiliary core, suitable for handling real-time tasks, security services, or low-power scenarios.

This template project provides the basic startup code, RT-Thread support, and peripheral initialization framework for subsequent application development.

**Note**: The **Titan_dual_core0** and **Titan_dual_core1** projects are designed to work together.

## Hardware Description

Core0 uses UART8 (USB-DBG)  as the debug serial port.

![image-20250905140536089](figures/image-20250905140536089.png)

![image-20250905140624456](figures/image-20250905140624456.png)

## FSP Configuration

* Create a `r_sci_b_uart` stack:

![image-20250829150344291](figures/image-20250829150344291.png)

* Configure `r_sci_b_uart`:

![image-20250829150549509](figures/image-20250829150549509.png)

* Configure UART8 pins:

![image-20250829150627517](figures/image-20250829150627517.png)

## RT-Thread Settings Configuration

* Enable the sub-nuclear startup.

![image-20250829132603640](figures/image-20250829132603640.png)

## Example Code Description

The sample program is located in `/src/hal_entry.c` and uses the blue LED (LED_B) to indicate the running status of Core0.

```c
#include <rtthread.h>
#include "hal_data.h"
#include <rtdevice.h>
#include <board.h>

#define LED_PIN     BSP_IO_PORT_00_PIN_12 /* Onboard LED pins */

void hal_entry(void)
{
    rt_kprintf("\n==================================================\n");
    rt_kprintf("This is core0!\n");
    rt_kprintf("Hello, Titan Board!\n");
    rt_kprintf("==================================================\n");

    rt_kprintf("\nThe dual-core project should collaborate with the SDK project for development: << Titan_dual_core1 >>\n");

    while (1)
    {
        rt_pin_write(LED_PIN, PIN_HIGH);
        rt_thread_mdelay(500);
        rt_pin_write(LED_PIN, PIN_LOW);
        rt_thread_mdelay(500);
    }
}
```

## Compilation & Download

* **RT-Thread Studio**: In RT-Thread Studio's package manager, download the Titan Board resource package, create a new project, and compile it.

After compilation, connect the development board's USB-DBG interface to the PC and download the firmware to the development board.

#### Download Methods

Currently, DAP-Link does not support programming the Core1 firmware, so you need to **use the Renesas Flash Programmer tool to program the firmware**. The detailed programming steps are as follows:

1. Download and install the Renesas Flash Programmer tool: [Renesas Flash Programmer (Programming GUI) | Renesas](https://www.renesas.com/en/software-tool/renesas-flash-programmer-programming-gui#downloads).
2. Connect the USB-DEV port of the development board to the PC.
3. Create a project: The development board needs to enter BOOT mode (**hold down the BOOT button and press the RESET button once**).

![image-20250829142936047](figures/image-20250829142936047-17568017299524.png)

4. Select the firmware to be programmed (Core0 and Core1 firmware):

![image-20250902162451353](figures/image-20250902162451353-17568017299526.png)

5. Flashing firmware (at this time, the development board must be in **BOOT** mode):

![image-20250829143543000](figures/image-20250829143543000-17568017299525.png)

## Run Effect

After resetting the development board, the blue LED will keep flashing.

<img src="figures/image-20250829133454177.png" alt="image-20250829133454177" style="zoom:150%;" />