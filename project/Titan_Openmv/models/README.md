# 模型部署

`sdcard/` 保存 Ethos-U55-256 编译模型及标签，按示例需要部署到 SD 卡。模型来源、参数和哈希见 [MODEL_MANIFEST.json](MODEL_MANIFEST.json)，许可对应关系见 [LICENSE_INDEX.json](LICENSE_INDEX.json)，许可声明保存在 [licenses](licenses/) 中。

## 模型分类

| 任务 | 模型 | 输出 |
|---|---|---|
| 人脸中心检测 | FOMO | 网格类别分数、中心点 |
| 人脸检测与关键点 | BlazeFace、Face Landmarks | 人脸框、468 关键点 |
| 手部关键点 | Palm、Hand Landmarks | 手掌框、21 关键点 |
| 人体姿态 | MoveNet | 17 关键点 |
| 人体检测 | YoloLC、YOLOv8n 192 | person 检测框 |
| 通用检测 | [YOLOv8n COCO80](licenses/COCO80_NOTICES.md) | 80 类检测框 |
| 姿态与分割 | YOLOv8 Pose / U-Net | 多人关键点 / 像素类别 |
| 专用检测 | 192×192 模型组 | 手势、猫狗、人体、人脸、VOC20 |
| 分类 | VWW、MobileNetV1、ResNet8 | 类别分数 |

脚本、预处理及后处理见 [机器学习示例](../examples/02_机器学习/README.md)。类别集合由模型权重决定，标签文件只用于显示名称。

模型清单记录输入输出张量、编译参数和文件校验值。各模型的检测效果、运行内存和帧率需结合实际应用确认。原始编译配置不随工程提供，记录的参数不保证重新编译后逐字节一致。

## 部署流程

前提：板端已加载配套 `openmv-vision.bin`。按 [机器学习示例](../examples/02_机器学习/README.md) 的模型表，从 `sdcard/` 选择模型及同名标签，保持原文件名复制到板端存储根目录。Hand 示例也直接使用带 `_u55_256` 后缀的模型文件名。

安全弹出主机磁盘后运行对应脚本。切换模型时停止脚本并软复位。

`/rom` 为 SD 根目录的只读别名。视觉文件、模型和用户文件共用 SD 空间。

## 运行参数

| 字段 / 接口 | 含义 |
|---|---|
| `inputs`、`outputs` | 张量形状、dtype、scale、zero point |
| `npu_calls_per_predict` | 一次预测中的 Ethos-U 子图调用次数 |
| `compiler_scratch_bytes` | 编译器声明的 scratch 区需求 |
| `Sram_Only` | Vela 编译时的 memory mode |
| `print(model)` | 运行时模型与 arena 地址、容量 |

TFLM 分配的 arena 还包含 allocator、持久状态及运行时数据。实际 SRAM / HyperRAM 驻留由运行时分配结果决定，可通过 `print(model)` 查看。
