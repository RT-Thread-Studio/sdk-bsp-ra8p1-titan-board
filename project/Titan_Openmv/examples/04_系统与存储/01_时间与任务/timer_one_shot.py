# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
# 一次性软件定时器 — RA8P1 Titan Board
# 来源: ThirdParty/openmv/scripts/examples/50-OpenMV-Boards/52-Alif-Boards/50-Board-Control/timer_control.py
# 条件: 无需外接；延迟 2 秒后点亮 LED，再由主循环退出。
# 适配: 使用 ONE_SHOT/period；回调只置标志，退出释放 Timer 槽位。

import time
from machine import Timer, LED

led = LED(1)
fired = False

def expire(timer):
    global fired
    fired = True

timer = Timer(-1)
try:
    timer.init(mode=Timer.ONE_SHOT, period=2000, callback=expire)
    while not fired:
        time.sleep_ms(10)
    led.on()
    print("One-shot timer fired")
    time.sleep_ms(1000)
finally:
    timer.deinit()
    led.off()
