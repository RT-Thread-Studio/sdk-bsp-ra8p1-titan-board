# 本板运行约定

示例面向 Titan Board、OV5640 MIPI 和本工程固件。
LED(1/2/3) 对应红 P013、绿 P613、蓝 P012；用户键 P110，低电平有效。
相机使用 J9，应用 USB 使用 J3，UART8 为 RT-Thread 控制台。

I2C0：SDA=P409、SCL=P410，与相机共用；I2C1：SDA=P511、SCL=P512，接 CEU
接口。两总线固定 400 kHz。

支持 GPIO/IRQ、I2C、SoftI2C、SoftSPI 和 `Timer(-1)`。硬件 UART/PWM/SPI/ADC、
LCD 未开放。请勿重配 HyperRAM、SD、USB、控制台和相机占用的脚；具体范围见
[引脚说明](../docs/peripherals.md)。

存储固定为 SD FAT/FAT32。`/rom` 是 SD 根目录的只读别名。电脑传输完成并安全
弹出 MSC 后再执行板端写文件的示例。HEX 与 `openmv-vision.bin` 必须同次构建。
