# SPDX-License-Identifier: MIT
# Continuous QVGA capture; stop on the first error and retain diagnostics.
import board
import csi
import time

DURATION_MS = 10 * 60 * 1000
REPORT_EVERY_MS = 5000
PREVIEW = True

board.preview(PREVIEW)
board.pipeline(True)
camera = csi.CSI()
frames = 0
start = time.ticks_ms()

try:
    camera.reset()
    camera.pixformat(csi.RGB565)
    camera.framesize(csi.QVGA)
    camera.snapshot(time=2000)
    start = last_report = time.ticks_ms()
    reported_frames = 0
    print("Camera test started: duration_ms=", DURATION_MS, "preview=", PREVIEW)
    while time.ticks_diff(time.ticks_ms(), start) < DURATION_MS:
        img = camera.snapshot()
        frames += 1
        now = time.ticks_ms()
        interval = time.ticks_diff(now, last_report)
        if interval >= REPORT_EVERY_MS:
            print("elapsed_ms=", time.ticks_diff(now, start), "frames=", frames,
                  "fps=", (frames - reported_frames) * 1000 / interval)
            print(board.camera_status())
            last_report = now
            reported_frames = frames
    elapsed = time.ticks_diff(time.ticks_ms(), start)
    camera.shutdown(True)
    print("PASS: frames=", frames, "elapsed_ms=", elapsed)
    print(board.camera_status())
except Exception as error:
    print("FAIL: frames=", frames, "elapsed_ms=", time.ticks_diff(time.ticks_ms(), start))
    print("error=", error)
    print(board.camera_status())
    raise
