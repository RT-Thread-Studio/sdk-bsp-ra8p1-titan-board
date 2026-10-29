# SPDX-License-Identifier: Apache-2.0
# 列出存储根目录 — RA8P1 Titan Board
# 本工程新增示例；使用当前固件的 MicroPython 标准库与 board 模块。
# 条件: 先插入/挂载所选 FAT 介质（默认 SD，也支持 Kconfig 选 Flash）；电脑结束复制并安全弹出 MSC 卷后再运行。
# 适配: 访问统一 / 路径；只读，不自动格式化、不自动切换介质。

import os

print("Current directory:", os.getcwd())
for name in os.listdir("/"):
    path = "/" + name
    info = os.stat(path)
    kind = "DIR " if (info[0] & 0x4000) else "FILE"
    print("%s %8d %s" % (kind, info[6], name))
