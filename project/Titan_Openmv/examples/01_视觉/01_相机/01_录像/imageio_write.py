# Titan Board OpenMV example: 图像流写入
# Source: ThirdParty/openmv/scripts/examples/01-Camera/01-Video-Recording/imageio_write.py
# Adaptation: LED 名称改为当前板级接口的数字编号（1/3）。
# Adaptation: 原始图像流固定录制 24 帧，避免十秒高速录制写满 6 MiB Flash。
# Adaptation: 完成后正常提示，不以人为 Exception 表示成功。

# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Image Writer Example
#
# NOTE: This example requires an SD card.
#
# This example shows how to use the Image Writer object to record a raw video file
# for later analysis using the Image Reader object.

import csi
import image
import time
import machine

N_FRAMES = 24  # QQVGA RGB565 raw frames: about 900 KiB plus headers.

csi0 = csi.CSI()
csi0.reset()  # Reset and initialize the sensor.
csi0.pixformat(csi.RGB565)  # Set pixel format to RGB565 (or GRAYSCALE)
csi0.framesize(csi.QQVGA)  # Set frame size to QQVGA (160x120)
csi0.snapshot(time=2000)  # Wait for settings take effect.
clock = time.clock()  # Create a clock object to track the FPS.

led = machine.LED(1)
stream = image.ImageIO("stream.bin", "w")

# Red LED on means we are capturing frames.
led.on()

for _ in range(N_FRAMES):
    clock.tick()
    img = csi0.snapshot()
    # Modify the image if you feel like here...
    stream.write(img)
    print(clock.fps())

stream.close()
led.off()

print("Saved. Stop the script before opening the file on the computer.")
