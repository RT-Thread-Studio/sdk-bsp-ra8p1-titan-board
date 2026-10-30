# Titan Board OpenMV example: 内存图像流录放
# Source: ThirdParty/openmv/scripts/examples/01-Camera/01-Video-Recording/imageio_memory.py
# Adaptation: 内存录制由 500 帧降为 20 帧，120×120 RGB565 约 563 KiB，适配本项目内存。

# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Image Memory Stream I/O Example
#
# This example shows how to use the ImageIO stream to record frames in memory and play them back.
# Note: While this should work on any board, the board should have an HyperRAM to be of any use.
import csi
import image
import time

# Number of frames to pre-allocate and record
N_FRAMES = 20

csi0 = csi.CSI()
csi0.reset()  # Reset and initialize the sensor.
csi0.pixformat(csi.RGB565)  # Set pixel format to RGB565 (or GRAYSCALE)
csi0.framesize(csi.QVGA)  # Set frame size to QVGA (320x240)

# This frame size must match the image size passed to ImageIO
csi0.window((120, 120))
csi0.snapshot(time=2000)

clock = time.clock()

# Write to memory stream
stream = image.ImageIO((120, 120, csi.RGB565), N_FRAMES)

for i in range(0, N_FRAMES):
    clock.tick()
    stream.write(csi0.snapshot())
    print(clock.fps())

while True:
    # Rewind stream and play back
    stream.seek(0)
    for i in range(0, N_FRAMES):
        img = stream.read(copy_to_fb=True, pause=True)
        # Do machine vision algorithms on the image here.
