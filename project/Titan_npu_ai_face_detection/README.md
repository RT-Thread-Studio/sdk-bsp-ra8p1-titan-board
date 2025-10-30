# NPU Accelerated Face Detection Instructions

**English** | [**Chinese**](./README_zh.md)

## Instructions

This example shows how to accelerate the operation of the YOLO-Fastest face detection model on the **Titan Board** using the **Arm® Ethos™ -u55 NPU**, And combine the CEU (Camera Engine Unit) camera interface and the RGB LCD display screen to achieve real-time face detection and display.

The main functions include:

- Collect real-time video streams via CEU (OV5640 camera)
- Perform YOLO-Fastest model inference on video frames using NPU
Display the detection results (with face box) on the LCD screen
- Supports hardware-accelerated YUV to RGB conversion and graphic rendering

## Overall system architecture

The system data flow of this example is shown in the following figure:

```css
[OV5640 Camera]
        │
        ▼
[CEU camera acquisition module]
        │ (YUV422)
        ▼
[DMA transfer to Frame Buffer (HyperRAM)]
        │
        ├──► [NPU (eethos -U55) runs YOLO-Fastest inference]
        │         │
        │         ▼
        │     [Test result: Coordinates + confidence level]
        │
        └──► [GLCDC Display controller]
                  │
                  ▼
          [RGB LCD real-time display]
```

## Arm® Ethos™-U55 NPU Features

The RA8P1 MCU used by the Titan Board integrates the **Arm® Ethos™-U55 neural processing unit (NPU) **, which can work in coordination with the Cortex-M85 CPU to significantly enhance the inference performance of neural networks.

### 1. Hardware Features

1. **Computing Power and Acceleration**
   - Supports INT8 quantized models
   - Delivers performance up to several hundred GOPS (depending on configuration)
   - Supports common operators such as convolution, pooling, ReLU, and Softmax
2. **Collaboration with CPU**
   - Works with the Cortex-M85 through the CMSIS-NN and Ethos-U drivers
   - Supports asynchronous execution between NPU and CPU
   - Model pre-processing and post-processing are handled by the CPU
3. **Memory and Bandwidth**
   - Supports direct feature map access from on-chip SRAM or external HyperRAM
   - DMA accelerates model input/output data transfer
   - Multi-level caching mechanisms reduce latency
4. **Compatibility**
   - Fully compatible with TensorFlow Lite for Microcontrollers (TFLM)
   - Supports model formats converted by the Arm NN SDK (.tflite)

## YOLO-Fastest Model Introduction

**YOLO-Fastest** is a lightweight object detection network designed for real-time operation on embedded devices.

| Item                       | Specification                                 |
| -------------------------- | --------------------------------------------- |
| Model Type                 | YOLO-Fastest (Face Detection)                 |
| Model Framework            | TensorFlow Lite (INT8)                        |
| Input Size                 | 192 x 192                                     |
| Output                     | Face bounding box coordinates + confidence    |
| Inference Time (Ethos-U55) | Approx. 25 ms/frame                           |
| Application Scenarios      | Face detection / Real-time visual recognition |

## FSP Configuration

### HyperRAM Configure

* Create a `r_ospi_b` stack:

![image-20250814160234405](figures/image-20250814160234405.png)

* Configure `r_ospi_b` stack:

![image-20250924115414432](figures/image-20250924115414432.png)

* HyperRAM pins configure:

![image-20250814160618340](figures/image-20250814160618340.png)

* Set the drive strength of all HyperRAM-related pins to **High (H)**, and configure OM_1_SIO0~OM_1_SIO7 as **input with pull-up**.![image-20250924114605538](figures/image-20250924114605538.png)

### CEU Configuration

* Create a  `r_ceu` stack:

![image-20250815152832972](figures/image-20250815152832972.png)

* Configure CEU:

![image-20250815153551538](figures/image-20250815153551538.png)

* Configure CEU pins:

![image-20250815154046223](figures/image-20250815154046223.png)

### CEU Clock Configuration

* Create a  `r_gpt` stack:

![image-20250815163248005](figures/image-20250815163248005.png)

* Configure the PWM output of the camera clock

![image-20250815165108280](figures/image-20250815165108280.png)

### D/AVE 2D Configuration

* Create a  `r_drw` stack:

![image-20250815162521851](figures/image-20250815162521851.png)

### RGB LCD Configuration

* Create a  `r_glcdc` stack:

![image-20250815161839268](figures/image-20250815161839268.png)

* Configure interrupt callback and graphics Layer 1:

![image-20250815154751007](figures/image-20250815154751007.png)

* Configure output parameters, CLUT, TCON.

![image-20250815154835759](figures/image-20250815154835759.png)

* Configure GLCDC pins:

![image-20250815154852533](figures/image-20250815154852533.png)

![image-20250815154859548](figures/image-20250815154859548.png)

### LCD Backlight Configuration

* Create a  `r_gpt` stack:

![image-20250815163248005](figures/image-20250815163248005.png)

* Configure backlight PWM output:

![image-20250815165329527](figures/image-20250815165329527.png)

### NPU Configuration

Create Google TFLM Core Lib stack:

![image-20251029172921966](figures/image-20251029172921966.png)

## RT-Thread Settings Configuration

* Enable the CUE camera, using the i2c1 and ov5640 cameras; Enable RGB565 LCD and use pwm7 output backlight.

![image-20250815155821339](figures/image-20250815155821339.png)

## Build & Download

- **RT-Thread Studio**: In RT-Thread Studio’s package manager, download the Titan Board resource package, create a new project, and compile it.

After compilation, connect the development board’s USB-DBG interface to the PC and download the firmware to the development board.

## Run Effect

After resetting the Titan Board, prepare a face picture and place it in front of the camera. At this time, observe the LCD screen and you can see that the face is framed by a green rectangle.

![image-20251029173335454](figures/image-20251029173335454.png)