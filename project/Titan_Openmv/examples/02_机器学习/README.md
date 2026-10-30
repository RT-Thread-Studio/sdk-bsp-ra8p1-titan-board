# 机器学习示例

运行环境：OpenMV `ml`、TFLM、Ethos-U55。将 `firmware/openmv-vision.bin` 复制到板端根目录，模型和标签从源码工程 `models/sdcard/` 取得。

## 模型准备

各示例的模型文件见下表；路径相对于源码工程 `models/`。模型和标签复制到板端根目录，`/rom` 映射到该目录。

单手和多手关键点使用以下文件名：

| `models/sdcard/` 中的文件 | 示例加载路径 |
|---|---|
| `palm_detection_full_192_u55_256.tflite` | `/rom/palm_detection_full_192_u55_256.tflite` |
| `hand_landmarks_full_224_u55_256.tflite` | `/rom/hand_landmarks_full_224_u55_256.tflite` |

从源码工程 `models/sdcard/` 选择示例需要的模型和同名标签，保持原文件名复制到板端存储根目录。无需创建 `rom` 或 `sdcard` 子目录。安全弹出电脑端磁盘后运行脚本。

## 示例列表

| 脚本 | 作用 | 所需模型和标签（`models/` 相对路径） |
|---|---|---|
| [blazeface_detector.py](00_神经网络/blazeface_detector.py) | BlazeFace 人脸与六关键点 | `sdcard/blazeface_front_128_u55_256.tflite` |
| [blazepalm_detection.py](00_神经网络/blazepalm_detection.py) | BlazePalm 手掌检测 | `sdcard/palm_detection_full_192_u55_256.tflite` |
| [face_landmarks_single_face.py](00_神经网络/face_landmarks_single_face.py) | 单脸 468 关键点跟踪 | `sdcard/face_landmarks_192_u55_256.tflite`<br>`sdcard/blazeface_front_128_u55_256.tflite` |
| [face_landmarks_multi_face.py](00_神经网络/face_landmarks_multi_face.py) | 多脸 468 关键点 | `sdcard/face_landmarks_192_u55_256.tflite`<br>`sdcard/blazeface_front_128_u55_256.tflite` |
| [hand_landmarks_single_hand.py](00_神经网络/hand_landmarks_single_hand.py) | 单手 21 关键点跟踪 | `sdcard/palm_detection_full_192_u55_256.tflite`<br>`sdcard/hand_landmarks_full_224_u55_256.tflite` |
| [hand_landmarks_multi_hand.py](00_神经网络/hand_landmarks_multi_hand.py) | 多手 21 关键点 | `sdcard/palm_detection_full_192_u55_256.tflite`<br>`sdcard/hand_landmarks_full_224_u55_256.tflite` |
| [movenet_singlepose_detection.py](00_神经网络/movenet_singlepose_detection.py) | MoveNet 人体 17 关键点 | `sdcard/movenet_singlepose_192_u55_256.tflite` |
| [tf_object_detection.py](00_神经网络/tf_object_detection.py) | FOMO 人脸中心检测 | `sdcard/fomo_face_detection_u55_256.tflite`<br>`sdcard/fomo_face_detection_u55_256.txt` |
| [yolo_lc_person_detector.py](00_神经网络/yolo_lc_person_detector.py) | YOLO LC 行人检测 | `sdcard/yolo_lc_192_u55_256.tflite`<br>`sdcard/yolo_lc_192_u55_256.txt` |
| [yolo_v8_detector.py](00_神经网络/yolo_v8_detector.py) | YOLOv8 行人检测 | `sdcard/yolov8n_192_u55_256_cpu_softmax.tflite`<br>`sdcard/yolov8n_192_u55_256_cpu_softmax.txt` |
| [yolov8_coco80_detector.py](00_神经网络/yolov8_coco80_detector.py) | COCO80 常见物品检测 | `sdcard/yolov8n_coco80_192_u55_256.tflite`<br>`sdcard/yolov8n_coco80_192_u55_256.txt` |
| [yolov5_xiao_voc20.py](00_神经网络/yolov5_xiao_voc20.py) | YOLOv5n6-xiao VOC20 目标检测 | `sdcard/yolov5n6_xiao_voc20_192_u55_256.tflite`<br>`sdcard/yolov5n6_xiao_voc20_192_u55_256.txt` |
| [yolo_fastest_face.py](00_神经网络/yolo_fastest_face.py) | YOLO-Fastest 灰度人脸框 | `sdcard/yolo_fastest_face_192_u55_256.tflite`<br>`sdcard/yolo_fastest_face_192_u55_256.txt` |
| [swift_yolo_gesture.py](00_神经网络/swift_yolo_gesture.py) | 剪刀、石头、布手势检测框 | `sdcard/swift_yolo_gesture_192_u55_256.tflite`<br>`sdcard/swift_yolo_gesture_192_u55_256.txt` |
| [swift_yolo_pet.py](00_神经网络/swift_yolo_pet.py) | 猫狗检测框 | `sdcard/swift_yolo_pet_192_u55_256.tflite`<br>`sdcard/swift_yolo_pet_192_u55_256.txt` |
| [swift_yolo_person.py](00_神经网络/swift_yolo_person.py) | 行人检测框 | `sdcard/swift_yolo_person_nano_192_u55_256.tflite`<br>`sdcard/swift_yolo_person_nano_192_u55_256.txt` |
| [yolov8n_pose.py](00_神经网络/yolov8n_pose.py) | YOLOv8n-pose 多人框、17关键点和骨架 | `sdcard/yolov8n_pose_192_u55_256.tflite`<br>`sdcard/yolov8n_pose_192_u55_256.txt` |
| [unet_pet_segmentation.py](00_神经网络/unet_pet_segmentation.py) | U-Net 宠物/背景/边界三类像素分割（需SD） | `sdcard/unet_mobilenetv2_128_u55_256.tflite`<br>`sdcard/unet_mobilenetv2_128_u55_256.txt` |
| [classification_vww96.py](00_神经网络/classification_vww96.py) | VWW 有人/无人分类 | `sdcard/vww96_u55_256.tflite`<br>`sdcard/vww96_u55_256.txt` |
| [classification_resnet8.py](00_神经网络/classification_resnet8.py) | ResNet8 CIFAR10 分类 | `sdcard/resnet8_u55_256.tflite`<br>`sdcard/resnet8_u55_256.txt` |
| [classification_mobilenetv1_025.py](00_神经网络/classification_mobilenetv1_025.py) | MobileNet ImageNet 1000 类分类 | `sdcard/mobilenetv1_025_u55_256.tflite`<br>`sdcard/mobilenetv1_025_u55_256.txt` |
| [blazeface_renesas.py](00_神经网络/blazeface_renesas.py) | Renesas INT8 BlazeFace 人脸检测 | `sdcard/blazeface128_u55_256.tflite` |
| [tf_regression.py](00_神经网络/tf_regression.py) | TFLM 数值回归与 ulab 数组 | 见脚本头部的 `/omv_assets/ml/` 文件 |
| [face_detection.py](01_Haar与传统学习/face_detection.py) | Haar 人脸检测 | 见脚本中的官方 `/rom/haarcascade_*.cascade` 路径 |
| [face_eye_detection.py](01_Haar与传统学习/face_eye_detection.py) | Haar 人脸及双眼检测 | 见脚本中的官方 `/rom/haarcascade_*.cascade` 路径 |
| [face_tracking.py](01_Haar与传统学习/face_tracking.py) | Haar 检测与关键点跟踪 | 见脚本中的官方 `/rom/haarcascade_*.cascade` 路径 |
| [iris_detection.py](01_Haar与传统学习/iris_detection.py) | 近距离眼睛与瞳孔检测 | 见脚本中的官方 `/rom/haarcascade_*.cascade` 路径 |
| [face_recognition.py](01_Haar与传统学习/face_recognition.py) | 官方 AT&T 数据集 LBP 人脸比较 | 自行准备存储根目录下的 `orl_faces/` 数据集 |

