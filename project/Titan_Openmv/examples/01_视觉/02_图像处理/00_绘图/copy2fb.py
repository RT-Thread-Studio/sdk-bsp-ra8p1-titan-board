# Titan Board OpenMV example: 图片文件加载到帧缓冲
# Source: ThirdParty/openmv/scripts/examples/02-Image-Processing/00-Drawing/copy2fb.py
# Adaptation: 自动绘制并保存 example.bmp，然后加载到帧缓冲显示，无缺失素材。

# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Copy image to framebuffer.
#
# This example shows how to load and display an image.

import image
import time

# Generate a BMP locally before loading it; no external example.bmp is needed.
sample = image.Image(160, 120, image.RGB565)
sample.clear()
sample.draw_rectangle((8, 8, 144, 104), color=(0, 128, 255), fill=True)
sample.draw_circle((80, 60, 30), color=(255, 220, 0), fill=True)
sample.draw_string((12, 12), "OpenMV", color=(255, 255, 255))
sample.save("example.bmp")

# Load image
img = image.Image("example.bmp", copy_to_fb=True)

# Send the loaded image to the IDE for display.
img.flush()

# Add a small delay to allow the IDE to read the image.
time.sleep_ms(1000)
