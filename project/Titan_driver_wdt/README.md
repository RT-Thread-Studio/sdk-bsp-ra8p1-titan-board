# WDT Usage Instructions

**English** | [**Chinese**](./README_zh.md)

## Introduction

The Watchdog Timer (WDT) ensures that our code runs as expected and can effectively prevent the program from “running away” due to uncontrollable factors.
 This example demonstrates how to use the **WDT module of the RA8 series MCU** on the **Titan Board**, together with the **RT-Thread WDT driver framework**, to implement watchdog functionality, including timer configuration, feeding the watchdog, and handling timeouts.

## WDT (Watchdog Timer) Instruction

### 1. General Description

The **WDT (Watchdog Timer)** is a **hardware timer used in embedded systems to monitor system operation and prevent the program from hanging or stalling**.

If the MCU or processor does not reset (Kick/Reload) the WDT within the specified period, the WDT will **automatically reset the system**, ensuring it can resume normal operation.

Typical use cases for WDT:

- Preventing software infinite loops or task stalls
- Improving system reliability and stability
- Critical applications such as industrial control, automotive electronics, drones, and communication devices

### 2. Working Principle

1. **Counter Mode**
   - The WDT includes a decrementing counter that starts from an initial value.
   - The MCU must **reset the counter (Reload/Kick)** before it reaches zero.
   - If the counter reaches zero, a **system reset** or **interrupt** is triggered.
2. **Feeding the Watchdog**
   - The MCU periodically writes a specific value to reset the counter.
   - Commonly called “feeding the dog,” this ensures that during normal operation, the WDT does not reset the system.
3. **Reset/Interrupt Mode**
   - **Reset Mode**: The WDT expiration directly resets the MCU.
   - **Interrupt Mode**: The WDT expiration generates an interrupt, allowing software to attempt recovery first.

### 3. Types of WDT

| Type                       | Description                                                  |
| -------------------------- | ------------------------------------------------------------ |
| **Independent WDT (IWDT)** | Has its own clock source; continues running during reset or suspend |
| **Window WDT (WWDT)**      | Requires feeding within a specific time window               |
| **Super WDT**              | Offers higher precision or longer configurable periods       |
| **Software WDT**           | Implemented via software timers, maintained by OS or applications |

### 4. Key Parameters

- **Counter width**: Determines maximum timeout period
- **Timeout**: Time from last feed to system reset trigger
- **Clock source**: Independent low-speed oscillator or system clock
- **Reset method**: Hardware reset or interrupt notification

Example:

- WDT timeout = 500 ms
- MCU feeds the WDT every 100 ms
- If the MCU stalls and fails to feed the WDT for 500 ms → WDT resets the system

### 5. Usage Guidelines

1. **Feed periodically**: Ensure WDT is refreshed during normal operation.
2. **Feed in critical tasks**: Typically in the main loop or key tasks.
3. **Interrupt-safe**: Feeding should not be blocked by interrupts.
4. **Reasonable timeout**: Must be longer than task execution time but not excessively long.

## RA8 Series WDT Module (r_wdt) Instruction

The RA8 MCU series integrates a high-performance **WDT module (r_wdt)**, providing reliable system monitoring with support for both reset and interrupt modes.

### 1. Key Features

- **Timer mode**: Single-shot or periodic
- **Clock sources**: Low-power oscillator (LPO) or system clock
- **Reset and interrupt outputs**:
  - Timeout can reset the MCU
  - Interrupt mode available for software recovery
- **Counter range**: 8–32 bits, configurable timeout period
- **Safety features**:
  - Protection against unintended writes (unlock sequences required)
  - Multi-level reset modes (WDT reset, NMI reset)
- **Independent operation**: Runs even in low-power or sleep mode

### 2. Module Architecture

The RA8 WDT module contains:

