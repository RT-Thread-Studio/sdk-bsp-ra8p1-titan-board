# SPDX-License-Identifier: Apache-2.0
# 文本文件写入与持久化 — RA8P1 Titan Board
# 来源: examples/storage_filesystem.py
# 条件: 先插入/挂载所选 FAT 介质（默认 SD，也支持 Kconfig 选 Flash）；电脑结束复制并安全弹出 MSC 卷后再运行。
# 适配: 选择未使用文件名，不覆盖已有文件；写后 flush 并校验 /rom 只读别名，保留结果。

import os

index = 0
while True:
    path = "/omv_text_%03d.txt" % index
    try:
        os.stat(path)
    except OSError as error:
        if error.args[0] != 2:
            raise
        break
    index += 1

payload = "Titan Board OpenMV persistent storage\n"
with open(path, "w") as stream:
    stream.write(payload)
    stream.flush()
with open(path, "a") as stream:
    stream.write("Append from MicroPython\n")
    stream.flush()
expected = payload + "Append from MicroPython\n"
with open(path, "r") as stream:
    assert stream.read() == expected
with open("/rom" + path, "r") as stream:
    assert stream.read() == expected
print("Saved:", path)
print("File is kept; check it again after reset.")
