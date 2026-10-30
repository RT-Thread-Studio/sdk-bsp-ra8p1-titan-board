# Titan Board RA8P1: Renesas INT8 BlazeFace 人脸检测
# Source: examples/blazeface_detection.py
# Prerequisites and exact copy paths: ../README.md.
# Copy models/sdcard/blazeface128_u55_256.tflite -> /blazeface128_u55_256.tflite

# SPDX-License-Identifier: MIT
# Renesas quantized INT8 BlazeFace; four dequantized heads match BlazeFace.
import csi
import time
import ml
from ml.postprocessing.mediapipe import BlazeFace

model = ml.Model("/rom/blazeface128_u55_256.tflite", postprocess=BlazeFace(threshold=0.4))
if tuple(model.input_shape[0]) != (1, 128, 128, 3) or model.input_dtype[0] != "b":
    raise ValueError("Use the supplied Renesas INT8 BlazeFace model")
expected_outputs = ((1, 512, 16), (1, 512, 1), (1, 384, 1), (1, 384, 16))
if tuple(tuple(shape) for shape in model.output_shape) != expected_outputs:
    raise ValueError("BlazeFace requires the matching four output heads")
camera = csi.CSI()
camera.reset()
camera.pixformat(csi.RGB565)
camera.framesize(csi.QVGA)
camera.window((240, 240))
camera.snapshot(time=2000)
clock = time.clock()
last_print = time.ticks_ms()
while True:
    clock.tick()
    img = camera.snapshot()
    # The supplied INT8 input uses scale=1/255, zero_point=-128: q=p-128.
    for rect, score, keypoints in model.predict([img]):
        img.draw_rectangle(rect, color=(0, 255, 0))
        ml.utils.draw_keypoints(img, keypoints, radius=2, color=(255, 0, 0))
    if time.ticks_diff(time.ticks_ms(), last_print) >= 1000:
        print("BlazeFace", clock.fps(), "fps")
        last_print = time.ticks_ms()
