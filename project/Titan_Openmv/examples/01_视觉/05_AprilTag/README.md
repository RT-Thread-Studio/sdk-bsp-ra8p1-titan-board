# AprilTag

运行前加载配套 `openmv-vision.bin`，在 OpenMV IDE 打开单个脚本。相机示例使用 OV5640。

| 示例 | 内容 | 使用条件 |
| --- | --- | --- |
| [find_apriltags.py](find_apriltags.py) | AprilTag识别 | 无需外部素材 |
| [find_apriltags_3d_pose.py](find_apriltags_3d_pose.py) | AprilTag三维位姿 | 定量位姿需用当前 OV5640 镜头标定 fx/fy/cx/cy；示例内参仅用于演示变化；明确上游 OV7725 镜头内参仅为演示值，保留位姿属性访问和六自由度 print 输出。 |
| [find_apriltags_max_res.py](find_apriltags_max_res.py) | VGA高分辨率AprilTag识别 | 说明当前固件已启用高分辨率 AprilTag，VGA 不受旧注释中的 64K 像素限制。 |
| [find_apriltags_w_lens_zoom.py](find_apriltags_w_lens_zoom.py) | 裁剪放大AprilTag识别 | 无需外部素材 |
| [find_small_apriltags.py](find_small_apriltags.py) | 色块预筛选小AprilTag识别 | 补齐 QVGA 设置；保留 ROI 上限并移除吞掉 MemoryError 的处理，使实际错误可见。 |
