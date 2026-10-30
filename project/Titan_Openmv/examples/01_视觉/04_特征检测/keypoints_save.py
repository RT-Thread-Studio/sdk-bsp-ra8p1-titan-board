# Titan Board OpenMV example: 关键点描述符保存与加载
# Source: ThirdParty/openmv/scripts/examples/05-Feature-Detection/keypoints_save.py
# Adaptation: 锁定稳定后的增益，去除上游示例对 OV5640 会严重饱和的 100 dB 强制增益。
# Adaptation: 保存后重新加载描述符，示范完整文件读写。
# Adaptation: 完成后正常提示，不以人为 Exception 表示成功。

# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Keypoints descriptor example.
# This example shows how to save a keypoints descriptor to file. Show the camera an object
# and then run the script. The script will extract and save a keypoints descriptor and the image.
# You can use the keypoints_editor.py util to remove unwanted keypoints.
import csi
import time
import image

csi0 = csi.CSI()
csi0.reset()
csi0.contrast(3)
csi0.gainceiling(16)
csi0.framesize(csi.QVGA)
csi0.window((320, 240))
csi0.pixformat(csi.GRAYSCALE)
csi0.snapshot(time=2000)
csi0.auto_gain(False)  # Freeze the settled gain; 100 dB would saturate OV5640.

FILE_NAME = "desc"
img = csi0.snapshot()

# NOTE: See the docs for other arguments
# NOTE: By default find_keypoints returns multi-scale keypoints extracted from an image pyramid.
kpts = img.find_keypoints(max_keypoints=150, threshold=10, scale_factor=1.2)

if kpts is None:
    raise (Exception("Couldn't find any keypoints!"))

image.save_descriptor(kpts, "%s.orb" % (FILE_NAME))
img.save("%s.pgm" % (FILE_NAME))
restored = image.load_descriptor("%s.orb" % FILE_NAME)
print("Reloaded descriptor:", restored)

img.draw_keypoints(kpts)
csi0.snapshot()
time.sleep_ms(1000)

print("Saved. Stop the script before opening the file on the computer.")
