# 资源来源与许可

两个 `.cascade` 直接复制到存储根目录，官方脚本通过 `/rom/haarcascade_frontalface.cascade` 和 `/rom/haarcascade_eye.cascade` 读取；本板 `/rom` 是根目录只读别名，不需要新建文件夹。`force_int_quant.tflite` 仍放在 `/omv_assets/ml/`。分发资源时保留本文件及 `LICENSE.frontalface.txt`。

本资源固定来源为项目 `ThirdParty/openmv`（revision `631681e5ac5ab332c920b6bf9cd3dba5cf299eac`）。

| 文件 | 源与生成方式 | 许可说明 |
|---|---|---|
| `haarcascade_frontalface.cascade` | `lib/haar/haarcascade_frontalface.xml` 经同版本 `tools/haar2c.py` 转换，25 stages | 原 XML 内含 Intel/OpenCV License，全文保留于 [LICENSE.frontalface.txt](LICENSE.frontalface.txt)，保留 Rainer Lienhart 署名。 |
| `haarcascade_eye.cascade` | `lib/haar/haarcascade_eye.xml` 经同版本 `tools/haar2c.py` 转换，24 stages | 所用 XML 无独立许可头；保留原始来源，不将示例 MIT 许可扩展到该级联权重。 |
| `force_int_quant.tflite` | 逐字节复制 `lib/models/force_int_quant.tflite`，10,600 字节 | 固定版本未见此二进制的独立权重许可声明；保留来源，不新赋予许可。 |

转换工具版权：Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>; Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>，原工具声明 MIT。

OpenMV 模型说明另见 [OPENMV_MODEL_NOTICES.md](../../../models/licenses/OPENMV_MODEL_NOTICES.md)。本目录脚本中的 MIT 声明适用于对应示例代码，不独立证明任何模型或训练数据的授权。
