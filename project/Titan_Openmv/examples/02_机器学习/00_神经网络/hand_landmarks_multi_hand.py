# Titan Board RA8P1: 多手 21 关键点
# Source: ThirdParty/openmv/scripts/examples/03-Machine-Learning/00-TensorFlow/hand_landmarks_multi_hand.py
# Prerequisites and exact copy paths: ../README.md.
# Copy models/sdcard/palm_detection_full_192_u55_256.tflite -> /palm_detection_full_192_u55_256.tflite
# Copy models/sdcard/hand_landmarks_full_224_u55_256.tflite -> /hand_landmarks_full_224_u55_256.tflite
# /rom is a read-only alias of the storage root; no rom directory is needed.

# This work is licensed under the MIT license.
# Copyright (c) 2013-2025 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# This example shows off Google's MediaPipe Hand Landmarks Detection model for multiple hands.
#
# NOTE: This exaxmple requires an OpenMV Cam with an NPU like the AE3 or N6 to run real-time.

import csi
import time
import ml
from ml.preprocessing import Normalization
from ml.postprocessing.mediapipe import BlazePalm
from ml.postprocessing.mediapipe import HandLandmarks

# Initialize the sensor.
csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.RGB565)
csi0.framesize(csi.QVGA)

# BlazePalm requires a square image for the best results.
# HandLandmarks works with non-square images from BlazePalm crops.
csi0.window((240, 240))

# Load the palm detection model from the storage root.
palm_detection = ml.Model("/rom/palm_detection_full_192_u55_256.tflite", postprocess=BlazePalm(threshold=0.4))
print(palm_detection)

# Load the hand landmark model from the storage root.
hand_landmarks = ml.Model("/rom/hand_landmarks_full_224_u55_256.tflite", postprocess=HandLandmarks(threshold=0.4))
print(hand_landmarks)

# Line connections between hand joints for drawing the hand skeleton.
hand_lines = ((0, 1), (1, 2), (2, 3), (3, 4), (0, 5), (5, 6), (6, 7), (7, 8),
              (5, 9), (9, 10), (10, 11), (11, 12), (9, 13), (13, 14), (14, 15), (15, 16),
              (13, 17), (17, 18), (18, 19), (19, 20), (0, 17))

MAX_HANDS = 2  # Reuse one model pair for both hands.

clock = time.clock()
while True:
    clock.tick()
    img = csi0.snapshot()

    # palms is a list of ((x, y, w, h), score, keypoints) tuples
    for r, score, keypoints in palm_detection.predict([img])[:MAX_HANDS]:
        # rect is (x, y, w, h) - enlarge by 3x for hand landmarks model
        wider_rect = (r[0] - r[2], r[1] - r[3], r[2] * 3, r[3] * 3)
        # Operate on just the ROI of the detected palm
        n = Normalization(roi=wider_rect)

        # hands is a list of ((x, y, w, h), score, keypoints) tuples
        # index 0 (if present) is left hand
        # index 1 (if present) is right hand
        hands = hand_landmarks.predict([n(img)])

        # Draw bounding boxes around the detected hands and keypoints.
        for i, detections in enumerate(hands):
            for r, score, keypoints in detections:
                ml.utils.draw_predictions(img, [r], ("right",) if i else ("left",), ((0, 0, 255),), format=None)

                # keypoints: ndarray (21, 3) of hand joints (x, y, z)
                # Indices follow MediaPipe convention:
                # 0: wrist
                # Thumb: 1 cmc, 2 mcp, 3 ip, 4 tip
                # Index: 5 mcp, 6 pip, 7 dip, 8 tip
                # Middle: 9 mcp, 10 pip, 11 dip, 12 tip
                # Ring: 13 mcp, 14 pip, 15 dip, 16 tip
                # Pinky: 17 mcp, 18 pip, 19 dip, 20 tip
                # (cmc=base, mcp=knuckle, pip=mid, dip=distal, ip=thumb joint, tip=fingertip)
                ml.utils.draw_skeleton(img, keypoints, hand_lines, kp_color=(255, 0, 0), line_color=(0, 255, 0))

    print(clock.fps(), "fps")
