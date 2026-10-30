# 引脚与外设

## 固定资源

| 功能 | 配置 |
|---|---|
| HyperRAM | OSPI1 CS0；P602–P607、PC00–PC04、PC08，RESET#=PC07 |
| 控制台 | SCI8：RX=P500，TX=P501，115200/8N1 |
| 相机时钟 | PA11，GPT6A，25 MHz |
| 相机控制 | J9；I2C0 SDA=P409、SCL=P410；RESETB=P411 |
| I2C1 | SDA=P511、SCL=P512，CEU 接口 |
| SDHI0 | P111、PD01–PD05，检测 PD07；4-bit 总线 |
| USB | J3；P413 选择 HS/FS，默认 HS；J4 为调试接口 |
| LED(1/2/3) | 红 P013、绿 P613、蓝 P012，低电平点亮 |
| 用户键 | P110，内部上拉，按下为 0 |

J9 的 GPIO0 用作模组 RESETB，不配置 PWDN。连接模组时应核对该引脚功能。

## Python 接口

支持 `board.LED`、`machine.Pin`/IRQ、`I2C(0/1)`、`SoftI2C`、`SoftSPI` 和
`Timer(-1)`。硬件 I2C 固定 400 kHz、100 ms 超时；I2C0 与相机共享，
相机工作时不要直接改写传感器寄存器。SoftI2C/SoftSPI 使用前需选择空闲引脚。
GPIO 使用 MCU 名称，如 `P110`。

HyperRAM、SD、USB、调试、UART8、I2C 及相机时钟/复位脚由系统保留；Python
尝试重新配置会返回忙错误。IRQ 和软件定时器回调在 VM 调度器执行。

硬件 `machine.UART/PWM/SPI/ADC`、`display/lcd` 未开放，也未提供网络、音频、
RTC/WDT、SPI/I2C 从机接口。

## RGB 状态灯

| 状态 | 指示 |
|---|---|
| 初始化中 | 蓝灯 |
| VM、相机初始化和视觉文件加载就绪 | 绿灯 |
| 初始化或视觉文件加载失败 | 红灯 |

脚本首次调用 `board.LED` 后接管三色灯，下一次软复位恢复状态指示。示例：

```python
import board

board.LED(1).on()   # 红灯
board.LED(1).off()
```

## 相机

相机示例统一使用 QVGA（320×240），可选择 RGB565 或灰度，并按算法需要裁剪窗口。
VGA 仍存在采集报错，当前使用 QVGA。曝光与白平衡按现场调整。
当前 `hmirror`/`vflip` 回调不改变 MIPI 输出方向。

需要诊断采集超时时，在重新启动相机前读取：

```python
import board
print(board.camera_status())
```

| 字段 | 含义 |
|---|---|
| `frames` | 本次采集会话移交给 OpenMV 的 DMA 帧数 |
| `faults` | 锁存故障位 |
| `vin_errors` / `csi_errors` | VIN / CSI 故障中断计数 |
| `corrected_ecc` / `lane_events` | 已纠正 ECC / 数据通道低功耗事件计数 |
| `stale_frames` / `starved_frames` | 过期中断 / 缺少空闲缓冲区导致的丢帧数 |
| `vin_irq` / `vin_status` | 最近 VIN 中断状态 |
| `lane0_status` / `lane1_status` / `vc_status` | 最近 CSI 通道状态 |
| `opened` / `streaming` | 驱动打开及采集状态 |

计数在新采集会话打开时归零，关闭后仍保留。排查时同时记录 UART8 的
`[titan.csi]` 日志；持续取帧脚本见
[camera_stress_test.py](../examples/06_性能测试/camera_stress_test.py)。
