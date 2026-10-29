# SPDX-License-Identifier: Apache-2.0
# 单调时间与周期任务 — RA8P1 Titan Board
# 本工程新增示例；使用当前固件的 MicroPython 标准库与 board 模块。
# 条件: 无需外接；ticks_ms/ticks_diff 用于间隔计算，不把启动时间当作日历 RTC。
# 适配: 用 ticks_add() 保持计划周期；超期后跳到下一次，避免连续补发。

import time

period_ms = 1000
deadline = time.ticks_add(time.ticks_ms(), period_ms)
count = 0
while True:
    now = time.ticks_ms()
    if time.ticks_diff(now, deadline) >= 0:
        count += 1
        print("periodic tick", count)
        deadline = time.ticks_add(deadline, period_ms)
        if time.ticks_diff(now, deadline) >= 0:
            deadline = time.ticks_add(now, period_ms)
    time.sleep_ms(10)
