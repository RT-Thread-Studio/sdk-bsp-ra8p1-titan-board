# Titan Board RA8P1: 单脸 468 关键点跟踪
# Source: examples/face_landmarks.py
# Prerequisites and exact copy paths: ../README.md.
# Copy models/sdcard/face_landmarks_192_u55_256.tflite -> /face_landmarks_192_u55_256.tflite
# Copy models/sdcard/blazeface_front_128_u55_256.tflite -> /blazeface_front_128_u55_256.tflite

# This work is licensed under the MIT license.
# Copyright (c) 2013-2025 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# This example shows off Google's MediaPipe Face Landmark Detection model for a single face.
#
# Load the landmark model first to give its arena priority in SRAM.

import csi
import time
import ml
from ml.preprocessing import Normalization
from ml.postprocessing.mediapipe import BlazeFace
from ml.postprocessing.mediapipe import FaceLandmarks

# Initialize the sensor.
csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.RGB565)
csi0.framesize(csi.VGA)

# BlazeFace requires a square image for the best results.
# FaceLandmarks works with non-square images from BlazeFace crops.
csi0.window((400, 400))

# Load landmarks before the search model to preserve arena allocation order.
# repr(model).ram_addr in 0x220... confirms SRAM; 0x70.../0x71... is HyperRAM.

# Load the quantized U55 landmark model from the selected storage filesystem.
face_landmarks = ml.Model("/rom/face_landmarks_192_u55_256.tflite", postprocess=FaceLandmarks(threshold=0.4))
print(face_landmarks)

# Load the companion U55 search model after the landmark arena is allocated.
face_detection = ml.Model("/rom/blazeface_front_128_u55_256.tflite", postprocess=BlazeFace(threshold=0.4))
print(face_detection)

print("SEARCH: BlazeFace U55 (12 NPU calls); TRACK: landmarks U55 (4 NPU calls)")
# Tracking vars.
n = None

clock = time.clock()
while True:
    clock.tick()
    img = csi0.snapshot()

    if n is None:
        # faces is a list of ((x, y, w, h), score, keypoints) tuples
        for r, score, keypoints in face_detection.predict([img]):
            # rect is (x, y, w, h) - enlarge by 2x for face landmarks model
            wider_rect = (r[0] - r[2] // 2, r[1] - r[3] // 2, r[2] * 2, r[3] * 2)
            # Operate on just the ROI of the detected face
            n = Normalization(roi=wider_rect)

    else:
        # marks is a list of ((x, y, w, h), score, keypoints) tuples
        marks = face_landmarks.predict([n(img)])

        # No faces detected, reset the tracker.
        if not marks:
            n = None
            continue

        # Draw bounding boxes around the detected faces and keypoints.
        for r, score, keypoints in marks:
            ml.utils.draw_predictions(img, [r], ("face",), ((0, 0, 255),), format=None)

            # keypoints is a ndarray of shape (468, 3) where each keypoint is (x, y, z)
            ml.utils.draw_keypoints(img, keypoints, radius=0, color=(255, 0, 0))

            # Center new_wider_rect on face for tracking
            new_wider_rect = (r[0] + (r[2] // 2) - (wider_rect[2] // 2),
                              r[1] + (r[3] // 2) - (wider_rect[3] // 2),
                              wider_rect[2],
                              wider_rect[3])
            # Operate on just the ROI of the detected face
            n = Normalization(roi=new_wider_rect)

    print(clock.fps(), "fps")
