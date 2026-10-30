# Titan Board RA8P1: VWW 有人/无人分类
# Source: examples/public_classification.py
# Prerequisites and exact copy paths: ../README.md.
# Copy models/sdcard/vww96_u55_256.tflite -> /vww96_u55_256.tflite
# Copy models/sdcard/vww96_u55_256.txt -> /vww96_u55_256.txt

# SPDX-License-Identifier: MIT
# Copy the selected {id}_u55_256.tflite and matching .txt to the selected storage root.
# Renesas official INT8 models, locally compiled for this OpenMV U55 port.
import csi
import time
import ml
from ulab import numpy as np
from ml.preprocessing import Normalization

MODEL_ID = "vww96"
SPECS = {
    "vww96": ((1, 96, 96, 3), 0.003921568859368563, -128, 2),
    "mobilenetv1_025": ((1, 224, 224, 3), 0.007843137718737125, -1, 1000),
    "resnet8": ((1, 32, 32, 3), 1.0, -128, 10),
}
# Exact bit pattern for official MobileNet float32 preprocessing and
# round(real / input_scale) + zero_point. Do not move zero_point inside round.
_MOBILENET_LUT = (
    128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
    144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
    159, 161, 161, 163, 163, 165, 165, 167, 167, 169, 169, 171, 171, 173, 173, 175,
    175, 177, 177, 179, 179, 181, 181, 183, 183, 185, 185, 187, 187, 189, 189, 191,
    192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
    208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
    224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
    240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
    32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
    47, 49, 49, 51, 51, 53, 53, 55, 55, 57, 57, 59, 59, 61, 61, 63,
    63, 65, 65, 67, 67, 69, 69, 71, 71, 73, 73, 75, 75, 77, 77, 79,
    79, 81, 81, 83, 83, 85, 85, 87, 87, 89, 89, 91, 91, 93, 93, 95,
    95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110,
    111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126,
)

class MobileNetInput:
    def __init__(self):
        self.lookup = np.array(_MOBILENET_LUT, dtype=np.uint8)
        self.normalization = None
        self.roi = None

    def set_image(self, img):
        self.normalization = Normalization()(img)
        self.roi = self.normalization.roi

    def __call__(self, buffer, shape, dtype):
        if dtype != ord("b"):
            raise ValueError("This lookup is for the supplied INT8 MobileNet")
        # First obtain unquantized RGB bytes with the standard resize path.
        self.normalization(buffer, shape, ord("B"))
        raw = np.frombuffer(buffer, dtype=np.uint8)
        # This ulab implementation collects indices before writing output,
        # so in-place lookup is valid. It allocates a temporary index vector.
        np.take(self.lookup, raw, out=raw)

spec_shape, spec_scale, spec_zero, class_count = SPECS[MODEL_ID]
model = ml.Model("/rom/" + MODEL_ID + "_u55_256.tflite", postprocess=None)
if (tuple(model.input_shape[0]) != spec_shape or model.input_dtype[0] != "b"
        or abs(model.input_scale[0] - spec_scale) > 1e-8
        or model.input_zero_point[0] != spec_zero):
    raise ValueError("Model does not match the selected input contract")
if not model.labels or len(model.labels) != class_count:
    raise ValueError("Copy the model's matching .txt labels to the selected storage root")
if len(model.output_shape) != 1 or tuple(model.output_shape[0]) != (1, class_count):
    raise ValueError("Unexpected classification output")
print(model)
camera = csi.CSI()
camera.reset()
camera.pixformat(csi.RGB565)
camera.framesize(csi.QVGA)
camera.snapshot(time=2000)
mobile_input = MobileNetInput() if MODEL_ID == "mobilenetv1_025" else None
clock = time.clock()
last_print = time.ticks_ms()
while True:
    clock.tick()
    img = camera.snapshot()
    if mobile_input:
        mobile_input.set_image(img)
        args = [mobile_input]
    else:
        args = [img]  # These supplied input contracts map RGB byte p to p - 128.
    outputs = model.predict(args)
    scores = outputs[0].flatten()  # The backend returns dequantized float scores.
    best = int(np.argmax(scores))
    img.draw_string((0, 0), "%s %.3f" % (model.labels[best], scores[best]), color=(0, 255, 0))
    # All three models already include Softmax.
    if time.ticks_diff(time.ticks_ms(), last_print) >= 1000:
        print(MODEL_ID, clock.fps(), "fps")
        last_print = time.ticks_ms()
