# SPDX-License-Identifier: Apache-2.0
# Titan Board I2C0: SDA P409, SCL P410, shared with the J9 camera.
# Run while no camera script is active. This scans addresses; it writes no registers.
from machine import I2C

bus = I2C(0, freq=400000)
print("I2C0:", [hex(address) for address in bus.scan()])
# I2C1 (P511/P512, CEU connector) is not a Mini onboard IMU bus.
