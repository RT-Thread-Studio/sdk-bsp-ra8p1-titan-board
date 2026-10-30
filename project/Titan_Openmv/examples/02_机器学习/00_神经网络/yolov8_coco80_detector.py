# This work is licensed under the MIT license.
# Copyright (c) 2013-2025 OpenMV LLC. All rights reserved.
# Source: ThirdParty/openmv/scripts/examples/03-Machine-Learning/00-TensorFlow/yolo_v8_detector.py
# Titan Board: COCO80 object detection, including person, bottle, cup, cell phone.
# Copy yolov8n_coco80_192_u55_256.tflite and its .txt to the storage root.
# /rom is the root's read-only alias. Model weights have separate license notices.

import csi
import time
import ml
from ml.postprocessing.ultralytics import YoloV8

CONFIDENCE_THRESHOLD = 0.4
MODEL_PATH = "/rom/yolov8n_coco80_192_u55_256.tflite"

model = ml.Model(MODEL_PATH, postprocess=YoloV8(threshold=CONFIDENCE_THRESHOLD,
                                             nms_threshold=CONFIDENCE_THRESHOLD))
if (tuple(model.input_shape[0]) != (1, 192, 192, 3)
        or model.input_dtype[0] != "b"
        or abs(model.input_scale[0] - 1.0 / 255.0) > 1e-8
        or model.input_zero_point[0] != -128
        or len(model.output_shape) != 1
        or tuple(model.output_shape[0]) != (1, 84, 756)):
    raise ValueError("Use the supplied COCO80 U55-256 detector, not an ImageNet or person-only model")
if not model.labels or len(model.labels) != 80:
    raise ValueError("Copy the matching 80-line COCO label .txt beside the detector model")
print(model)

camera = csi.CSI()
camera.reset()
camera.pixformat(csi.RGB565)
camera.framesize(csi.QVGA)
camera.snapshot(time=2000)
clock = time.clock()
last_print = time.ticks_ms()

while True:
    clock.tick()
    img = camera.snapshot()
    # Official decoder returns per-class ((x, y, w, h), score) detections,
    # mapped from normalized model coordinates back to this captured image.
    detections = model.predict([img])
    count = 0
    for class_id, objects in enumerate(detections):
        for rect, score in objects:
            img.draw_rectangle(rect, color=(0, 255, 0), thickness=2)
            label = "%s %.2f" % (model.labels[class_id], score)
            x = max(0, min(rect[0], img.width() - len(label) * 8))
            y = max(0, rect[1] - 10)
            img.draw_string((x, y), label, color=(0, 255, 0))
            count += 1
    if time.ticks_diff(time.ticks_ms(), last_print) >= 1000:
        print("COCO80", count, "objects", clock.fps(), "fps")
        last_print = time.ticks_ms()
