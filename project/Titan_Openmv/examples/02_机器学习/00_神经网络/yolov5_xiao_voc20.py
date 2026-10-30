# This example is licensed under the MIT license.
# Seeed YOLOv5n6-xiao weights and exporter are separately licensed GPL-3.0.
# Copy yolov5n6_xiao_voc20_192_u55_256.tflite and its .txt to the storage root.
# /rom is the root's read-only alias. The actual release weights have 20 VOC
# classes, including person, bottle, cat and dog; they do not include cup/phone.

import csi
import time
import ml
from ml.utils import NMS
from ulab import numpy as np

MODEL_PATH = "/rom/yolov5n6_xiao_voc20_192_u55_256.tflite"
CONFIDENCE_THRESHOLD = 0.3
MAX_CANDIDATES = 60


class XiaoDetector:
    def __init__(self, threshold=CONFIDENCE_THRESHOLD):
        self.threshold = threshold

    def __call__(self, model, inputs, outputs):
        rows = outputs[0].reshape((model.output_shape[0][1], 25))
        scale = model.output_scale[0]
        zero = float(model.output_zero_point[0])
        # YOLOv5 uses objectness * class probability, not objectness alone.
        # Reduce in ulab, then copy/dequantize only a bounded set of candidates.
        class_max = np.max(rows[:, 5:], axis=1)
        scores = ((rows[:, 4] - zero) * scale) * ((class_max - zero) * scale)
        indices = np.nonzero(scores > self.threshold)[0]
        candidates = []
        for index in indices:
            index = int(index)
            score = min(1.0, float(scores[index]))
            if len(candidates) < MAX_CANDIDATES:
                candidates.append((score, index))
                candidates.sort()
            elif score > candidates[0][0]:
                candidates[0] = (score, index)
                candidates.sort()
        if not candidates:
            return ()
        indices = [item[1] for item in candidates]
        selected = np.take(rows, indices, axis=0)
        classes = np.argmax(selected[:, 5:], axis=1)
        per_class = {}
        for i, (score, _) in enumerate(candidates):
            row = selected[i]
            cx = (float(row[0]) - zero) * scale * 192
            cy = (float(row[1]) - zero) * scale * 192
            width = (float(row[2]) - zero) * scale * 192
            height = (float(row[3]) - zero) * scale * 192
            if width > 0 and height > 0:
                class_id = int(classes[i])
                if class_id not in per_class:
                    per_class[class_id] = NMS(192, 192, inputs[0].roi)
                nms = per_class[class_id]
                nms.add_bounding_box(cx - width / 2, cy - height / 2,
                                     cx + width / 2, cy + height / 2,
                                     score, class_id)
        detections = [[] for _ in range(20)]
        for class_id, nms in per_class.items():
            kept = nms.get_bounding_boxes(threshold=self.threshold, sigma=0.1)
            if len(kept) > class_id:
                detections[class_id] = kept[class_id]
        return detections


model = ml.Model(MODEL_PATH, postprocess=XiaoDetector())
if (len(model.input_shape) != 1 or tuple(model.input_shape[0]) != (1, 192, 192, 3)
        or model.input_dtype[0] != "b"
        or abs(model.input_scale[0] - 1.0 / 255.0) > 1e-8
        or model.input_zero_point[0] != -128
        or len(model.output_shape) != 1 or tuple(model.output_shape[0]) != (1, 2295, 25)
        or model.output_dtype[0] != "b"
        or abs(model.output_scale[0] - 0.005157493986189365) > 1e-8
        or model.output_zero_point[0] != -126):
    raise ValueError("Use the supplied YOLOv5n6-xiao VOC20 192 U55-256 model")
if not model.labels or len(model.labels) != 20:
    raise ValueError("Copy the matching 20-line label .txt beside the model")
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
    detections = model.predict([img])
    count = 0
    for class_id, objects in enumerate(detections):
        for rect, score in objects:
            img.draw_rectangle(rect, color=(0, 255, 0), thickness=2)
            label = "%s %.2f" % (model.labels[class_id], score)
            x = max(0, min(rect[0], img.width() - len(label) * 8))
            img.draw_string((x, max(0, rect[1] - 10)), label, color=(0, 255, 0))
            count += 1
    if time.ticks_diff(time.ticks_ms(), last_print) >= 1000:
        print("YOLOv5 Xiao", count, "objects", clock.fps(), "fps")
        last_print = time.ticks_ms()
