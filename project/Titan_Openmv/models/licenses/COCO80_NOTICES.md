# COCO80 常见物品目标检测

源码工程 `models/sdcard/yolov8n_coco80_192_u55_256.tflite` 是输出目标位置和类别的 YOLOv8n 模型。COCO 80 类包括 person、bottle、cup、cell phone；它与 ImageNet 1000 类分类模型、原工程单类 person YOLO 模型不同。

脚本位于 `examples/02_机器学习/00_神经网络/yolov8_coco80_detector.py`。把 `.tflite` 和同名 `.txt` 放到所选存储介质根目录，通过 `/rom/...` 读取。需要先加载匹配当前固件的 `openmv-vision.bin`。分类与检测使用不同模型，不能只替换标签文件。

## 来源与编译

- 来源：[Himax YOLOv8 on WE2](https://github.com/HimaxWiseEyePlus/YOLOv8_on_WE2)，固定 revision `2483a5ebe99576947ecf7e4564aff9cec93fa373`，原始模型 `vela/img_yolov8_192/yolov8n_full_integer_quant_size_192.tflite`。
- 原模型 SHA-256：`2dda256475a9a65d3a9272ef7f8e7a33d43e7ff6b8ea110c5b34810e4cf39e5b`。
- 使用 Vela 5.1.0，U55-256、COP1、`Sram_Only`、`Ethos_U55_High_End_Embedded`、Performance。重新编译源模型，没有使用为 WE2/U55-64 编译的产物。

## 许可

保留 [Himax MIT 声明](LICENSE.Himax-MIT.txt) 和 [Ultralytics AGPL-3.0 声明](LICENSE.Ultralytics-AGPL-3.0.txt)。项目发布和商业使用应遵循上游模型许可条件。
