# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
# 周期软件定时器 — RA8P1 Titan Board
# 来源: ThirdParty/openmv/scripts/examples/50-OpenMV-Boards/52-Alif-Boards/50-Board-Control/timer_control.py
# 条件: 无需外接；Timer 为 VM 调度器软件定时器，精度会受图像处理等任务影响。
# 适配: 本端口使用 period 毫秒参数，不接受 freq；LED 用数字编号。

import time
from machine import Timer, LED

led = LED(3)

def tick(timer):
    led.toggle()

timer = Timer(-1)
try:
    timer.init(mode=Timer.PERIODIC, period=500, callback=tick)
    while True:
        time.sleep_ms(100)
finally:
    timer.deinit()
    led.off()
