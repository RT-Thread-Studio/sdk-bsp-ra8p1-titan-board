## RUHMI Model Zoo

BSD 3-Clause License

Copyright (c) 2026, Renesas Electronics

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


## Third-Party Models and Data Licenses

The repository source code is released under BSD-3-Clause (see `LICENSE`).
Third-party models and data/assets remain under their original licenses.

| Use Case | Model License | Sample Inputs Included in GitHub | Validation/Calibration Dataset Used by Scripts |
|----------|---------------|-----------------------------------|-----------------------------------------------|
| MobileNetV1 (Image Classification) | Apache 2.0 (TensorFlow/Keras weights) | COCO sample images in `sample_images/` (CC-BY 4.0) | ImageNet-1k validation set from image-net.org (Terms of Access; user-provided for INT8) |
| MobileNetV2 (Image Classification) | Apache 2.0 (TensorFlow/Keras weights) | COCO sample images in `sample_images/` (CC-BY 4.0) | ImageNet-1k validation set from image-net.org (Terms of Access; user-provided for INT8) |
| MobileNetV3 (Image Classification) | Apache 2.0 (TensorFlow/Keras weights) | COCO sample images in `sample_images/` (CC-BY 4.0) | ImageNet-1k validation set from image-net.org (Terms of Access; user-provided for INT8) |
| ShuffleNetV2 (Image Classification) | BSD-3-Clause (torchvision weights) | COCO sample images in `sample_images/` (CC-BY 4.0) | ImageNet-1k validation set from image-net.org (restricted/research-use terms; user-provided for INT8) |
| SqueezeNet 1.1 (Image Classification) | BSD-3-Clause (torchvision weights) | COCO sample images in `sample_images/` (CC-BY 4.0) | ImageNet-1k validation set from image-net.org (Terms of Access; user-provided for INT8) |
| ResNet8 (Image Classification) | Apache 2.0 (MLCommons Tiny) | CIFAR-10 sample images in `sample_images/` (dataset terms) | CIFAR-10 dataset from University of Toronto (dataset terms) |
| Visual Wake Words (Image Classification) | Apache 2.0 (MLCommons Tiny) | COCO sample images in `sample_images/` (CC-BY 4.0) | COCO val2014 images/annotations (CC-BY 4.0; auto-downloaded if missing) |
| YOLO-Fastest 1.1 (Object Detection) | MIT | COCO sample images in `sample_images/` (CC-BY 4.0) | COCO dataset for calibration/evaluation (CC-BY 4.0) |
| YOLOX-Tiny (Object Detection) | Apache 2.0 (Megvii YOLOX) | COCO sample images in `sample_images/` (CC-BY 4.0) | COCO 2017 dataset for calibration/validation (CC-BY 4.0) |
| BlazeFace Front (Face Detection) | Apache 2.0 (MediaPipe/PINTO) | WIDER FACE sample images in `sample_images/` | WIDER FACE dataset for validation (Creative Common License) |
| Keyword Spotting DS-CNN (Audio Classification) | Apache 2.0 (MLCommons Tiny) | Speech Commands sample audio in `sample_audio/` (CC-BY 4.0) | Google Speech Commands v0.02 (CC-BY 4.0; downloaded via tfds when needed) |
| Auto Encoder (Anomaly Detection) | Apache 2.0 (MLCommons Tiny) | DCASE ToyCar sample audio in `sample_audio/` (CC-BY-SA 4.0) | DCASE 2020 Task 2 ToyCar dataset from Zenodo (CC-BY-SA 4.0) |

> Verify current upstream license terms before redistribution in your target product, geography, and commercial context.