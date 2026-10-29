# SPDX-License-Identifier: Apache-2.0
# 目录、重命名与清理 — RA8P1 Titan Board
# 来源: examples/storage_filesystem.py
# 条件: 先插入/挂载所选 FAT 介质（默认 SD，也支持 Kconfig 选 Flash）；电脑结束复制并安全弹出 MSC 卷后再运行。
# 适配: 只创建独占的新目录；演示删除仅限本脚本刚创建的测试文件及目录。

import os

index = 0
while True:
    directory = "/omv_directory_%03d" % index
    try:
        os.stat(directory)
    except OSError as error:
        if error.args[0] != 2:
            raise
        break
    index += 1

os.mkdir(directory)
original = directory + "/original.txt"
renamed = directory + "/renamed.txt"
with open(original, "w") as stream:
    stream.write("Directory and rename example\n")
    stream.flush()
os.rename(original, renamed)
print("After rename:", os.listdir(directory))
with open(renamed, "r") as stream:
    print(stream.read())
# 仅删除本次在全新目录中创建的文件。
os.remove(renamed)
os.rmdir(directory)
print("Removed the example's own file and directory:", directory)
