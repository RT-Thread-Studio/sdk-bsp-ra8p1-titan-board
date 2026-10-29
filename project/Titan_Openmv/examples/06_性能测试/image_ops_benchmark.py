# SPDX-License-Identifier: Apache-2.0
# Titan Board / OpenMV 5 image-operation benchmark, using one captured RGB frame.
# The OpenMV public benchmark's exact input and morph kernel are unpublished.
# Results from this script therefore use a separate, explicit test definition.
import binascii
import board
import csi
import gc
import image
import json
import os
import sys
import time
import uctypes

WARMUP = 10
SAMPLES = 100
REPEATS = 3
SETTLE_MS = 2000
FRAMEBUFFERS = 5
EXPORT_FIXTURES = True
MEMORY_MODES = ("framebuffer", "gc")


def emit(record):
    print("@BENCH " + json.dumps(record))
    time.sleep_ms(2)


def image_info(img):
    pixfmt = img.format()
    name = ("GRAYSCALE" if pixfmt == image.GRAYSCALE else
            "RGB565" if pixfmt == image.RGB565 else
            "JPEG" if pixfmt == image.JPEG else "OTHER")
    return {"pixfmt": pixfmt, "pixfmt_name": name,
            "width": img.width(), "height": img.height(), "size": img.size(),
            "buffer_address": hex(uctypes.addressof(img))}


def export_fixture(name, img):
    # Image implements MP_BUFFER_READ; bytes(img) copies its complete raw buffer.
    # No file is written, and the image remains alive throughout transmission.
    data = bytes(img)
    if len(data) != img.size():
        raise ValueError("Fixture buffer length differs from image.size()")
    for offset in range(0, len(data), 128):
        encoded = binascii.hexlify(data[offset:offset + 128]).decode()
        print("@FIXTURE %s %d %s" % (name, offset, encoded))
        time.sleep_ms(2)
    print("@FIXTURE_END %s %d" % (name, len(data)))
    time.sleep_ms(2)


