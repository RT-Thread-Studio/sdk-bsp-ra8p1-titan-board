# 性能测试

| 脚本 | 测试对象 |
|---|---|
| [camera_stress_test.py](camera_stress_test.py) | QVGA RGB565 连续取帧 10 分钟；每 5 秒输出采集诊断 |
| [image_ops_benchmark.py](image_ops_benchmark.py) | 相机固定输入；9 项图像操作；分别测量 HyperRAM / SRAM 工作图 |
| [vision_benchmark.py](vision_benchmark.py) | 人工灰度色块；二值化、均值滤波、色块、Canny |

## 相机持续取帧

`camera_stress_test.py` 需要包含 `board.camera_status()` 的 Titan OpenMV 固件及其
配套视觉镜像。默认开启 IDE 预览和连续 DMA，运行 10 分钟；可修改 `DURATION_MS`
延长测试，或设置 `PREVIEW=False` 对照关闭预览时的表现。脚本不写 SD。

正常结束会打印 `PASS`。首次异常会打印已完成帧数、运行时间及诊断字典，然后
保留异常停止，不自动重试。请保存 IDE 输出及 UART8 的 `[titan.csi]` 日志。
诊断字段含义见 [相机故障定位](../../docs/peripherals.md#相机)。

## 图像算子对照

`image_ops_benchmark.py` 面向 RA8P1 / OpenMV 5。前提为相机正常、板端已加载匹配的 `openmv-vision.bin`。运行会停止当前脚本，占用相机完成一次图像采集。

测试在采集结束、VIN DMA 停止后执行，关闭图像预览。每项每轮预热 10 次、测量 100 次，重复 3 轮。复制、工作图重建、显式 GC 和打印位于计时区外。

| 模式 | 工作图 | 地址检查 |
|---|---|---|
| `framebuffer` | `image.Image(...,copy_to_fb=True)` | `0x70000000–0x71FFFFFF` HyperRAM |
| `gc` | `source.copy()` | `0x22000000–0x221FFFFF` SRAM |

源图保留原始像素，各次调用前恢复工作图。腐蚀与膨胀分别测量，`morph` 使用脚本内指定的锐化核，JPEG quality=90。测试输入和部分参数与 OpenMV 公开表不同，比较时记录这些条件。

在 OpenMV IDE 中连接开发板，打开并运行本目录的 `image_ops_benchmark.py`，保存串行终端输出。默认 `EXPORT_FIXTURES=True` 输出十六进制输入数据；只观察时间时可关闭此选项。每条 `@BENCH` JSON 包含参数、地址、原始采样和统计值；最后应出现 `type=complete`。

每次运行会重新采集图像，比较 JPEG 等依赖图像内容的操作时，应同时记录输入数据和哈希。

比较结果时记录固件版本、工作图地址、输入数据和测试参数。

## 人工图像测试

`vision_benchmark.py` 使用固定的 320×240 灰度色块图，不需要相机采集。使用兼容 OpenMV 5 API 的固件，修改 `BOARD_LABEL` 和 `FIRMWARE_LABEL` 记录平台与版本，保持其他参数不变。

该脚本未固定像素缓冲所在内存。跨板测试时应另行记录地址与内存区域，避免将 SRAM 工作图与 HyperRAM 采集帧直接比较。
