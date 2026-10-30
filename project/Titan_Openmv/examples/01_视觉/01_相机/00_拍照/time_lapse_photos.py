# Titan Board OpenMV example: 定时连拍（持续供电）
# Source: ThirdParty/openmv/scripts/examples/01-Camera/00-Snapshot/time_lapse_photos.py
# Adaptation: LED 名称改为当前板级接口的数字编号（1/3）。
# Adaptation: 保留定时拍照功能，使用持续供电的 ticks_ms/sleep_ms 定时；明确不提供本端口未实现的 RTC 唤醒/深睡眠。

# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
# Time-lapse adaptation for Titan Board: powered interval capture.
# RTC wakeup/deepsleep are not implemented by this port.
import csi
import os
import time

INTERVAL_MS = 10000
PHOTO_COUNT = 10
csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.RGB565)
csi0.framesize(csi.QVGA)
csi0.snapshot(time=2000)
if "timelapse" not in os.listdir():
    os.mkdir("timelapse")
for index in range(PHOTO_COUNT):
    started = time.ticks_ms()
    path = "timelapse/photo-%03d.jpg" % index
    csi0.snapshot().save(path, quality=80)
    print("Saved", path)
    while time.ticks_diff(time.ticks_ms(), started) < INTERVAL_MS:
        time.sleep_ms(50)
print("Time-lapse complete")
