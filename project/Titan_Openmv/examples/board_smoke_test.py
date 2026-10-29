# SPDX-License-Identifier: Apache-2.0
# Titan Board first-boot checks: no SD writes or raw RAM writes.
import board
import gc
import os
import time
from machine import Pin

print("Board:", board.info())
gc.collect()
print("Memory:", board.memory())
print("User key P110 (0 = pressed):", Pin("P110", Pin.IN, Pin.PULL_UP).value())
leds = [board.LED(i) for i in (1, 2, 3)]
try:
    for led in leds:
        led.on()
        time.sleep_ms(300)
        led.off()
finally:
    for led in leds:
        led.off()

try:
    print("SD root:", os.listdir("/"))
    with open("/openmv-vision.bin", "rb") as stream:
        print("Vision file prefix:", stream.read(16))
    print("File present; firmware loader must still verify its build ID and checksum.")
except OSError as error:
    print("SD/vision file unavailable:", error)
print("Next: run the camera helloworld example and inspect the IDE image.")
