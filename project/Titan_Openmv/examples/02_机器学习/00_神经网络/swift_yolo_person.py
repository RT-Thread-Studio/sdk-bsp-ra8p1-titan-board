# This work is licensed under the MIT license.
# Seeed Swift-YOLO person, compiled for Titan Board Ethos-U55-256.
# Copy swift_yolo_person_nano_192_u55_256.tflite and its .txt to the storage root.
# /rom is a read-only alias of the storage root; do not create a rom directory.
# Model source and MIT notice: see models/README.md and models/licenses/.

import csi
import time
import ml
from ml.utils import NMS
from ulab import numpy as np

CONFIDENCE_THRESHOLD = 0.4
MODEL_PATH = "/rom/swift_yolo_person_nano_192_u55_256.tflite"


def postprocess(model, inputs, outputs):
    # Seeed's exported Swift-YOLO graph already decodes anchors and sigmoid.
    # xywh are MODEL PIXELS and column 4 is a PERCENT score (not 0..1).
    # The SSCMA decoder uses that score directly and argmax of class columns.
    scale = model.output_scale[0]
    zero = model.output_zero_point[0]
    rows = outputs[0][0]
    indices = np.nonzero(rows[:, 4] > CONFIDENCE_THRESHOLD * 100 / scale + zero)[0]
    if not len(indices):
        return ()
    boxes = (np.take(rows, indices, axis=0) - float(zero)) * scale
    classes = np.argmax(boxes[:, 5:], axis=1)
    nms = NMS(192, 192, inputs[0].roi)
    for i in range(boxes.shape[0]):
        cx, cy, width, height = boxes[i, 0], boxes[i, 1], boxes[i, 2], boxes[i, 3]
        score = min(1.0, boxes[i, 4] / 100.0)
        nms.add_bounding_box(cx - width / 2, cy - height / 2,
                             cx + width / 2, cy + height / 2, score, int(classes[i]))
    # Reuse OpenMV's native example library for suppression and ROI mapping.
    return nms.get_bounding_boxes(threshold=CONFIDENCE_THRESHOLD, sigma=0.1)


model = ml.Model(MODEL_PATH, postprocess=postprocess)
if (tuple(model.input_shape[0]) != (1, 192, 192, 3)
        or model.input_dtype[0] != "b"
        or abs(model.input_scale[0] - 1.0 / 255.0) > 1e-8
        or model.input_zero_point[0] != -128
        or len(model.output_shape) != 1
        or tuple(model.output_shape[0]) != (1, 2268, 6)
        or model.output_dtype[0] != "b"
        or abs(model.output_scale[0] - 2.0924370288848877) > 1e-6
        or model.output_zero_point[0] != -124):
    raise ValueError("Use the supplied Swift-YOLO person U55-256 model")
if tuple(model.labels or ()) != ('person',):
    raise ValueError("Copy the matching Swift-YOLO label .txt beside the model")
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
    # ml.Model resizes the entire RGB image to the 192x192 input tensor.
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
        print("Swift-YOLO person", count, "objects", clock.fps(), "fps")
        last_print = time.ticks_ms()
