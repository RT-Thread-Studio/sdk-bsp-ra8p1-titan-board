# IST8310

中文页 | [English](README_EN.md)

## 简介

本软件包是为 Isentek 公司的 IST8310 三轴磁力计提供的传感器驱动包。本软件包已对接 RT-Thread 的 I2C驱动框架，开发者可以快速集成和使用该磁力计。

## 支持情况

| 功能特性         | 支持情况 |
| ---------------- | -------- |
| **通讯接口**     |          |
| IIC              | √        |
| **工作模式**     |          |
| 轮询             | √        |
| 中断             |          |
| **电源模式**     |          |
| 掉电             | √        |
| 普通             | √        |
| **数据输出速率** | √        |
| **测量范围**     | √        |
| **软件滤波**     | √        |
| **多实例**       | √        |

## 使用说明

### 依赖

- RT-Thread 4.0.0+
- Sensor 组件
- IIC 驱动：IST8310 设备使用 IIC 进行数据通讯，需要系统 IIC 驱动支持

### 获取软件包

使用 IST8310 软件包需要在 RT-Thread 的包管理中选中它，具体路径如下：

```
RT-Thread online packages  --->
  peripheral libraries and drivers  --->
    ist8310: 3-axis magnetometer driver package
            Version (latest)  --->
    [*] Enable ist8310 sample
    (i2c2)  i2c dev name with ist8310
    [*] Enable ist8310 mag
    [*]     Enable software filter
    (100)       Sample period (ms)
    (5)         Average times
```

**Enable ist8310 mag**：配置开启磁力计功能  
**Enable software filter**：配置开启软件滤波功能  
**Sample period**：设置采样周期（毫秒）  
**Average times**：设置平均滤波次数  

### 使用软件包

IST8310 软件包初始化函数如下所示：

```c
ist8310_device_t ist8310_init(const char *i2c_bus_name);
```

该函数需要由用户调用，主要完成以下功能：
- 设备配置和初始化
- 注册传感器设备

#### 初始化示例

```c
#include <rtthread.h>
#include "ist8310.h"

ist8310_device_t dev = ist8310_init("i2c2");
if (dev == RT_NULL) {
    rt_kprintf("IST8310 init failed\n");
    return;
}
```

#### 读取数据示例

```c
/* 设置磁偏角（根据实际位置设置） */
ist8310_set_declination(dev, 0.15f);  /* 例如：0.15弧度 */

while (1)
{
    ist8310_data_t data;
    if (ist8310_read_magnetometer(dev, &data) == RT_EOK)
    {
        rt_kprintf("Magnetic: X=%.2f µT, Y=%.2f µT, Z=%.2f µT\n", data.x, data.y, data.z);
    }

    float heading = ist8310_read_heading(dev);
    rt_kprintf("Heading: %.2f°\n", heading);

    rt_thread_mdelay(1000);
}
```

## 注意事项

1. 使用前请确保 I2C 总线已正确初始化
2. 如需高精度测量，建议进行传感器校准
3. 软件滤波会占用额外内存和CPU资源

## 联系人信息

维护人:

- [kurisaW](https://github.com/kurisaW) 

- 主页：<https://github.com/kurisaW/IST8310>