# SPDX-License-Identifier: Apache-2.0
# 系统与内存信息 — RA8P1 Titan Board
# 本工程新增示例；使用当前固件的 MicroPython 标准库与 board 模块。
# 条件: 无需外接；可在视觉资源加载前使用基础 Python/board 接口。
# 适配: 只读取 CPU 频率；当前不提供动态调频、RTC、WDT、sleep/deepsleep。

import sys
import gc
import machine
import board
import binascii

print("implementation:", sys.implementation)
print("platform:", sys.platform)
print("CPU:", machine.freq(), "Hz")
print("unique ID:", binascii.hexlify(machine.unique_id()))
print("board (Hz, I-cache, D-cache, MVE, name):", board.info())
gc.collect()
print("Python heap allocated:", gc.mem_alloc())
print("Python heap free:", gc.mem_free())
print("Memory pools:", board.memory())
print("MSC media currently visible:", board.usb_msc())
