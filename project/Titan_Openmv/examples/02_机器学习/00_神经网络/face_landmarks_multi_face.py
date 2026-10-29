# Titan Board RA8P1: 多脸 468 关键点
# Source: ThirdParty/openmv/scripts/examples/03-Machine-Learning/00-TensorFlow/face_landmarks_multi_face.py
# Prerequisites and exact copy paths: ../README.md.
# Copy models/sdcard/face_landmarks_192_u55_256.tflite -> /face_landmarks_192_u55_256.tflite
# Copy models/sdcard/blazeface_front_128_u55_256.tflite -> /blazeface_front_128_u55_256.tflite

# This work is licensed under the MIT license.
# Copyright (c) 2013-2025 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# This example shows off Google's MediaPipe Face Landmark Detection model for multiple faces.
#
# NOTE: This exaxmple requires an OpenMV Cam with an NPU like the AE3 or N6 to run real-time.

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

# Load the landmark arena first; both models are reused for all faces.
face_landmarks = ml.Model("/rom/face_landmarks_192_u55_256.tflite", postprocess=FaceLandmarks(threshold=0.4))
face_detection = ml.Model("/rom/blazeface_front_128_u55_256.tflite", postprocess=BlazeFace(threshold=0.4))
MAX_FACES = 2  # Bound per-frame latency; this does not create extra models.

clock = time.clock()
while True:
    clock.tick()
    img = csi0.snapshot()

    # faces is a list of ((x, y, w, h), score, keypoints) tuples
    for r, score, keypoints in face_detection.predict([img])[:MAX_FACES]:
        # rect is (x, y, w, h) - enlarge by 2x for face landmarks model
        wider_rect = (r[0] - r[2] // 2, r[1] - r[3] // 2, r[2] * 2, r[3] * 2)
        # Operate on just the ROI of the detected face
        n = Normalization(roi=wider_rect)

        # marks is a list of ((x, y, w, h), score, keypoints) tuples
        for r, score, keypoints in face_landmarks.predict([n(img)]):
            ml.utils.draw_predictions(img, [r], ("face",), ((0, 0, 255),), format=None)

            # keypoints is a ndarray of shape (468, 3) where each keypoint is (x, y, z)
            ml.utils.draw_keypoints(img, keypoints, radius=0, color=(255, 0, 0))

    print(clock.fps(), "fps")
