# RA8P1-Titan Board Face Detection AI Model Deployment

This document explains how to deploy an AI model on the Renesas RA8P1 development board.

The required resources are provided in the shared cloud-drive file: `USE.zip`.

Download link: https://pan.baidu.com/s/1cighdC449cvuiGMGbRTNdw?pwd=ydic 
Extraction code: `ydic`

This document includes the following sections:

- AI model training
- AI model conversion
- AI model deployment

## 1. Environment Setup

### 1.1 Darknet

Refer to [How to compile Darknet on Windows using CMake](https://github.com/AlexeyAB/darknet?tab=readme-ov-file#how-to-compile-on-windows-using-cmake) to build the Darknet training framework.

Because installing tools such as VS 2022 and CMake can take a long time, a prebuilt `darknet.exe` is provided at `\USE\workspace\Yolo-Fastest\darknet.exe`. You can use it to train the model directly on the CPU. If you need to train the model on a GPU, rebuild Darknet by following the official Darknet installation steps.

Before compiling, modify the configuration file according to your requirements.

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

### 1.2 Install the Python Environment

Python 3.10.0 is required for this guide. Download it from the following link:

[Python Release Python 3.10.0 | Python.org](https://www.python.org/downloads/release/python-3100/)

Select the version for your operating system. You can choose a custom download path.

![python](figures/python.png)

If another Python version was installed previously, uninstall it first. Otherwise, the installation may fail. Alternatively, place the Python 3.10 path above other Python paths in the environment variables so that the system uses the first Python path by default.

![606f80c7-5fc2-4b26-805d-4093995bd07a](figures/606f80c7-5fc2-4b26-805d-4093995bd07a.png)

Open a CMD terminal and run `python --version` to check whether the current Python version is 3.10.

![c1691d7b-c5a0-4b12-af68-22b066493553](figures/c1691d7b-c5a0-4b12-af68-22b066493553.png)

Extract each archive under the `USE` resource package, as shown below:

![f79d7287-ad5d-4cb8-b83c-8505831635e4](figures/f79d7287-ad5d-4cb8-b83c-8505831635e4.png)

### 1.3 Set Up the Convert Environment

This environment is used for model conversion:

```text
Darknet .weights
        ->
Keras .h5
        ->
INT8 quantization
        ->
TFLite .tflite
```

Extract `convert.zip`.

### 1.4 Set Up the RUHMI Environment

GitHub: [renesas/ruhmi-framework-mcu: Renesas RUHMI Framework supports AI model optimization and deployment, and is powered by EdgeCortix MERA](https://github.com/renesas/ruhmi-framework-mcu)

`ruhmi-framework-mcu` is **Renesas' MCU AI deployment framework**. It mainly performs three tasks:

1. Model compilation: compiles the `.tflite` model into a **computation graph suitable for MCU/NPU execution**
2. NPU acceleration adaptation
3. MCU project code generation

```text
TFLite INT8
      ->
MERA
      ->
Ethos-U55 NPU
      ->
MCU runtime
```

Extract `ruhmi-framework-mcu-Release-2026-02-02.zip`.

Open a CMD terminal in the extracted directory, then run:

```powershell
# Switch pip to a China mirror
pip config set global.index-url https://mirrors.aliyun.com/pypi/simple/

# Create a virtual environment
python -m venv mera-env

# Activate the virtual environment
.\mera-env\Scripts\activate

# Install the MERA toolchain, which is Renesas' AI model compiler/deployment toolchain
pip install .\install\mera-2.5.0+pkg.3577-cp310-cp310-win_amd64.whl
```

The computer must be connected to the internet because some installation dependencies will be downloaded online. This step may take some time.

![c08cd6de-9981-4a1d-bdf8-93c2509b11cf](figures/c08cd6de-9981-4a1d-bdf8-93c2509b11cf.png)

### 1.5 Set Up the RT-Thread Studio Environment

Run `RT-Thread_Studio_2.2.9-setup.exe` and install it in a path that contains only English characters.

![0028128e-79cb-4b65-a061-e860004eb941](figures/0028128e-79cb-4b65-a061-e860004eb941.png)

After installation, click SDK Manager in the upper-left corner.

![7c6e22b5-c569-45aa-a10e-9cc2056fe0d8](figures/7c6e22b5-c569-45aa-a10e-9cc2056fe0d8.png)

Click Operation, select Import, and choose `sdk-bsp-ra8p1-titan-board-1.0.2.zip`.

![7f27704d-8a41-498b-98c7-eafc9bef4839](figures/7f27704d-8a41-498b-98c7-eafc9bef4839.png)

![8ce69122-b1e5-4f36-b525-cf6d0267ba76](figures/8ce69122-b1e5-4f36-b525-cf6d0267ba76.png)

After import, the offline package can be found under `Board_Support_Packages/Renesas`. RT-Thread Studio will also help install the latest ARM toolchain and the J-Link Debug tool.

![6cc2fdb1-5047-4586-af3f-7b8d0d6f04e4](figures/6cc2fdb1-5047-4586-af3f-7b8d0d6f04e4.png)

![a82582af-f843-405a-aa9c-04235705e39b](figures/a82582af-f843-405a-aa9c-04235705e39b.png)

## 2. AI Model Training

Extract `workspace.zip`.

### Train the Model

Use the provided `\USE\workspace\Yolo-Fastest\darknet.exe`. Open a CMD terminal in the `\USE\workspace\Yolo-Fastest` directory and run the following command. The CPU will start training the model.

```powershell
darknet.exe detector train datasets/voc.data  datasets/yolo-fastest.cfg -dont_show
```

![6d68b534-e86f-4f90-8cde-6b688c3cb298](figures/6d68b534-e86f-4f90-8cde-6b688c3cb298.png)

![36720123-a704-4bc3-9828-892b16c0875e](figures/36720123-a704-4bc3-9828-892b16c0875e.png)

After training is complete, the model weights are saved in `\USE\workspace\Yolo-Fastest\backup`.

![099cd0ef-4941-4792-b221-8f7261c36261](figures/099cd0ef-4941-4792-b221-8f7261c36261.png)

Select the weights from the last training iteration for conversion. The dataset and the number of training iterations used in this document have both been reduced.

## 3. AI Model Conversion

### 3.1 Darknet to TFLite

The required conversion configuration is shown below:

![e2bd8f31-2695-4757-9b74-da8183c14aaa](figures/e2bd8f31-2695-4757-9b74-da8183c14aaa.png)

Copy the trained weights file (`\USE\workspace\Yolo-Fastest\backup\yolo-fastest_last.weights`) and the cfg file (`\USE\workspace\Yolo-Fastest\datasets\yolo-fastest.cfg`) to `\USE\workspace\keras-YOLOv3-model-set`.

Open a CMD terminal in the same directory.

First activate the convert environment:

```powershell
..\..\convert\Scripts\activate
```

At this point, `(convert)` should appear on the left side of the command prompt.

![db6473c9-2b30-4bd5-8e44-3e81020d9d47](figures/db6473c9-2b30-4bd5-8e44-3e81020d9d47.png)

Then run the following command:

```powershell
python tools/model_converter/conver2tflite_and_int8.py --config_path yolo-fastest.cfg --weights_path yolo-fastest_last.weights --output_path face.h5 --int8_img_path ../Yolo-Fastest/datasets/face/data_1/train/image/ --save_tflite_path face_int8.tflite -f
```

![8365dd9b-6ca1-4b7b-bda6-7f82651bb43c](figures/8365dd9b-6ca1-4b7b-bda6-7f82651bb43c.png)

Finally, the `face_int8.tflite` model will be generated in the root directory of `keras-YOLOv3-model-set`. This step takes about 5 to 10 minutes.

### 3.2 TFLite to RUHMI

Copy the generated `face_int8.tflite` file to `\USE\workspace\ruhmi-framework-mcu\models`.

Open a CMD terminal in the `USE\workspace\ruhmi-framework-mcu\scripts` directory.

First activate the RUHMI environment by running the following command in the CMD terminal:

```powershell
..\..\..\ruhmi-framework-mcu-Release-2026-02-02\mera-env\Scripts\activate
```

At this point, `(mera-env)` should appear on the left side of the command prompt.

![8a69b1f9-bbd0-4569-8fdc-e0fa124f66dd](figures/8a69b1f9-bbd0-4569-8fdc-e0fa124f66dd.png)

Then run the following command:

```powershell
python mcu_deploy.py --ethos --ref_data ../models/ deploy_qtzed_ethos
```

![f0b6d58c-e36d-486e-bfa8-54a361406ac7](figures/f0b6d58c-e36d-486e-bfa8-54a361406ac7.png)

## 4. AI Model Deployment

Extract `Titan_npu_ai_face_detection.zip`.

The TFLite-to-RUHMI step generates deployable model files under `\USE\workspace\ruhmi-framework-mcu\scripts\deploy_qtzed_ethos\face_int8_no_ospi\build\MCU\compilation\src`. Delete `*_io_data.c`, `*_io_data.h`, and `hal_entry.c`, then copy the remaining files to `\USE\Titan_npu_ai_face_detection\src\models`.

![1701a24a-a1db-4f43-a43a-6890226b3346](figures/1701a24a-a1db-4f43-a43a-6890226b3346.png)

Open RT-Thread Studio and import the example.

![0d53701a-53ca-4cfb-b9a1-8b5639bfefe4](figures/0d53701a-53ca-4cfb-b9a1-8b5639bfefe4.png)

![db8ea5c1-fe5c-461a-8cdb-0ba75ed8e5a7](figures/db8ea5c1-fe5c-461a-8cdb-0ba75ed8e5a7.png)

![2264f371-6efa-42b4-9a33-9b4c369c6403](figures/2264f371-6efa-42b4-9a33-9b4c369c6403.png)

Build and download:

![248a377b-8940-4a84-8dc6-53229b099977](figures/248a377b-8940-4a84-8dc6-53229b099977.png)

## 5. Running Result

Camera and screen wiring:

Use a 22-pin reverse-direction FFC cable to connect the CEU_CAM FFC connector on the development board to the DVP FFC connector on the camera adapter board.

![ef8a1d3f-0a5a-4adc-adb8-4f5fcfa45bd4](figures/ef8a1d3f-0a5a-4adc-adb8-4f5fcfa45bd4.png)

Use a 40-pin same-direction FFC cable to connect the LCD_RGB FFC connector on the development board to the screen.

![image-20260513135308719](figures/image-20260513135308719.png)

Running result:

![2adbeb8d-057a-4457-9a06-e011dfd82949](figures/2adbeb8d-057a-4457-9a06-e011dfd82949.png)
