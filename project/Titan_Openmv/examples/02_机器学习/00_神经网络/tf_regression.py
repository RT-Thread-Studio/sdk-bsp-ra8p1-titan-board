# Titan Board RA8P1: TFLM 数值回归与 ulab 数组
# Source: ThirdParty/openmv/scripts/examples/03-Machine-Learning/00-TensorFlow/tf_regression.py
# Prerequisites and exact copy paths: ../README.md.
# Copy 02_机器学习/assets/force_int_quant.tflite -> /omv_assets/ml/force_int_quant.tflite

# This work is licensed under the MIT license.
# Copyright (c) 2013-2024 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# TensorFlow Lite Regression Example
#
# This example shows off running a regression model on the OpenMV Cam.
# A regression model takes an input list of numbers and produces an
# output list of numbers. You may pass ndarrays arrays to predict()
# and you will get a list of the results back.
#
# Note: The input list of numbers must be the same size as the input
# tensor size of the model.

import ml
from ulab import numpy as np

# The supplied official model has FLOAT32 input and output.
model = ml.Model("/omv_assets/ml/force_int_quant.tflite")
print(model)

i = np.array([-3, -1, -2, 5, -2, 10, -1, 9, 0, # noqa
               2,  0,  9, 1, 10,  2, -1, 3, 5, # noqa
               3,  9,  3, 9,  6,  2,  6, 7, 5, # noqa
               10, 6, -1, 7,  4,  7,  8, 5, 7], # noqa
               dtype=np.float).reshape(model.input_shape[0]) # noqa

print(model.predict([i])[0])
# Should print 53.78332