def statistics(values):
    ordered = sorted(values)
    count = len(ordered)
    if not count:
        return None
    middle = count // 2
    median = (ordered[middle] if count % 2 else
              (ordered[middle - 1] + ordered[middle]) / 2)
    return {"mean": sum(values) / count, "p50": median,
            "p95": ordered[(95 * count + 99) // 100 - 1],
            "min": ordered[0], "max": ordered[-1]}


def run_case(case_name, repeat, source, operation, parameters, memory_mode):
    durations = [0] * SAMPLES
    measured = 0
    warmup_completed = 0
    output_info = None
    frame = None
    output = None
    failure = None
    addresses = []
    try:
        for index in range(WARMUP + SAMPLES):
            # Restore pixels and collect old objects before starting the timer.
            if memory_mode == "framebuffer":
                frame = image.Image(source.width(), source.height(), source.format(), copy_to_fb=True)
                frame.replace(source)
            else:
                frame = source.copy()
            gc.collect()
            address = uctypes.addressof(frame)
            if memory_mode == "framebuffer":
                if not 0x70000000 <= address < 0x72000000:
                    raise RuntimeError("Working framebuffer is not in HyperRAM")
            elif not 0x22000000 <= address < 0x22200000:
                raise RuntimeError("GC working image is not in SRAM")
            if address not in addresses:
                addresses.append(address)
            start = time.ticks_us()
            output = operation(frame)
            duration = time.ticks_diff(time.ticks_us(), start)
            if duration < 0:
                raise ValueError("Timed call exceeded the ticks_diff range")
            if index < WARMUP:
                warmup_completed += 1
            else:
                durations[measured] = duration
                measured += 1
            # Result inspection, reference release and printing are not timed.
            output_info = image_info(output)
            output = None
            frame = None
    except BaseException as error:
        failure = error
    finally:
        frame = None
        output = None

    values = durations[:measured]
    record = {"type": "case", "case": case_name, "repeat": repeat,
              "memory_mode": memory_mode,
              "working_addresses": [hex(value) for value in addresses],
              "status": "pass" if failure is None else "error",
              "input": image_info(source), "parameters": parameters,
              "warmup_completed": warmup_completed, "samples": measured,
              "samples_us": values, "statistics_us": statistics(values),
              "output": output_info}
    if failure is not None:
        record["error"] = repr(failure)
    emit(record)
    if failure is not None:
        raise failure


def main():
    if WARMUP < 0 or SAMPLES < 1 or REPEATS < 1 or FRAMEBUFFERS < 5:
        raise ValueError("Invalid benchmark configuration")
    previous_preview = board.preview(False)
    previous_pipeline = None
    try:
        previous_pipeline = board.pipeline(False)
        camera = csi.CSI()
        camera.reset()
        camera.pixformat(csi.RGB565)
        camera.framesize(csi.QVGA)
        camera.framebuffers(FRAMEBUFFERS)
        camera.snapshot(time=SETTLE_MS)
        # With pipeline=False, ra8_csi.c stops/drains capture before returning.
        rgb = camera.snapshot().copy()
        actual_framebuffers = camera.framebuffers()
        if board.pipeline() or board.preview():
            raise RuntimeError("Preview and capture pipeline must remain disabled")
        if actual_framebuffers < 5:
            raise RuntimeError("RA8P1 capture requires at least five framebuffers")
        if (rgb.width(), rgb.height(), rgb.format()) != (320, 240, image.RGB565):
            raise RuntimeError("Unexpected captured image format")

        # Both gray fixtures derive from this same captured RGB565 image.
        gray = rgb.to_grayscale(copy=True)
        reference = gray.copy()
        reference.invert()
        kernel = (0, -1, 0, -1, 5, -1, 0, -1, 0)
        uname_function = getattr(os, "uname", None)
        uname = list(uname_function()) if uname_function is not None else None
        metadata = {
            "type": "metadata", "benchmark": "titan_image_ops_v2",
            "board_info": list(board.info()), "sys_version": sys.version,
            "sys_implementation": repr(sys.implementation), "os_uname": uname,
            "preview_enabled": board.preview(), "pipeline_enabled": board.pipeline(),
            "capture_dma": "stopped by non-pipelined snapshot before return",
            "framebuffers_requested": FRAMEBUFFERS,
            "framebuffers_actual": actual_framebuffers,
            "settle_ms": SETTLE_MS, "warmup": WARMUP,
            "samples": SAMPLES, "repeats": REPEATS, "case_count": 9,
            "memory_modes": MEMORY_MODES,
            "working_memory": "framebuffer=HyperRAM; gc=SRAM, checked for every call",
            "unit": "us", "copy_and_explicit_gc": "outside timing",
            "timing": "Python wrapper and image call; includes call allocations",
            "fixtures": {"rgb565": image_info(rgb), "gray": image_info(gray),
                         "gray_inverted": image_info(reference)},
            "fixture_derivation": "RGB snapshot; gray=RGB.to_grayscale(copy=True); reference=gray.copy().invert()",
            "rgb565_byte_order": "little-endian", "export_fixtures": EXPORT_FIXTURES,
            "morph_kernel": kernel, "morph_kernel_sum": sum(kernel),
            "official_conditions_equal": False,
            "comparison_note": "Static captured input and five or more framebuffers; public morph kernel and fixture are unavailable",
        }
        emit(metadata)
        if EXPORT_FIXTURES:
            export_fixture("rgb565", rgb)
            export_fixture("gray", gray)
            export_fixture("gray_inverted", reference)
        gc.collect()

        cases = (
            ("gray_invert", gray, lambda img: img.invert(), {}),
            ("gray_difference", gray, lambda img: img.difference(reference),
             {"reference": "gray_inverted"}),
            ("gray_erode_3x3", gray, lambda img: img.erode(1, threshold=0),
             {"ksize": 1, "window": [3, 3], "threshold": 0}),
            ("gray_dilate_3x3", gray, lambda img: img.dilate(1, threshold=0),
             {"ksize": 1, "window": [3, 3], "threshold": 0}),
            ("gray_mean_3x3", gray, lambda img: img.mean(1),
             {"ksize": 1, "window": [3, 3], "threshold": False}),
            ("gray_morph_sharpen_3x3", gray,
             lambda img: img.morph(1, kernel, mul=1.0, add=0.0),
             {"ksize": 1, "kernel": kernel, "kernel_sum": 1,
              "mul": 1.0, "add": 0.0, "threshold": False}),
            ("gray_jpeg_q90", gray,
             lambda img: img.compress(quality=90, copy=False,
                                      subsampling=image.JPEG_SUBSAMPLING_AUTO),
             {"quality": 90, "copy": False, "subsampling": "AUTO"}),
            ("rgb565_jpeg_q90", rgb,
             lambda img: img.compress(quality=90, copy=False,
                                      subsampling=image.JPEG_SUBSAMPLING_AUTO),
             {"quality": 90, "copy": False, "subsampling": "AUTO"}),
            ("rgb565_to_gray", rgb, lambda img: img.to_grayscale(copy=False),
             {"copy": False, "rgb_channel": -1}),
        )
        for memory_mode in MEMORY_MODES:
            for repeat in range(1, REPEATS + 1):
                for case_name, source, operation, parameters in cases:
                    run_case(case_name, repeat, source, operation, parameters, memory_mode)
        emit({"type": "complete", "case_records": len(cases) * REPEATS * len(MEMORY_MODES),
              "measured_calls": len(cases) * REPEATS * SAMPLES * len(MEMORY_MODES)})
    finally:
        # Restore both settings, including when capture or an operation fails.
        try:
            if previous_pipeline is not None:
                board.pipeline(previous_pipeline)
        finally:
            board.preview(previous_preview)


main()