## 使用要点

| 场景 | 配置 |
|---|---|
| 常见物体检测 | `yolov8_coco80_detector.py`，COCO80 类别，默认阈值 0.4 |
| 轻量人脸中心检测 | `tf_object_detection.py`，FOMO 96×96 |
| 多脸 / 多手 | 每帧默认最多 2 个目标，共用相应模型 |
| 图像分类 | 输出类别分数；显示的矩形为分类输入区域 |
| U-Net 分割 | 宠物 / 背景 / 边界三类，使用 SD 卡 |
| Haar / 瞳孔 | 使用清晰正脸或近距离眼部图像，按目标距离调焦 |
| LBP 人脸比较 | 另行准备 AT&T `orl_faces/` 数据集，至少 s1..s5、每组 1..10.pgm |
| 数值回归 | 使用 `force_int_quant.tflite`，由 CPU 执行 |

一次运行一个示例；切换模型前停止脚本并软复位。模型、视觉文件和用户文件存放在 SD 卡上。

## 素材

Haar 文件位于 [assets](assets/)，复制到板端根目录。将同目录的 `force_int_quant.tflite` 复制到板端 `/omv_assets/ml/force_int_quant.tflite`；按需创建 `omv_assets/ml/` 子目录。其他素材按对应脚本头部说明复制。

模型参数和许可见 [模型清单](../../models/MODEL_MANIFEST.json) 与 [许可索引](../../models/LICENSE_INDEX.json)。素材来源见 [assets/README.md](assets/README.md)。