1. **Counter Unit** – decrements with clock input, detects overflow
2. **Control Registers** – configure timeout, mode, and feeding sequence
3. **Status Registers** – indicate WDT state, timeout flags, interrupt/reset flags
4. **Reset & Interrupt Unit** – triggers MCU reset or interrupt/NMI upon timeout

### 3. Working Flow

1. **Start WDT**: Configure clock source, counter, and timeout period, then enable.
2. **Feed operation**: Software writes the specific sequence to reset the counter.
3. **Timeout event**: If feeding is missed → counter overflows:
   - Triggers MCU reset (reset mode)
   - Triggers interrupt (interrupt mode)
4. **Safety protection**: Registers require unlock before modification.

## RT-Thread WDT Framework Instruction

The **RT-Thread WDT framework** provides a unified abstraction for hardware watchdogs across different MCUs. Applications can use the same API regardless of underlying hardware.

### 1. Device Model

In RT-Thread, the WDT is managed as a **device object** (`struct rt_device`, type `RT_Device_Class_WDT`).
 Developers interact via high-level device APIs, without touching registers directly.

### 2. Operation Interfaces

Applications interact with WDT via the following APIs:

| Function              | Description                    |
| --------------------- | ------------------------------ |
|  rt_device_find()     | Find a watchdog device by name |
|  rt_device_init()     | Initialize the watchdog device |
|  rt_device_control()  | Control watchdog settings      |
|  rt_device_close()    | Close the watchdog device      |

### 3. Framework Features

- **Unified abstraction** for all WDT hardware
- **Cross-platform support** with portable API
- **Timeout protection** via configurable expiration time
- **Modes**: Reset mode and interrupt mode
- **Reliable safety** through hardware-enforced resets

### 4. Usage Workflow

* **Find WDT device**

```c
#define WDT_DEVICE_NAME    "wdt"    /* Watchdog device name */

static rt_device_t wdg_dev;         /* Watchdog device handle */
/* Find the watchdog device by name and get its handle */
wdg_dev = rt_device_find(WDT_DEVICE_NAME);
```

* **Initialize the watchdog**

```c
/* Initialize the device */
rt_device_init(wdg_dev);
```

* **Control the watchdog**

The command control codes are defined as follows:

```c
#define RT_DEVICE_CTRL_WDT_GET_TIMEOUT    (1) /* Get timeout value */
#define RT_DEVICE_CTRL_WDT_SET_TIMEOUT    (2) /* Set timeout value */
#define RT_DEVICE_CTRL_WDT_GET_TIMELEFT   (3) /* Get remaining time */
#define RT_DEVICE_CTRL_WDT_KEEPALIVE      (4) /* Feed the watchdog */
#define RT_DEVICE_CTRL_WDT_START          (5) /* Start the watchdog */
#define RT_DEVICE_CTRL_WDT_STOP           (6) /* Stop the watchdog */
```

Example of setting the watchdog timeout value:

```c
#define WDT_DEVICE_NAME    "wdt"    /* Watchdog device name */

rt_uint32_t timeout = 1;       /* Timeout value in seconds */
static rt_device_t wdg_dev;    /* Watchdog device handle */
/* Find the watchdog device by name and get its handle */
wdg_dev = rt_device_find(WDT_DEVICE_NAME);
/* Initialize the device */
rt_device_init(wdg_dev);

/* Set the watchdog timeout value */
rt_device_control(wdg_dev, RT_DEVICE_CTRL_WDT_SET_TIMEOUT, &timeout);
/* Set the idle thread callback function */
rt_thread_idle_sethook(idle_hook);
```

Example of feeding the watchdog in the idle thread hook function:

```c
static void idle_hook(void)
{
    /* Feed the watchdog in the idle thread callback function */
    rt_device_control(wdg_dev, RT_DEVICE_CTRL_WDT_KEEPALIVE, NULL);
}
```

**Close the watchdog**

```c
rt_device_close(wdg_dev);
```

## Hardware Description

None

## FSP Configuration Instructions

* Open the FSP tool and create a new stack, selecting `r_wdt`.

