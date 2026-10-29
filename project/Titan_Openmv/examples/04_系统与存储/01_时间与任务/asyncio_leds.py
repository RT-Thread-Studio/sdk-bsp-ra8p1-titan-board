# SPDX-License-Identifier: Apache-2.0
# asyncio 并发 LED — RA8P1 Titan Board
# 本工程新增示例；使用当前固件的 MicroPython 标准库与 board 模块。
# 条件: 无需安装库；当前固件已冻结 asyncio；三个任务采用协作调度。
# 适配: 使用 board LED 数字编号；取消任务和停止脚本时关闭 LED。

import asyncio
from machine import LED

leds = [LED(1), LED(2), LED(3)]

async def blink(led, interval_ms):
    try:
        while True:
            led.toggle()
            await asyncio.sleep_ms(interval_ms)
    finally:
        led.off()

async def main():
    tasks = [asyncio.create_task(blink(led, interval))
             for led, interval in zip(leds, (250, 500, 750))]
    try:
        await asyncio.sleep(10)
    finally:
        for task in tasks:
            task.cancel()
        await asyncio.gather(*tasks, return_exceptions=True)

try:
    asyncio.run(main())
finally:
    for led in leds:
        led.off()
    asyncio.new_event_loop()
