# RA8P1—Titan Board 人脸识别AI模型部署

本文档主要用于熟悉如何在瑞萨的ra8p1开发板上部署模型。

资料在通过网盘分享的文件：USE.zip

链接:https://pan.baidu.com/s/1cighdC449cvuiGMGbRTNdw?pwd=ydic提取码: ydic

文档分为以下几个部分：

- ai模型训练
- ai模型转换
- ai模型部署

## 1. 环境安装

### 1.1 Darknet

参考 [https://github.com/AlexeyAB/darknet?tab=readme-ov-file#how-to-compile-on-windows-using-cmake](https://github.com/AlexeyAB/darknet?tab=readme-ov-file#how-to-compile-on-windows-using-cmake，将darknet训练框架编译好。) ，将darknet训练框架编译好。

由于需要安装 vs 2022、cmake 等工具，下载需要的时间过久，这边有编译好的darknet.exe（`\USE\workspace\Yolo-Fastest\darknet.exe`），可以使用cpu直接训练模型，如果你们需要使用gpu训练模型，可以按照darknet官方的安装步骤重新编译。

编译前注意修改配置文件，根据自己的需求修改。

```yaml
option(CMAKE_VERBOSE_MAKEFILE "Create verbose makefile" ON)
option(CUDA_VERBOSE_BUILD "Create verbose CUDA build" OFF)
option(BUILD_SHARED_LIBS "Create dark as a shared library" ON)
option(BUILD_AS_CPP "Build Darknet using C++ compiler also for C files" OFF)
option(BUILD_USELIB_TRACK "Build uselib_track" ON)
option(MANUALLY_EXPORT_TRACK_OPTFLOW "Manually export the TRACK_OPTFLOW=1 define" OFF)
option(ENABLE_OPENCV "Enable OpenCV integration" OFF)
option(ENABLE_CUDA "Enable CUDA support" OFF)
option(ENABLE_CUDNN "Enable CUDNN" OFF)
option(ENABLE_CUDNN_HALF "Enable CUDNN Half precision" OFF)
option(ENABLE_ZED_CAMERA "Enable ZED Camera support" OFF)
option(ENABLE_VCPKG_INTEGRATION "Enable VCPKG integration" OFF)
```

### 1.2 安装python环境

本次需要安装python 3.10.0环境，可通过下方链接下载：

[Python Release Python 3.10.0 | Python.org](https://www.python.org/downloads/release/python-3100/)

根据不同的操纵系统选择自己的版本，下载路径可以自定义。

![python](figures/python.png)

如果之前有下载过其他版本的，需要卸载，否则会安装失败。可以在环境变量中将3.10版本的python路径放在其他python路径之上即可，这样系统会默认调用第一个路径的python。

![606f80c7-5fc2-4b26-805d-4093995bd07a](figures/606f80c7-5fc2-4b26-805d-4093995bd07a.png)

可以打开CMD命令台，输入python --version，检查当前python版本是否为3.10。

![c1691d7b-c5a0-4b12-af68-22b066493553](figures/c1691d7b-c5a0-4b12-af68-22b066493553.png)

将资料包 USE 下面的压缩包分别进行解压缩，如下图所示：

![f79d7287-ad5d-4cb8-b83c-8505831635e4](figures/f79d7287-ad5d-4cb8-b83c-8505831635e4.png)

### 1.3 设置convert环境

该环境用来进行模型转化：

```
Darknet .weights
        ↓
Keras .h5
        ↓
INT8 量化
        ↓
TFLite .tflite
```

将`convert.zip`进行解压。

### 1.4 设置ruhmi环境

github：[renesas/ruhmi-framework-mcu: Renesas RUHMI Framework supports AI model optimization and deployment, and is powered by EdgeCortix® MERA™](https://github.com/renesas/ruhmi-framework-mcu)

`ruhmi-framework-mcu` 是 **Renesas 的 MCU AI部署框架**，它主要做三件事：

1. 模型编译 把 `.tflite` 模型编译成 **适合 MCU/NPU 的计算图**
2. NPU加速适配
3. 生成 MCU 工程代码

```
TFLite INT8
      ↓
MERA
      ↓
Ethos-U55 NPU
      ↓
MCU运行
```

将`ruhmi-framework-mcu-Release-2026-02-02.zip`进行解压。

在解压路径中打开CMD命令台，然后运行：

```powershell
# pip 换国内源
pip config set global.index-url https://mirrors.aliyun.com/pypi/simple/

# 创建虚拟环境
python -m venv mera-env

# 激活虚拟环境
.\mera-env\Scripts\activate

# 安装MERA工具（Renesas 的 AI 模型编译器 / 部署工具链）
pip install .\install\mera-2.5.0+pkg.3577-cp310-cp310-win_amd64.whl
```

电脑需要联网，因为会联网下载一些安装依赖，这一步需要花点时间等待下载完成。

![c08cd6de-9981-4a1d-bdf8-93c2509b11cf](figures/c08cd6de-9981-4a1d-bdf8-93c2509b11cf.png)

### 1.5 设置RT-Thread studio环境

点击`RT-Thread_Studio_2.2.9-setup.exe`，安装在英文路径上。

![0028128e-79cb-4b65-a061-e860004eb941](figures/0028128e-79cb-4b65-a061-e860004eb941.png)

安装好后，点击左上角的SDK管理器。

![7c6e22b5-c569-45aa-a10e-9cc2056fe0d8](figures/7c6e22b5-c569-45aa-a10e-9cc2056fe0d8.png)

点击操作，然后选择导入，选择`sdk-bsp-ra8p1-titan-board-1.0.2.zip`。

![7f27704d-8a41-498b-98c7-eafc9bef4839](figures/7f27704d-8a41-498b-98c7-eafc9bef4839.png)

![8ce69122-b1e5-4f36-b525-cf6d0267ba76](figures/8ce69122-b1e5-4f36-b525-cf6d0267ba76.png)

导入后会在`Board_Support_Packages/Renesas`下找到离线包，同时他也会帮助我们安装最新的ARM工具链还有J-Link Debug工具。

![6cc2fdb1-5047-4586-af3f-7b8d0d6f04e4](figures/6cc2fdb1-5047-4586-af3f-7b8d0d6f04e4.png)

![a82582af-f843-405a-aa9c-04235705e39b](figures/a82582af-f843-405a-aa9c-04235705e39b.png)

## 2. AI模型训练

解压`workspace.zip`。

### 训练模型

使用提供好的`\USE\workspace\Yolo-Fastest\darknet.exe`，在`\USE\workspace\Yolo-Fastest`目录下启动CMD命令台，输入以下指令，CPU就开始训练模型了。

```powershell
darknet.exe detector train datasets/voc.data  datasets/yolo-fastest.cfg -dont_show
```

![6d68b534-e86f-4f90-8cde-6b688c3cb298](figures/6d68b534-e86f-4f90-8cde-6b688c3cb298.png)

![36720123-a704-4bc3-9828-892b16c0875e](figures/36720123-a704-4bc3-9828-892b16c0875e.png)

模型训练结束后，会在`\USE\workspace\Yolo-Fastest\backup`中保存模型权重。

![099cd0ef-4941-4792-b221-8f7261c36261](figures/099cd0ef-4941-4792-b221-8f7261c36261.png)

我们选择最后一轮的权重，用于转换。（本文档使用的数据集和训练轮次都是减少过的）

## 3. AI模型转换

### 3.1 darknet->tflite

转换所需配置如下：

![e2bd8f31-2695-4757-9b74-da8183c14aaa](figures/e2bd8f31-2695-4757-9b74-da8183c14aaa.png)

我们需要复制训练好的权重(`\USE\workspace\Yolo-Fastest\backup\yolo-fastest_last.weights`)、cfg文件(`\USE\workspace\Yolo-Fastest\datasets\yolo-fastest.cfg`)

到`\USE\workspace\keras-YOLOv3-model-set`下，也在该目录下启动CMD命令台。

在先进入到convert环境中，运行：

```
..\..\convert\Scripts\activate
```

这时候应该左侧会出现(convert)

![db6473c9-2b30-4bd5-8e44-3e81020d9d47](figures/db6473c9-2b30-4bd5-8e44-3e81020d9d47.png)

接着再输入下面的指令：

```powershell
python tools/model_converter/conver2tflite_and_int8.py --config_path yolo-fastest.cfg --weights_path yolo-fastest_last.weights --output_path face.h5 --int8_img_path ../Yolo-Fastest/datasets/face/data_1/train/image/ --save_tflite_path face_int8.tflite -f
```

![8365dd9b-6ca1-4b7b-bda6-7f82651bb43c](figures/8365dd9b-6ca1-4b7b-bda6-7f82651bb43c.png)

最终会在 `keras-YOLOv3-model-set` 根目录下得到 `face_int8.tflite`模型，这步大概需要5-10分钟。

### 3.2 tflite->ruhmi

将上一步得到的`face_int8.tflite`文件复制到`\USE\workspace\ruhmi-framework-mcu\models`下。

在`USE\workspace\ruhmi-framework-mcu\scripts`目录下启动CMD命令台。

先进入到 ruhmi 环境中，在CMD命令台中运行以下指令：

```
..\..\..\ruhmi-framework-mcu-Release-2026-02-02\mera-env\Scripts\activate
```

这时候应该左侧会出现(mera-env):

![8a69b1f9-bbd0-4569-8fdc-e0fa124f66dd](figures/8a69b1f9-bbd0-4569-8fdc-e0fa124f66dd.png)

然后在输入下面的命令：

```powershell
python mcu_deploy.py --ethos --ref_data ../models/ deploy_qtzed_ethos
```

![f0b6d58c-e36d-486e-bfa8-54a361406ac7](figures/f0b6d58c-e36d-486e-bfa8-54a361406ac7.png)

## 4. AI模型部署

解压`Titan_npu_ai_face_detection.zip`。

tflite->ruhmi会在`\USE\workspace\ruhmi-framework-mcu\scripts\deploy_qtzed_ethos\face_int8_no_ospi\build\MCU\compilation\src`下生成可部署模型文件，删除 `*_io_data.c`, `*_io_data.h` 和 `hal_entry.c`， 然后将剩下的文件拷贝到`\USE\Titan_npu_ai_face_detection\src\models`下。

![1701a24a-a1db-4f43-a43a-6890226b3346](figures/1701a24a-a1db-4f43-a43a-6890226b3346.png)

打开rt-thread studio，导入实例。

![0d53701a-53ca-4cfb-b9a1-8b5639bfefe4](figures/0d53701a-53ca-4cfb-b9a1-8b5639bfefe4.png)

![db8ea5c1-fe5c-461a-8cdb-0ba75ed8e5a7](figures/db8ea5c1-fe5c-461a-8cdb-0ba75ed8e5a7.png)

![2264f371-6efa-42b4-9a33-9b4c369c6403](figures/2264f371-6efa-42b4-9a33-9b4c369c6403.png)

编译下载：

![248a377b-8940-4a84-8dc6-53229b099977](figures/248a377b-8940-4a84-8dc6-53229b099977.png)

## 5. 运行结果

摄像头与屏幕接线：

使用 22Pin 异向排线连接开发板的 CEU_CAM 排线座与摄像头转接板的 DVP 排线座。

![ef8a1d3f-0a5a-4adc-adb8-4f5fcfa45bd4](figures/ef8a1d3f-0a5a-4adc-adb8-4f5fcfa45bd4.png)

使用 40Pin 同向排线连接开发板的 LCD_RGB 排线座与屏幕。

![image-20260513135308719](figures/image-20260513135308719.png)

运行结果：

![2adbeb8d-057a-4457-9a06-e011dfd82949](figures/2adbeb8d-057a-4457-9a06-e011dfd82949.png)