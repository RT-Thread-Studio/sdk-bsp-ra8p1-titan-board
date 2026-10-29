# 录像

运行前加载配套 `openmv-vision.bin`，在 OpenMV IDE 打开单个脚本。相机示例使用 OV5640。

| 示例 | 内容 | 使用条件 |
| --- | --- | --- |
| [gif.py](gif.py) | GIF录制 | 可写 FAT 介质（SD 卡）；运行时电脑停止读写该盘；LED 名称改为当前板级接口的数字编号（1/3）。；完成后正常提示，不以人为 Exception 表示成功。 |
| [gif_on_face_detection.py](gif_on_face_detection.py) | 人脸触发GIF录制 | 将 haarcascade_frontalface.cascade 复制到介质根目录，脚本沿用 /rom/haarcascade_frontalface.cascade 读取；可写 FAT 介质（SD 卡）；运行时电脑停止读写该盘；LED 名称改为当前板级接口的数字编号（1/3）。；沿用官方 /rom 模型路径；本板将 FAT 根目录挂载为 /rom 只读别名。 |
| [gif_on_movement.py](gif_on_movement.py) | 运动触发GIF录制 | 可写 FAT 介质（SD 卡）；运行时电脑停止读写该盘；LED 名称改为当前板级接口的数字编号（1/3）。 |
| [imageio_memory.py](imageio_memory.py) | 内存图像流录放 | 内存录制 20 帧，120×120 RGB565 约 563 KiB。 |
| [imageio_read.py](imageio_read.py) | 图像流读取（自动生成素材） | 可写 FAT 介质（SD 卡）；运行时电脑停止读写该盘；自动录制 12 帧生成 stream.bin 后回放，不依赖缺失输入文件。 |
| [imageio_write.py](imageio_write.py) | 图像流写入 | 可写 FAT 介质（SD 卡）；运行时电脑停止读写该盘；LED 名称改为当前板级接口的数字编号（1/3）。；原始图像流固定录制 24 帧，避免十秒高速录制写满 6 MiB Flash。；完成后正常提示，不以人为 Exception 表示成功。 |
| [mjpeg.py](mjpeg.py) | MJPEG录制 | 可写 FAT 介质（SD 卡）；运行时电脑停止读写该盘；LED 名称改为当前板级接口的数字编号（1/3）。；完成后正常提示，不以人为 Exception 表示成功。 |
| [mjpeg_on_face_detection.py](mjpeg_on_face_detection.py) | 人脸触发MJPEG录制 | 将 haarcascade_frontalface.cascade 复制到介质根目录，脚本沿用 /rom/haarcascade_frontalface.cascade 读取；可写 FAT 介质（SD 卡）；运行时电脑停止读写该盘；LED 名称改为当前板级接口的数字编号（1/3）。；沿用官方 /rom 模型路径；本板将 FAT 根目录挂载为 /rom 只读别名。 |
| [mjpeg_on_movement.py](mjpeg_on_movement.py) | 运动触发MJPEG录制 | 可写 FAT 介质（SD 卡）；运行时电脑停止读写该盘；LED 名称改为当前板级接口的数字编号（1/3）。 |
