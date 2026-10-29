# SPDX-License-Identifier: Apache-2.0
# Copyright 2022, 2025 Arm Limited and/or its affiliates.
# Licensed under the Apache License, Version 2.0; you may not use this file
# except in compliance with the License. Obtain a copy at
# https://www.apache.org/licenses/LICENSE-2.0
# Distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND.
# YOLO-Fastest Face v4: Emza model used by Arm CMSIS-MLEK.
# Decoder follows Arm's DetectorPostProcessing (Copyright 2022, 2025 Arm Ltd).
# Copy yolo_fastest_face_192_u55_256.tflite and its .txt to the storage root.
# /rom is the storage root's read-only alias; no rom directory is needed.
# These weights differ from the separately trained RT-Thread face example.

import csi
import math
import ml
import time

MODEL_PATH = "/rom/yolo_fastest_face_192_u55_256.tflite"
CONFIDENCE_THRESHOLD = 0.5
NMS_THRESHOLD = 0.45
MAX_CANDIDATES = 60
MAX_FACES = 16


class YoloFastestFace:
    # Export order: 6x6 head (large faces), then 12x12 head (small faces).
    ANCHORS = (((38, 77), (47, 97), (61, 126)),
               ((14, 26), (19, 37), (28, 55)))
    SCALES = (0.13408391177654266, 0.18535925447940826)
    ZEROS = (47, 10)

    def __init__(self, threshold=CONFIDENCE_THRESHOLD, nms_threshold=NMS_THRESHOLD,
                 max_candidates=MAX_CANDIDATES, max_faces=MAX_FACES):
        if not 0 < threshold < 1 or not 0 <= nms_threshold <= 1:
            raise ValueError("Threshold must be in (0, 1), NMS in [0, 1]")
        if max_faces < 1 or max_candidates < max_faces:
            raise ValueError("Candidate limit must be >= face limit > 0")
        self.threshold = threshold
        self.nms_threshold = nms_threshold
        self.max_candidates = max_candidates
        self.max_faces = max_faces
        logit = math.log(threshold / (1 - threshold))
        self.raw_thresholds = tuple(math.floor(logit / s + z)
                                    for s, z in zip(self.SCALES, self.ZEROS))

    def validate(self, model):
        if (len(model.input_shape) != 1
                or tuple(model.input_shape[0]) != (1, 192, 192, 1)
                or model.input_dtype[0] != "b"
                or abs(model.input_scale[0] - 0.00392117677256465) > 1e-9
                or model.input_zero_point[0] != -128
                or len(model.output_shape) != 2):
            raise ValueError("Use the supplied Emza YOLO-Fastest Face v4 U55-256 model")
        for i, grid in enumerate((6, 12)):
            if (tuple(model.output_shape[i]) != (1, grid, grid, 18)
                    or model.output_dtype[i] != "b"
                    or abs(model.output_scale[i] - self.SCALES[i]) > 1e-8
                    or model.output_zero_point[i] != self.ZEROS[i]):
                raise ValueError("YOLO-Fastest Face output/anchor contract mismatch")

    @staticmethod
    def sigmoid(value):
        return 1.0 / (1.0 + math.exp(-value))

    @staticmethod
    def iou(a, b):
        overlap = (max(0, min(a[2], b[2]) - max(a[0], b[0]))
                   * max(0, min(a[3], b[3]) - max(a[1], b[1])))
        union = ((a[2] - a[0]) * (a[3] - a[1])
                 + (b[2] - b[0]) * (b[3] - b[1]) - overlap)
        return overlap / union if union > 0 else 0

    def __call__(self, model, inputs, outputs):
        candidates = []
        for head, grid in enumerate((6, 12)):
            rows = outputs[head].reshape((grid * grid * 3, 6))
            scale, zero = self.SCALES[head], self.ZEROS[head]
            for index in range(grid * grid * 3):
                raw_objectness = int(rows[index, 4])
                if raw_objectness <= self.raw_thresholds[head]:
                    continue
                objectness = self.sigmoid((raw_objectness - zero) * scale)
                # This model outputs class logits as well as objectness logits.
                score = objectness * self.sigmoid((int(rows[index, 5]) - zero) * scale)
                if score <= self.threshold:
                    continue
                candidates.append((score, head, index))
                if len(candidates) > self.max_candidates:
                    candidates.sort(reverse=True)
                    candidates.pop()
        candidates.sort(reverse=True)
        selected = []
        for score, head, index in candidates:
            grid = 6 if head == 0 else 12
            row = outputs[head].reshape((grid * grid * 3, 6))[index]
            scale, zero = self.SCALES[head], self.ZEROS[head]
            cell, anchor = index // 3, index % 3
            cx = (cell % grid + self.sigmoid((int(row[0]) - zero) * scale)) / grid
            cy = (cell // grid + self.sigmoid((int(row[1]) - zero) * scale)) / grid
            aw, ah = self.ANCHORS[head][anchor]
            width = aw * math.exp((int(row[2]) - zero) * scale) / 192
            height = ah * math.exp((int(row[3]) - zero) * scale) / 192
            box = (cx - width / 2, cy - height / 2, cx + width / 2, cy + height / 2)
            if any(self.iou(box, old_box) > self.nms_threshold for old_box, _ in selected):
                continue
            selected.append((box, score))
            if len(selected) >= self.max_faces:
                break
        rx, ry, rw, rh = inputs[0].roi
        result = []
        for box, score in selected:
            x1, y1 = max(0, min(1, box[0])), max(0, min(1, box[1]))
            x2, y2 = max(0, min(1, box[2])), max(0, min(1, box[3]))
            x, y = rx + int(x1 * rw), ry + int(y1 * rh)
            width, height = int((x2 - x1) * rw), int((y2 - y1) * rh)
            if width > 0 and height > 0:
                result.append(((x, y, width, height), score))
        return result


def main():
    decoder = YoloFastestFace()
    model = ml.Model(MODEL_PATH, postprocess=decoder)
    decoder.validate(model)
    print(model)
    camera = csi.CSI()
    camera.reset()
    # The trained model is grayscale. OpenMV converts bytes to INT8 by -128,
    # matching Arm's reference preprocessing; no [-1, 1] normalization.
    camera.pixformat(csi.GRAYSCALE)
    camera.framesize(csi.QVGA)
    camera.snapshot(time=2000)
    clock = time.clock()
    last_print = time.ticks_ms()
    while True:
        clock.tick()
        img = camera.snapshot()
        faces = model.predict([img])
        for rect, score in faces:
            img.draw_rectangle(rect, color=255, thickness=2)
            img.draw_string((rect[0], max(0, rect[1] - 10)), "face %.2f" % score, color=255)
        if time.ticks_diff(time.ticks_ms(), last_print) >= 1000:
            print("YOLO-Fastest", len(faces), "faces", clock.fps(), "fps")
            last_print = time.ticks_ms()


if __name__ == "__main__":
    main()
