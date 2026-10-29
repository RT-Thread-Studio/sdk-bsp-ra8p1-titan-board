# 构建与固件部署

## 工具要求

| 工具 | 用途 |
|---|---|
| Arm GNU Toolchain 13.3.Rel1 | Cortex-M85 C/C++ 编译和链接 |
| Python 3.10+、SCons | 工程生成、MicroPython 元数据生成与固件打包 |
| 主机 GCC、GNU make、sh/cat/sed | 构建运行在电脑上的 `mpy-cross` |
| J-Link v8.48 | 下载和调试 |

`RTT_EXEC_PATH` 指向 ARM 工具链的 `bin`。可用
`python -m pip install scons` 在所选 Python 中安装 SCons。
Studio 调用的 Python 不满足要求时，设置 `TITAN_OPENMV_PYTHON` 为 Python 3
解释器的完整路径。

Windows 主机工具可使用 MSYS2 的 UCRT64/MinGW-w64 GCC 和 MSYS shell，或
提供同时包含 `gcc.exe`、`make.exe`、`sh.exe`、`cat.exe`、`sed.exe` 的工具目录。
脚本会查找 PATH、Windows 注册的 PATH 以及系统盘的 `msys64`/`msys32`。
非默认安装可设置 `MSYS2_ROOT`；完整工具目录可通过 `OPENMV_HOST_TOOLS` 指定。
`MPY_HOST_CC` 可显式指定主机 GCC，不能指向 `arm-none-eabi-gcc`。

## RT-Thread Studio

从本 BSP 示例列表创建 `Titan_Openmv` 工程，或导入源码目录中的 Eclipse 工程。
工程自带 FSP 6.4，使用当前 SDK 的 `rt-thread/` 和 `libraries/`；BSP 创建流程
会将这两个共享目录复制到导出工程。

首次导入源码或改变工具链路径后，在工程目录生成本机工程配置：

```text
python -m SCons --target=eclipse
```

此命令生成 `build/make/settings.json`、`build/make/tools.mk` 等本机文件，
并更新 Studio 工程。刷新工程后执行 Build，构建目标为 `openmv-all`。
Studio 生成 `Debug/makefile` 后，也可在其构建终端执行：

```text
cd Debug
make -j16 openmv-all
make -j16 openmv-download
```

`openmv-all` 顺序完成元数据准备、编译链接和固件打包。
`openmv-download` 只下载已有的 `Debug/rtthread.hex`，不会自动重新编译。
工程提供的下载入口用于固件编程，Studio 普通下载按钮使用的是另一套脚本。

## SCons 命令行

Windows CMD 环境变量示例，路径按实际安装修改：

```bat
set "RTT_EXEC_PATH=C:\toolchains\arm-gnu-13.3\bin"
set "MSYS2_ROOT=C:\msys64"
cd /d C:\work\sdk-bsp-ra8p1-titan-board\project\Titan_Openmv
python -m SCons -j8
```

Linux 使用相同的 SCons 命令，将 ARM GCC 13.3 和主机工具加入 PATH。
只生成 QSTR、注册表和冻结模块时可使用 `python -m SCons --prepare-only`。
这会按需构建主机 `mpy-cross`，但不编译或链接 MCU 固件。

Windows 下工具链路径过长可能导致 C++ 多库头文件无法打开。脚本优先使用
Windows 8.3 短路径；不支持短路径的磁盘应使用较短的工具链安装目录。

## 下载与 SD 文件

完整构建输出 `firmware/rtthread.hex` 和 `firmware/openmv-vision.bin`。
从工程目录运行：

```text
python tools/download.py --jlink "C:/tools/JLink_v848/JLink.exe"
```

也可设置 `JLINK_EXE`，或让脚本从 Studio 工具目录查找 v8.48。
目标为 `R7KA8P1KF_CPU0`，接口 SWD，默认 1000 kHz；多探针使用
`--serial` 指定序列号。`--hex` 可指定要下载的 HEX。
脚本下载并校验后通过复位引脚启动，日志位于 `build/jlink-download.log`。

将同次构建的 `openmv-vision.bin` 复制到 FAT/FAT32 SD 卡根目录，安全弹出磁盘
后复位。脚本不复制 SD 文件，也不格式化 SD 卡。没有 SD 或视觉文件不匹配时，
相机和图像模块不可用。

## 工程维护

`.config`、`rtconfig.h` 和 `rtconfig_project.h` 控制功能；通过
`python -m SCons --pyconfig` 调整 RT-Thread 配置。FSP 配置由本工程的
`ra_cfg/` 和 `ra_gen/` 源码维护。

`build/`、`Debug/`、`Release/` 和 `firmware/` 为生成目录，不提交到版本库。
修改相机驱动时可运行以下主机回归检查；它们检查事件和缓冲区交接，不替代
实板采集验证：

```text
python tools/tests/test_camera_events.py
python tools/tests/test_capture_handoff.py
```
