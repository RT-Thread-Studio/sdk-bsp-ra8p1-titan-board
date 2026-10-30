# SPDX-License-Identifier: Apache-2.0
# Titan Board: LED 1/2/3 = red P013 / green P613 / blue P012, active low.
import board
import time

leds = [board.LED(i) for i in (1, 2, 3)]
try:
    while True:
        for led in leds:
            led.on()
            time.sleep_ms(250)
            led.off()
finally:
    for led in leds:
        led.off()