![image-20250731092603474](figures/image-20250731092603474.png)

* **Note:** Using WDT requires the `OFS0` register configuration to be enabled.

![image-20250731092848714](figures/image-20250731092848714.png)

## RT-Thread Settings Configuration

* Enable WDT.

![image-20250814172751032](figures/image-20250814172751032.png)

## Example project description

The sample program is located at `project/Titan_driver_wdt/src/hal_entry.c`.

```c
/*
 * Copyright (c) 2006-2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author        Notes
 * 2024-03-11     kurisaw       first version
 */

#include <rtthread.h>
#include "hal_data.h"
#include <rtdevice.h>
#include <board.h>

#define DBG_TAG "wdt"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define WDT_DEVICE_NAME "wdt"    // Default watchdog device name, depending on the BSP
#define WDT_FEED_INTERVAL 1000   // Feeding interval (unit: ms)
#define WDT_TIMEOUT 3            // Watchdog timeout period (unit: s)

static rt_device_t wdt_dev = RT_NULL;
static rt_thread_t feed_thread = RT_NULL;

#define LED_PIN_0    BSP_IO_PORT_06_PIN_00 /* Onboard LED pins */

void hal_entry(void)
{
    rt_kprintf("\nHello RT-Thread!\n");
    rt_kprintf("==================================================\n");
    rt_kprintf("This example project is an driver wdt routine!\n");
    rt_kprintf("==================================================\n");

    LOG_I("Tips:");
    LOG_I("You can run wdt testcase by executing the instruction: \'wdt_sample\'");

    while (1)
    {
        rt_pin_write(LED_PIN_0, PIN_HIGH);
        rt_thread_mdelay(1000);
        rt_pin_write(LED_PIN_0, PIN_LOW);
        rt_thread_mdelay(1000);
    }
}

static void feed_dog_entry(void *parameter)
{
    int count = 0;

    while (1)
    {
        if (count < 10)
        {
            rt_device_control(wdt_dev, RT_DEVICE_CTRL_WDT_KEEPALIVE, RT_NULL);
            LOG_I("[FeedDog] Feeding watchdog... %d", count);
        }
        else
        {
            LOG_E("[FeedDog] Simulate exception! Stop feeding.");
        }

        count++;
        rt_thread_mdelay(WDT_FEED_INTERVAL);
    }
}

static int wdt_sample(void)
{
    rt_err_t ret;

    wdt_dev = rt_device_find(WDT_DEVICE_NAME);
    if (wdt_dev == RT_NULL)
    {
        LOG_E("Cannot find %s device!", WDT_DEVICE_NAME);
        return -1;
    }

    ret = rt_device_control(wdt_dev, RT_DEVICE_CTRL_WDT_START, RT_NULL);
    if (ret != RT_EOK)
    {
        LOG_E("Start watchdog failed!");
        return -1;
    }

    LOG_I("Watchdog started...", WDT_TIMEOUT);

    feed_thread = rt_thread_create("feed_dog", feed_dog_entry, RT_NULL, 1024, 10, 10);
    if (feed_thread != RT_NULL)
        rt_thread_startup(feed_thread);

    return 0;
}
MSH_CMD_EXPORT(wdt_sample, wdt_sample);
```

## Compilation & Download

* **RT-Thread Studio**: Download the Titan Board resource package in the RT-Thread Studio package manager, then create a new project and compile it.

After compilation, connect the USB-DBG interface of the development board to the PC, and download the firmware to the development board.

## Run Effects

Enter the `wdt_sample` command at the terminal to run the WDT test program. After feeding the dog 10 times, stop feeding the dog to simulate an abnormal program situation.

![PixPin_2025-07-28_10-11-00](figures/PixPin_2025-07-28_10-11-00.png)

## References

Device and Driver: [WDT Device](https://www.rt-thread.org/document/site/#/rt-thread-version/rt-thread-standard/programming-manual/device/watchdog/watchdog)
