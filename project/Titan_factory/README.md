# Production Test Program User Guide

**English** | [**Chinese**](./README_zh.md)

## Test Preparation

1. Development board test firmware: **rtthread.hex**
2. Wi-Fi firmware: **43438A1.bin**, **43438A1.clm_blob**
3. SD card: Place Wi-Fi firmware in the root directory of the SD card
4. MIPI CSI interface OV5640 camera
5. 800 × 480 RGB LCD screen
6. 3–6 LED lights

## Hardware Connections

### PWM

Connect LEDs to the six PWM pins on the Raspberry Pi interface: **VL_PWM (P714), UL_PWM (P712), WL_PWM (P108), VH_PWM (P715), UH_PWM (P713), and WH_PWM (P109).**

![image-20250905141150256](figures/image-20250905141150256.png)

### CANFD

Connect CAN0 and CAN1 (CAN0_H to CAN1_H; CAN0_L to CAN1_L).

![image-20250905141309245](figures/image-20250905141309245.png)

### RS485

Connect either of the two RS485 ports on the development board to the RS485 port of another development board (A to A; B to B).

![image-20250905141405784](figures/image-20250905141405784.png)

### MIPI CSI Camera

Connect an OV5640 camera module to the MIPI CSI interface.

![image-20250905141434842](figures/image-20250905141434842.png)

### RGB LCD Screen

Connect an 800 × 480 RGB screen to the RGB LCD interface on the back of the development board.

![image-20250813141932673](figures/image-20250813141932673.png)

### Ethernet

Insert a network cable into the ETH1 port.

![image-20250905141513257](figures/image-20250905141513257.png)

## LED Indicators

The development board has three LED indicators: blue, green, and red.

- **Blue LED:** Flashes continuously at 500 ms intervals, indicating program running status. If the blue LED stops flashing, the program has crashed.
- **Green LED:** Off by default. Lights up when all functional tests pass.
- **Red LED:** Off by default. Lights up when a functional test fails.

![image-20250905141546512](figures/image-20250905141546512.png)

## Program Execution Flow

### 1. Mounting HyperFlash and SD Card File Systems

After power-on, the board automatically mounts the SD card and HyperFlash file systems.

![image-20250811133648869](figures/image-20250811133648869.png)

### 2. Automatic Wi-Fi Connection

After mounting the SD card, the board loads Wi-Fi firmware from it, initializes Wi-Fi, and automatically connects to the Wi-Fi network (SSID: **Titan Board**, Password: **12345678**).

![image-20250811154027832](figures/image-20250811154027832.png)

### 3. PWM Output

After power-on, the six PWM pins on the Raspberry Pi interface output PWM waves. LEDs connected to these pins should show brightness changes (breathing light effect).

![image-20250811094346315](figures/image-20250811094346315.png)

### 4. Buzzer Test

The buzzer plays the melody of "Two Tigers" as a functional test.

### 5. HyperRAM Test

The HyperRAM test program measures HyperRAM read/write speed.

![image-20250811151046625](figures/image-20250811151046625.png)

### 6. Wi-Fi Test

The Wi-Fi test program initializes the Wi-Fi module, loads firmware from the SD card, and uses the `ping` command to test connectivity.

![image-20250811133756446](figures/image-20250811133756446.png)

### 7. Ethernet Test

The Ethernet test program uses the `ping` command to verify network connectivity.

![image-20250811134019902](figures/image-20250811134019902.png)

### 8. RTC Test

The RTC test outputs the configured date and time in the terminal.

![image-20250811134041938](figures/image-20250811134041938.png)

### 9. ADC Test

The ADC test program sequentially tests channels 0, 1, 2, and 3 of ADC0.

![image-20250811134118655](figures/image-20250811134118655.png)

### 10. CANFD Test

In the CANFD test program, CANFD0 transmits data, and CANFD1 receives it.

![image-20250811134147266](figures/image-20250811134147266.png)

### 11. IMU Test

The IMU test program verifies correct data reading from the BMI088 module.

![image-20250811134535211](figures/image-20250811134535211.png)

### 12. Magnetometer Test

The magnetometer test program verifies correct data reading from the IST8310 module.

![image-20250811134600225](figures/image-20250811134600225.png)

### 13. Camera and Screen Test

At the end of the test program, the camera is activated. Wave your hand in front of the camera to trigger capture; the LCD screen will display the captured image.

![image-20250815172223282](figures/image-20250815172223282.png)

## Test Result Description

- **All tests passed:** At the end of testing, if all functions pass, the green LED lights up and the terminal outputs `ALL Test Run Successful!`.

  ![image-20250811135100512](figures/image-20250811135100512.png)
  
- **Test failures detected:** If some tests fail, the red LED lights up and the terminal outputs which functions failed.

![image-20250811140343939](figures/image-20250811140343939.png)

**Example terminal output of test program:**

![2025-08-13_093555_600](figures/2025-08-13_093555_600.png)

