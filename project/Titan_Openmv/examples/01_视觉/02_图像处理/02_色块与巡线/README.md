# 色块与巡线

运行前加载配套 `openmv-vision.bin`，在 OpenMV IDE 打开单个脚本。相机示例使用 OV5640。

| 示例 | 内容 | 使用条件 |
| --- | --- | --- |
| [automatic_grayscale_color_tracking.py](automatic_grayscale_color_tracking.py) | 自动灰度阈值跟踪 | 无需外部素材 |
| [automatic_rgb565_color_tracking.py](automatic_rgb565_color_tracking.py) | 自动彩色阈值跟踪 | 无需外部素材 |
| [black_grayscale_line_following.py](black_grayscale_line_following.py) | 灰度黑线巡线 | 无需外部素材 |
| [image_histogram_info.py](image_histogram_info.py) | 图像直方图 | 无需外部素材 |
| [image_statistics_info.py](image_statistics_info.py) | 图像统计值 | 无需外部素材 |
| [ir_beacon_grayscale_tracking.py](ir_beacon_grayscale_tracking.py) | 灰度亮信标跟踪 | 亮点或适配镜头的信标；红外灯需无红外截止滤光片镜头 |
| [ir_beacon_rgb565_tracking.py](ir_beacon_rgb565_tracking.py) | 彩色亮信标跟踪 | 亮点或适配镜头的信标；红外灯需无红外截止滤光片镜头 |
| [multi_color_blob_tracking.py](multi_color_blob_tracking.py) | 多颜色色块跟踪 | 补上官方脚本调用 image 几何辅助函数时遗漏的 import image。 |
| [multi_color_code_tracking.py](multi_color_code_tracking.py) | 多组颜色编码跟踪 | 无需外部素材 |
| [single_color_code_tracking.py](single_color_code_tracking.py) | 单组颜色编码跟踪 | 补上官方脚本调用 image 几何辅助函数时遗漏的 import image。 |
| [single_color_grayscale_blob_tracking.py](single_color_grayscale_blob_tracking.py) | 灰度色块跟踪 | 补上官方脚本调用 image 几何辅助函数时遗漏的 import image。 |
| [single_color_rgb565_blob_tracking.py](single_color_rgb565_blob_tracking.py) | 彩色色块跟踪 | 补上官方脚本调用 image 几何辅助函数时遗漏的 import image。 |
