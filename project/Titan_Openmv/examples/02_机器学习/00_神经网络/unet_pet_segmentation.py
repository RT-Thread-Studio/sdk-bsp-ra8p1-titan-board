# SPDX-License-Identifier: Apache-2.0
# Titan Board: U-Net / MobileNetV2 Oxford-IIIT Pet segmentation (128 x 128).
# Based on the Google Coral semantic-segmentation model and example:
# https://coral.ai/models/semantic-segmentation/
# https://github.com/google-coral/pycoral/blob/master/examples/semantic_segmentation.py
# Use SD storage. Copy unet_mobilenetv2_128_u55_256.tflite and its .txt to
# the SD root; /rom is its read-only alias. Model licenses are supplied separately.
# This model segments pets, background and borders, not people or 37 pet breeds.

import csi
import image
import ml
import time
from ulab import numpy as np

MODEL_PATH = "/rom/unet_mobilenetv2_128_u55_256.tflite"
MASK_WIDTH = 128
MASK_HEIGHT = 128


def pet_mask_callback(model, inputs, outputs):
    # All three channels use the same positive quantization scale/zero point,
    # so UINT8 argmax gives exactly the same classes as dequantized logits.
    # A callback reads the NPU buffer directly, avoiding a 192 KiB float copy.
    # ulab argmax returns int16; explicitly compact the mask to 16 KiB uint8.
    return np.array(np.argmax(outputs[0][0], axis=2), dtype=np.uint8)


model = ml.Model(MODEL_PATH, postprocess=pet_mask_callback)
if (len(model.input_shape) != 1
        or tuple(model.input_shape[0]) != (1, MASK_HEIGHT, MASK_WIDTH, 3)
        or model.input_dtype[0] != "B"
        or abs(model.input_scale[0] - 2.0 / 255.0) > 1e-8
        or model.input_zero_point[0] != 127
        or len(model.output_shape) != 1
        or tuple(model.output_shape[0]) != (1, MASK_HEIGHT, MASK_WIDTH, 3)
        or model.output_dtype[0] != "B"
        or abs(model.output_scale[0] - 0.16552436351776123) > 1e-7
        or model.output_zero_point[0] != 120):
    raise ValueError("Use the supplied U-Net MobileNetV2 128 U55-256 model")
print(model)

# The source model's embedded labels are exactly: pet, background, border.
# Palettes must have 256 entries. Background (index 1) is fully transparent.
colors = image.Image(256, 1, image.RGB565)
alpha = image.Image(256, 1, image.GRAYSCALE)
colors.set_pixel((0, 0), (0, 255, 0))
colors.set_pixel((2, 0), (255, 255, 0))
alpha.set_pixel((0, 0), 112)
alpha.set_pixel((2, 0), 192)

camera = csi.CSI()
camera.reset()
camera.pixformat(csi.RGB565)
camera.framesize(csi.QVGA)
camera.snapshot(time=2000)
clock = time.clock()
last_print = time.ticks_ms()

while True:
    clock.tick()
    img = camera.snapshot()
    # Official integer Normalization resizes the entire RGB image to 128x128
    # and copies RGB bytes, as in Coral's UINT8 example. Do not divide by 255.
    classes = model.predict([img])
    # Keep classes alive while mask wraps its storage (no extra mask copy).
    mask = image.Image(MASK_WIDTH, MASK_HEIGHT, image.GRAYSCALE, buffer=classes)
    # Match the input's full-image resize; nearest-neighbour retains class IDs.
    img.draw_image(mask, 0, 0, x_scale=img.width() / MASK_WIDTH,
                   y_scale=img.height() / MASK_HEIGHT, color_palette=colors,
                   alpha_palette=alpha)
    img.draw_rectangle((0, 0, 232, 12), color=(0, 0, 0), fill=True)
    img.draw_string((1, 1), "pet=green  border=yellow", color=(255, 255, 255))
    if time.ticks_diff(time.ticks_ms(), last_print) >= 1000:
        print("U-Net pet segmentation", clock.fps(), "fps")
        last_print = time.ticks_ms()
