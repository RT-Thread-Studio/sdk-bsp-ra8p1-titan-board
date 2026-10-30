# 拍照

运行前加载配套 `openmv-vision.bin`，在 OpenMV IDE 打开单个脚本。相机示例使用 OV5640。

| 示例 | 内容 | 使用条件 |
| --- | --- | --- |
| [emboss_snapshot.py](emboss_snapshot.py) | 浮雕滤镜拍照 | 可写 FAT 介质（SD 卡）；运行时电脑停止读写该盘；LED 名称改为当前板级接口的数字编号（1/3）。；完成后正常提示，不以人为 Exception 表示成功。 |
| [snapshot.py](snapshot.py) | 拍照保存 | 可写 FAT 介质（SD 卡）；运行时电脑停止读写该盘；LED 名称改为当前板级接口的数字编号（1/3）。；完成后正常提示，不以人为 Exception 表示成功。 |
| [snapshot_on_face_detection.py](snapshot_on_face_detection.py) | 人脸触发拍照 | 将 haarcascade_frontalface.cascade 复制到介质根目录，脚本沿用 /rom/haarcascade_frontalface.cascade 读取；可写 FAT 介质（SD 卡）；运行时电脑停止读写该盘；LED 名称改为当前板级接口的数字编号（1/3）。；沿用官方 /rom 模型路径；本板将 FAT 根目录挂载为 /rom 只读别名。 |
| [snapshot_on_movement.py](snapshot_on_movement.py) | 运动触发拍照 | 可写 FAT 介质（SD 卡）；运行时电脑停止读写该盘；LED 名称改为当前板级接口的数字编号（1/3）。 |
| [time_lapse_photos.py](time_lapse_photos.py) | 定时连拍（持续供电） | 可写 FAT 介质（SD 卡）；运行时电脑停止读写该盘；LED 名称改为当前板级接口的数字编号（1/3）。；保留定时拍照功能，使用持续供电的 ticks_ms/sleep_ms 定时；明确不提供本端口未实现的 RTC 唤醒/深睡眠。 |
