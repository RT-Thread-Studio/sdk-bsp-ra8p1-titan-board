# SPDX-License-Identifier: Apache-2.0
# 二进制文件与随机读取 — RA8P1 Titan Board
# 来源: examples/storage_filesystem.py
# 条件: 先插入/挂载所选 FAT 介质（默认 SD，也支持 Kconfig 选 Flash）；电脑结束复制并安全弹出 MSC 卷后再运行。
# 适配: 未使用文件名；用 struct 打包、seek/readinto 读回；不覆盖用户文件。

import os
import struct

index = 0
while True:
    path = "/omv_binary_%03d.bin" % index
    try:
        os.stat(path)
    except OSError as error:
        if error.args[0] != 2:
            raise
        break
    index += 1

record_size = struct.calcsize("<IH")
with open(path, "wb") as stream:
    for index in range(10):
        stream.write(struct.pack("<IH", index, index * 100))
    stream.flush()
with open(path, "rb") as stream:
    stream.seek(5 * record_size)
    record = bytearray(record_size)
    if stream.readinto(record) != record_size:
        raise OSError("Incomplete record")
    index, value = struct.unpack("<IH", record)
    assert (index, value) == (5, 500)
    print("Record:", index, value)
print("Saved:", path)
