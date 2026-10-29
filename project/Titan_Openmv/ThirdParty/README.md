# 固定第三方依赖

上游组件的版本、来源和逐文件 SHA-256 登记在 [components.lock.json](components.lock.json)。发布检查按该清单验证源码完整性。

| 目录 | 用途 | 固定版本 |
|---|---|---|
| `openmv` | 图像算法、Python 模块与示例 | v5.0.1 / `631681e5ac5a` |
| `micropython` | Python 解释器与 mpy-cross | `dd984559aa42` |
| `apriltag` | AprilTag 检测 | `636b9ba14ba8` |
| `ulab` | NumPy 风格数值运算 | `d16144332717` |
| `libtflm` | OpenMV TFLM 适配 | `5b8e786fc4af` |
| `tflm` | TensorFlow Lite Micro | `a57349f0bc3b` |
| `cmsis_nn` | Cortex-M 神经网络 CPU 算子 | `01dee38e6d6b` |
| `kissfft` | FFT | v130 / `918336beac80` |
| `tinyusb` | USB 协议栈 | `6d46dea28419` |
| `ethos-u-core-driver` | Ethos-U 驱动 | 从本 BSP FSP 包提取，按文件哈希固定 |

板级代码位于 `platform/`，包含 RT-Thread、MicroPython、OpenMV 和 USB 的接口实现。构建步骤见 [构建说明](../docs/build.md)。更新组件版本时需同步维护锁文件与接口实现，并检查编译和运行结果。

为保证 Git 提交与检出不改变快照字节，项目禁用换行转换；MicroPython 的 `.gitattributes` 增加了对应覆盖规则。该版本管理元数据改动及其上游原始哈希记录在锁文件的 `vendor_overrides` 中，不涉及 C 或 Python 源码。

各组件保留原有 LICENSE、COPYING、NOTICE 和源码版权头。快照中的其他板型、测试和文档同样属于哈希清单的一部分。
