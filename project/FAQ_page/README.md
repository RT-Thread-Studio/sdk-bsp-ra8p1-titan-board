# Frequently Asked Questions (FAQ)

This document collects common issues encountered by users during usage and provides corresponding solutions.

## 1. Firmware Programming

### Q1. DAP-Link not detected during firmware flashing

![image-20251103101117122](figures/image-20251103101117122.png)

**Solution:**

Uninstall the **WinUSB** device in *Device Manager*, then reconnect the USB cable.

![image-20251103101304605](figures/image-20251103101304605.png)

After doing so, both the serial port and **CMSIS-DAP** devices should appear in *Device Manager*.

![image-20251103112011806](figures/image-20251103112011806.png)

------

### Q2. First compilation error after creating an example project

![image-20251103101635806](figures/image-20251103101635806.png)

**Solution:**
 Right-click the project and select **Synchronize scons configuration to project**.

![image-20251103101758894](figures/image-20251103101758894.png)

------

### Q3. RFP stuck while creating a new project

![image-20251103103104173](figures/image-20251103103104173.png)

**Solution:**

1. Select **Tool**

![image-20251103103152739](figures/image-20251103103152739.png)

2. Put the development board into **BOOT mode** (hold the **BOOT** button while pressing **RST**). When the following window appears, click **OK** to connect.(Do not release the BOOT button until the project is created successfully.)

![image-20251103103227657](figures/image-20251103103227657.png)

------

### Q4. Unable to flash HEX file using RFP

![image-20251103102857223](figures/image-20251103102857223.png)

1. **Solution:**
   1. Connect via the USB port labeled **USB-DEV**.
   2. Make sure the development board is in **BOOT mode**.

------

------

### Q5. Chip cannot be programmed

**Solution:**
 If the chip cannot be programmed, you can reinitialize it as follows:

Enter **BOOT mode**, then click **Initialize Chip** as shown below:

![image-20251103103547340](figures/image-20251103103547340.png)

------

### Q6. Error after clicking download:

```
Error(E0000305): The program file exceeds the flash memory size of target device. Operation failed.
```

**Solution:**

Refer to the image below:

![image-20251103104256487](figures/image-20251103104256487.png)

------

### Q7. DAP-Link malfunction after flashing dual-core program

![image-20251103104744996](figures/image-20251103104744996.png)

1. **Solution:**
   1. **Method 1:** Use **RFP** to initialize the device and flash a **single-core project**.
   2. **Method 2:** A community developer shared an article about **updating the debugger firmware**, which supports flashing **dual-core projects**:
       https://club.rt-thread.org/ask/article/3387ad4472d12ead.html

------

### Q8. Error “rtthread.bin not found (file not found)” when flashing

![image-20251103105126768](figures/image-20251103105126768.png)

**Solution:**

Configure the project’s **debug and download parameters** as shown below:

![image-20251103105322750](figures/image-20251103105322750.png)

![image-20251103105437733](figures/image-20251103105437733.png)

------

## 2. Camera and Display Connection

### Q9. Camera Connection

**Solution:**

- **MIPI CSI:**

  Use a **22-pin reverse FFC cable** to connect the development board’s **MIPI DSI/CSI** connector to the camera adapter board’s **MIPI** connector.

![image-20251103133548729](figures/image-20251103133548729.png)

* CEU：

​	Use a **22-pin reverse FFC cable** to connect the development board’s **CEU_CAM** connector to the camera adapter board’s **DVP** connector.

![image-20251103133556818](figures/image-20251103133556818.png)

------

### Q10. RGB LCD Connection

**Solution:**

Display model: **ALIENTEK 4.3" RGB LCD 800×480**

Use a **40-pin same-direction FFC cable** to connect the development board’s **LCD_RGB** connector to the display.

![image-20251103115738001](figures/image-20251103115738001.png)

------

### Q11. MIPI LCD Connection

**Solution:**

Display model: **Guanxian TL043WVV02CT**

Use a **22-pin reverse FFC cable** to connect the development board’s **MIPI DSI/CSI** connector to the display adapter board’s **DIS-MIPI** connector.
 Then connect the **MIPI display** to the adapter board’s **TITAN-MIPI** connector.

>Use a Dupont wire to connect a GPIO pin to the **BL** pin on the adapter board to provide backlight power.

![image-20251103115211524](figures/image-20251103115211524.png)

------

## 3. Runtime Issues

### Q12. System freezes after running for a while

**Solution:**

Possible causes include:

- **Insufficient task stack space** → Increase stack size when creating threads.
- **Memory leaks** → Check heap usage.
- **Deadlock or priority inversion** → Review mutex and semaphore usage.

------

##Feedback & Support

If you encounter other issues, you can get help through the following channels:

- 📘 View project documentation: [User Manual](https://rt-thread-studio.github.io/sdk-bsp-ra8p1-titan-board/)
- 🐞 Submit an issue: [GitHub Issues](https://github.com/RT-Thread-Studio/sdk-bsp-ra8p1-titan-board/issues)
- 💬 Contact technical support: [RT-Thread Community Forum](https://club.rt-thread.org/)