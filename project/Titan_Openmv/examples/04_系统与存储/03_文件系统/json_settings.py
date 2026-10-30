# SPDX-License-Identifier: Apache-2.0
# JSON 参数文件 — RA8P1 Titan Board
# 来源: examples/storage_filesystem.py
# 条件: 先插入/挂载所选 FAT 介质（默认 SD，也支持 Kconfig 选 Flash）；电脑结束复制并安全弹出 MSC 卷后再运行。
# 适配: 创建新的 JSON 示例文件并读回；不修改 boot.py、main.py 或现有配置。

import os
import json

index = 0
while True:
    path = "/omv_settings_%03d.json" % index
    try:
        os.stat(path)
    except OSError as error:
        if error.args[0] != 2:
            raise
        break
    index += 1

settings = {"name": "Titan Board", "threshold": 120, "enabled": True}
with open(path, "w") as stream:
    json.dump(settings, stream)
    stream.flush()
with open(path, "r") as stream:
    restored = json.load(stream)
assert restored == settings
print("Saved:", path, restored)
