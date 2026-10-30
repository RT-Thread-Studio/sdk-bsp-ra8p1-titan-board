# Titan Board OpenMV

[English](README.md) | **中文**

本工程在 RA8P1 Titan Board 上运行 RT-Thread、MicroPython 和 OpenMV 5.0.1，
提供 OV5640 MIPI 相机采集、OpenMV IDE 预览、USB CDC/MSC、SD 存储及
TFLM/Ethos-U55 推理支持。第三方源码位于 `ThirdParty/`，内核和公共驱动使用
本 SDK 的 `rt-thread/` 与 `libraries/`。

## 硬件配置

| 资源 | 配置 |
|---|---|
| 主核 | Cortex-M85；不启动 Cortex-M33 |
| 外部内存 | 32 MiB HyperRAM，起始地址 `0x70000000` |
| 相机 | J9，OV5640 双 lane MIPI |
| 应用 USB | J3，USB HS CDC + MSC |
| 调试接口 | J4，J-Link/SWD |
| 控制台 | SCI8，P500 RX / P501 TX，115200、8N1 |
| 存储 | SDHI0，FAT/FAT32 SD 卡 |
| LED | 红 P013、绿 P613、蓝 P012，低电平点亮 |
| 用户键 | P110，低电平按下 |

Python 提供 `board.LED`、GPIO/IRQ、I2C、SoftI2C、SoftSPI 和软件定时器。
硬件 `machine.UART/PWM/SPI/ADC` 及 LCD/display 接口未开放。
完整接线和接口范围见 [引脚与外设](docs/peripherals.md)。

## 构建与下载

需要 Arm GNU Toolchain **13.3.Rel1**、Python **3.10+**、SCons，以及用于生成
`mpy-cross` 的主机 GCC、GNU make 和 POSIX shell。工具安装、环境变量和
Studio 导入方式见 [构建说明](docs/build.md)。

在 RT-Thread Studio 中构建工程，使用 `make -j16 openmv-all`。完成后运行
**openmv-download** Make Target；也可在 Studio 构建终端的 `Debug/` 目录执行：

```text
make -j16 openmv-download
```

使用命令行时，在本工程目录执行：

```text
python -m SCons -j8
python tools/download.py --jlink "C:/tools/JLink_v848/JLink.exe"
```

将 J-Link 路径替换为实际安装位置。工程使用 **v8.48**，下载使用工程提供的
Make Target 或脚本。

两种构建方式均生成以下配套文件：

| 文件 | 部署位置 |
|---|---|
| `firmware/rtthread.hex` | 通过 J-Link 烧录到 MCU |
| `firmware/openmv-vision.bin` | SD 卡根目录 |

**更新时必须同时使用同次构建的 HEX 和 BIN。** 相机采集、图像处理和 IDE
实时预览均需要 SD 卡上的配套视觉文件；缺少该文件时，基本 USB/REPL 仍可使用。

## 运行示例

1. 断电连接 J9 的 OV5640 模组，插入 FAT/FAT32 SD 卡。
2. 烧录 HEX，将 `openmv-vision.bin` 复制到 SD 根目录。可通过 J3 的 USB
   存储盘或读卡器复制。
3. 安全弹出电脑端磁盘，复位开发板，在 OpenMV IDE 中通过 J3 连接。
4. 先运行 [基础检查](examples/board_smoke_test.py)，再运行
   [相机预览](examples/01_视觉/00_入门/helloworld.py)。

启动状态灯：蓝灯表示正在初始化，绿灯表示运行环境和视觉文件就绪，红灯表示
初始化或视觉文件加载失败。脚本调用 `board.LED` 后自行控制三色灯，下一次
软复位恢复启动状态指示。

需要开机运行应用时，将脚本保存为 SD 根目录 `main.py`；可选的 `boot.py`
和 `lib/` 也放在根目录。`/rom` 是 SD 根目录的只读别名，无需创建同名文件夹。
板端写文件前，先结束电脑端文件访问并安全弹出磁盘。

IDE 显示 **AE3** 是协议兼容标识。固件更新使用本工程的文件和下载工具，
不能安装 Alif AE3 固件。

## 使用资料

- [Python 示例目录](examples/README.md)
- [模型部署与许可证](models/README.md)
- [第三方依赖版本与许可证](ThirdParty/README.md)

`ra/`、`ra_cfg/`、`ra_gen/` 为工程内的 FSP 6.4 配置源码，修改时需同步维护。
工程不包含可直接重新生成这些文件的 RASC `configuration.xml`。
