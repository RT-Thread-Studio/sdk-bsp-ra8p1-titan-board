# Titan Board OpenMV example: 图像流读取（自动生成素材）
# Source: ThirdParty/openmv/scripts/examples/01-Camera/01-Video-Recording/imageio_read.py
# Adaptation: 自动录制 12 帧生成 stream.bin 后回放，不依赖缺失输入文件。

# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Image Reader Example
#
# NOTE: This example requires an SD card.
#
# This example shows how to use the Image Reader object to replay a raw video file.

import csi
import image
import time

# Generate a small sample first, so no separately recorded input is needed.
csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.RGB565)
csi0.framesize(csi.QQVGA)
csi0.snapshot(time=2000)
writer = image.ImageIO("stream.bin", "w")
for _ in range(12):
    writer.write(csi0.snapshot())
writer.close()
stream = image.ImageIO("stream.bin", "r")

clock = time.clock()  # Create a clock object to track the FPS.
while True:
    clock.tick()
    img = stream.read(copy_to_fb=True, loop=True, pause=True)
    # Do machine vision algorithms on the image here.
    print(clock.fps())
