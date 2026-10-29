# Titan Board 外设示例

- [LED](01_GPIO与LED/blinky.py)：红/绿/蓝 LED 依次点亮，无需外接线。
- [用户键](01_GPIO与LED/gpio_button.py)：读取本板 P110，按下为 0。
- [I2C 扫描](03_I2C/i2c_scanner.py)：相机脚本停止后扫描 J9 的 I2C0 总线。

本板引脚见 [BOARD.md](../BOARD.md)。SoftI2C/SoftSPI 可用，但未为未知外接设备
预选引脚。硬件 UART/PWM/SPI/ADC/LCD 接口未开放。
