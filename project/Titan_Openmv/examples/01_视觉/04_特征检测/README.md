# 特征检测

运行前加载配套 `openmv-vision.bin`，在 OpenMV IDE 打开单个脚本。相机示例使用 OV5640。

| 示例 | 内容 | 使用条件 |
| --- | --- | --- |
| [edges.py](edges.py) | 边缘检测 | 无需外部素材 |
| [find_circles.py](find_circles.py) | 圆检测 | 无需外部素材 |
| [find_line_segments.py](find_line_segments.py) | 线段检测 | 无需外部素材 |
| [find_lines.py](find_lines.py) | 直线检测 | 无需外部素材 |
| [find_rects.py](find_rects.py) | 矩形检测 | 无需外部素材 |
| [hog.py](hog.py) | 方向梯度直方图 | 可写 FAT 介质（SD 卡）；运行时电脑停止读写该盘 |
| [keypoints.py](keypoints.py) | 关键点匹配与跟踪 | 锁定稳定后的增益，去除上游示例对 OV5640 会严重饱和的 100 dB 强制增益。 |
| [keypoints_save.py](keypoints_save.py) | 关键点描述符保存与加载 | 可写 FAT 介质（SD 卡）；运行时电脑停止读写该盘；锁定稳定后的增益，去除上游示例对 OV5640 会严重饱和的 100 dB 强制增益。；保存后重新加载描述符，示范完整文件读写。；完成后正常提示，不以人为 Exception 表示成功。 |
| [lbp.py](lbp.py) | 局部二值模式人脸描述符 | 将 haarcascade_frontalface.cascade 复制到 SD 根目录，通过 /rom 只读别名读取；使用 QVGA 采集和 240×160 window。 |
| [linear_regression.py](linear_regression.py) | 线性回归巡线 | 无需外部素材 |
| [selective_search.py](selective_search.py) | 选择性搜索候选区域 | 无需外部素材 |
| [template_matching.py](template_matching.py) | 现场采集模板并匹配 | 有清晰纹理的目标；启动时对准画面中心32×32区域；启动时从画面中心采集 32×32 灰度模板，替代缺失的 template.pgm。 |
