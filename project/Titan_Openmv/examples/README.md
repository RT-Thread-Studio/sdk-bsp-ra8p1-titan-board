# Titan Board Python 示例

本目录提供 Titan Board 的视觉、机器学习及外设示例。首先运行 [基础检查](board_smoke_test.py)，再运行
[相机预览](01_视觉/00_入门/helloworld.py)。硬件约定见 [BOARD.md](BOARD.md)。

相机示例统一使用 QVGA（320×240）采集；光流、模板匹配和部分模型示例在此基础上
裁剪所需窗口。VGA 仍存在采集报错，当前示例不使用该模式。

| 分类 | 内容 |
|---|---|
| [视觉](01_视觉/README.md) | 相机、绘图、滤波、色块、条码、特征与 AprilTag |
| [机器学习](02_机器学习/README.md) | 分类、检测、姿态、关键点及模型部署 |
| [外设](03_外设/README.md) | 本板 LED、P110 用户键及 I2C0 扫描 |
| [系统与存储](04_系统与存储/README.md) | 系统信息、软件定时器、SD 文件操作 |
| [通信范围](05_通信/README.md) | USB CDC 与控制台接口 |
| [性能测试](06_性能测试/README.md) | 连续取帧与图像算子测试 |

模型及标签从 `../models/sdcard/` 按需复制到 SD 根目录；其他素材按各分类 README
部署。写文件示例可能覆盖同名示例文件，运行前查看脚本并结束电脑端磁盘访问。
独立脚本一次运行一个。算法阈值和相机内参需按实际场景调整。

许可见 [LICENSE](LICENSE)、[OpenMV MIT](LICENSE.openmv-MIT.txt) 和
[Apache-2.0](LICENSE.Apache-2.0.txt)，模型许可单独见 [models](../models/README.md)。
