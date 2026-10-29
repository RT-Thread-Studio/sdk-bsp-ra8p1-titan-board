# SPDX-License-Identifier: Apache-2.0
# Titan Board onboard user key P110: 0 = pressed. No external wiring.
from machine import Pin
import time

key = Pin("P110", Pin.IN, Pin.PULL_UP)
previous = None
while True:
    value = key.value()
    if value != previous:
        time.sleep_ms(20)
        if key.value() == value:
            print("pressed" if value == 0 else "released")
            previous = value
    time.sleep_ms(10)
