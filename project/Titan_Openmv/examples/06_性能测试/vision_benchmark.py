# SPDX-License-Identifier: Apache-2.0
# Fixed-input CPU vision benchmark. No camera, NPU, drawing or USB frame timing.
import gc
import image
import json
import sys
import time

BOARD_LABEL = "RA8P1 Titan Board"  # Change when running on another board.
FIRMWARE_LABEL = "record firmware version and build hash here"
WIDTH = 320
HEIGHT = 240
WARMUP = 10
SAMPLES = 100
REPEATS = 3


def make_fixture():
    img = image.Image(WIDTH, HEIGHT, image.GRAYSCALE)
    # Fill every pixel; input does not depend on camera exposure or image codecs.
    for y in range(HEIGHT):
        for x in range(WIDTH):
            shade = 32 + (((x // 32) + (y // 32)) % 4) * 48
            img.set_pixel((x, y), shade)
    return img


def binary(img):
    return img.binary([(96, 192)])


def mean(img):
    return img.mean(1)


def blobs(img):
    return img.find_blobs([(160, 255)], x_stride=1, y_stride=1,
                          pixels_threshold=20, area_threshold=20, merge=False)


def canny(img):
    return img.find_edges(image.EDGE_CANNY, threshold=(50, 80))


def statistics(values):
    ordered = sorted(values)
    count = len(ordered)
    middle = count // 2
    median = (ordered[middle] if count % 2 else
              (ordered[middle - 1] + ordered[middle]) / 2)
    return {"mean": sum(values) / count, "p50": median,
            "p95": ordered[(95 * count + 99) // 100 - 1],
            "min": ordered[0], "max": ordered[-1]}


def benchmark(source, operation):
    elapsed = [0] * SAMPLES
    for index in range(WARMUP + SAMPLES):
        # Each function sees the same pixels, including functions that modify them.
        frame = source.copy()
        gc.collect()
        start = time.ticks_us()
        result = operation(frame)
        duration = time.ticks_diff(time.ticks_us(), start)
        if index >= WARMUP:
            elapsed[index - WARMUP] = duration
        result = None
        frame = None
    return statistics(elapsed)


def main():
    if SAMPLES < 1 or WARMUP < 0 or REPEATS < 1:
        raise ValueError("Invalid sample count")
    print(json.dumps({"benchmark": "vision_cpu_v1", "board": BOARD_LABEL,
                      "firmware": FIRMWARE_LABEL, "python": sys.version,
                      "fixture": "gray_blocks_v1", "width": WIDTH, "height": HEIGHT,
                      "warmup": WARMUP, "samples": SAMPLES, "repeats": REPEATS,
                      "unit": "us", "gc": "collect before each timed call"}))
    source = make_fixture()
    for repeat in range(1, REPEATS + 1):
        for name, operation in (("binary", binary), ("mean_3x3", mean),
                                ("find_blobs", blobs), ("canny", canny)):
            values = benchmark(source, operation)
            print(json.dumps({"repeat": repeat, "case": name, "time_us": values}))
    print("Benchmark complete. Save all JSON lines with the firmware hash.")


main()
