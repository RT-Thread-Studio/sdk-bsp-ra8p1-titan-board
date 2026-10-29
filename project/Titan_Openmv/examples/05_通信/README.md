# 通信

本板通过 J3 的 USB CDC 连接 OpenMV IDE，提供脚本运行、REPL 和图像预览。
J3 同时提供 SD 卡的 USB 存储访问；电脑端复制文件后安全弹出磁盘，再运行
会写文件的脚本。

SCI8（P500 RX / P501 TX，115200、8N1）用于 RT-Thread 控制台。
Python 的硬件 `machine.UART` 接口未开放。
