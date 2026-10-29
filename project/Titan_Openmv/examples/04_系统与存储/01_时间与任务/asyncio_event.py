# SPDX-License-Identifier: Apache-2.0
# asyncio 事件通知 — RA8P1 Titan Board
# 本工程新增示例；使用当前固件的 MicroPython 标准库与 board 模块。
# 条件: 无需外接；演示两个协作任务用 Event 交接工作，不使用网络或线程模块。
# 适配: Event 在 asyncio 任务中设置；不从硬件 IRQ 直接操作 Event。

import asyncio
from machine import LED

event = asyncio.Event()
led = LED(2)

async def producer():
    for index in range(5):
        await asyncio.sleep_ms(500)
        event.set()

async def consumer():
    for index in range(5):
        await event.wait()
        event.clear()
        led.toggle()
        print("event", index + 1)

async def main():
    await asyncio.gather(producer(), consumer())

try:
    asyncio.run(main())
finally:
    led.off()
    asyncio.new_event_loop()
