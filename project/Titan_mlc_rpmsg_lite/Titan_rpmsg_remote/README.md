# RPMsg-Lite Usage Instructions（Core0 Master）

**English** | [**Chinese**](./README_zh.md)

## Introduction

This project runs on **Core1 of RA8P1 (Cortex-M33, 250 MHz)** as the remote core in a dual-core system:

- Receives commands and data from Core0 (Master) via **RPMsg-Lite**;
- Handles specific tasks (such as communication processing, data acquisition, safety control, and low-power management) and sends results back to Core0;
- Supports independent application logic and a message-driven mechanism.

**Key Features**

- **Multicore Communication**: RPMsg-Lite Remote initialization, endpoint creation, and message transmission/reception.
- **RT-Thread Support**: Independent threads handle message sending and receiving, supporting event/queue synchronization.
- **Shared Memory & Cache Safety**: Uses shared memory reserved by Core0, ensuring cache coherency.
- **Handshake Mechanism**: HELLO/READY/ACK communication with the Core0 Master.
- **Extensible Protocol Layer**: Lightweight protocol examples (Ping/Pong, command frames, data streams).

## RPMsg-Lite Instructions

**RPMsg-Lite** is a lightweight implementation of Remote Processor Messaging (RPMsg), primarily used in multi-core processor systems to enable data exchange and message passing between different cores. It is based on the **OpenAMP** framework and leverages shared memory and virtual channels (Virtio Channels) to achieve efficient and reliable inter-core communication in resource-constrained embedded systems.

#### 1. Design Background

In multi-core MCUs or SoC systems, different cores may execute different types of tasks, for example:

- Cortex-M cores run real-time control tasks
- Cortex-A cores run operating systems or high-level applications

In such cases, an efficient communication mechanism is needed to exchange data or trigger events between cores. RPMsg-Lite provides a lightweight, easy-to-use, and reliable message-passing solution.

#### 2. Key Features

1. **Lightweight**
   - Compared with full RPMsg implementations, RPMsg-Lite consumes less memory and CPU resources, making it ideal for resource-constrained embedded systems.
   - Core data structures and queue mechanisms are optimized to minimize memory copies.
2. **Inter-core Communication**
   - Supports communication between different cores.
   - Uses shared memory as a data buffer, with messages transmitted via virtual channels.
3. **Reliability**
   - Provides reliable message delivery mechanisms to ensure messages are neither lost nor duplicated.
   - Configurable message queue lengths allow for high-reliability data transmission.
4. **Flexibility**
   - Supports static or dynamic creation of communication channels to adapt to different system architectures.
5. **Simple Interface**
   - Offers easy-to-use APIs for sending, receiving, and managing channels.
   - Developers can integrate it quickly without needing to understand the underlying shared memory or interrupt mechanisms.

#### 3. Working Principle

RPMsg-Lite is based on the **virtio** virtual device mechanism:

1. Shared memory stores message data and queue information.
2. The sending core writes messages to the shared memory queue and notifies the receiving core via an interrupt.
3. The receiving core reads the messages and processes them according to application requirements.
4. After processing, the receiving core can send acknowledgment messages to ensure communication reliability.

#### 4. Application Scenarios

- **Dual-core or Multi-core MCU systems**: For example, in the RA8P1 dual-core MCU, M85 handles high-performance tasks while M33 handles real-time tasks, with communication between the cores via RPMsg-Lite.
- **Real-time task separation**: Time-sensitive tasks and high-level application tasks can run separately to improve system performance.
- **Resource-constrained embedded systems**: Low memory footprint and communication overhead make it suitable for industrial control, robotics, automotive electronics, and other fields.

## Hardware Description

None

## FSP Configuration

* Create two  `r_ipc` stacks:

![image-20250829153214184](figures/image-20250829153214184-17564539243051.png)

* Configure IPC0：

![image-20250829155944641](figures/image-20250829155944641.png)

* Configure IPC1：

![image-20250829160001065](figures/image-20250829160001065.png)

## RT-Thread Settings Configuration

* Enable sub-nuclear startup and rpmsg-lite。

![image-20250829153455687](figures/image-20250829153455687-17564539243064.png)

## Example Code Description

The sample program is located at `/board/ports/rpmsg-adapt`.

## Compilation & Download

* **RT-Thread Studio**: In RT-Thread Studio's package manager, download the Titan Board resource package, create a new project, and compile it.

#### Download Methods

Currently, DAP-Link does not support programming the Core1 firmware, so you need to **use the Renesas Flash Programmer tool to program the firmware**. The detailed programming steps are as follows:

1. Download and install the Renesas Flash Programmer tool: [Renesas Flash Programmer (Programming GUI) | Renesas](https://www.renesas.com/en/software-tool/renesas-flash-programmer-programming-gui#downloads).
2. Connect the USB-DEV port of the development board to the PC.
3. Create a project: The development board needs to enter BOOT mode (**hold down the BOOT button and press the RESET button once**).

![image-20250829142936047](figures/image-20250829142936047.png)

4. Select the firmware to be programmed:

![image-20250902162451353](figures/image-20250902162451353.png)

5. Flashing firmware (at this time, the development board must be in **BOOT** mode):

![image-20250829143543000](figures/image-20250829143543000.png)

## Run Effect

Core1 will automatically start **rp_remote**, and once Core0 starts **rp_master**, inter-core communication can take place.

![image-20250829160852624](figures/image-20250829160852624.png)