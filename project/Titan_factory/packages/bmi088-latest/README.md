# BMI088

## 简介

本软件包是为 BOSCH BMI088 6轴传感器提供的通用传感器驱动包。

本驱动软件包暂时只支持SPI驱动。

本文档介绍该软件包的基本功能和使用方法，本文主要内容如下：

- 传感器介绍
- 支持情况
- 使用说明

## 传感器介绍

BMI088是BOSCH推出的一款高性能惯性测量单元（IMU），具有较好的抗震性能，为无人机、机器人应用环境特别设计。其包含3轴陀螺仪和3轴加速度计。

## 支持情况

| 包含设备         | 加速计   | 陀螺仪 | 
| ---------------- | -------- | ------ | 
| **通讯接口**     |          |        |  
| IIC              |          |        | 
| SPI              | √        | √      | 
| **工作模式**     |          |        |
| 轮询             | √        | √      | 
| 中断             |          |        | 
| FIFO             |          |        | 
| **电源模式**     |          |        | 
| 掉电             | √        | √      | 
| 低功耗           | √        |        | 
| 普通             | √        | √      | 
| 高功耗           |          |        | 
| **数据输出速率** | √        | √      | 
| **测量范围**     | √        | √      | 
| **自检**         | √        | √      | 
| **多实例**       | √        | √      | 

## 使用说明

### 依赖

- RT-Thread 4.0.0+
- Sensor 组件
- SPI 驱动：BMI088 设备使用 SPI 进行数据通讯，需要系统 SPI 驱动框架支持；

### 获取软件包

使用 BMI088 软件包需要在 RT-Thread 的包管理中选中它，具体路径如下：

```
BMI088: BMI088 Digital 6-axis sensor
    [*]   Enable BMI088 accelerometer
    [*]   Enable BMI088 gyroscope
        Version (latest)  --->
```

**Enable BMI088 accelerometer**： 配置开启加速度测量功能

**Enable BMI088 gyroscope**：配置开启陀螺仪功能

**Version**：软件包版本选择

### 使用软件包

BMI088 软件包初始化函数如下所示：

```
rt_err_t rt_hw_bmi088_init(const char *name, struct rt_sensor_config *acc_cfg, struct rt_sensor_config *gyr_cfg);
```

`rt_hw_bmi088_init`函数需要由用户调用，函数主要完成的功能有，

- 对传感器相应SPI设备进行配置；
- 对传感器初始化，并将range、bw、odr进行默认配置；
- 注册相应的传感器设备，完成 BMI088 设备的注册；

BMI088共用一个驱动，初始化程序会检查加速度计和陀螺仪对应SPI设备是否挂载在同一SPI总线，并对总线统一进行配置。

## 注意事项

暂无

## 联系人信息

维护人:

- [MyGuo](https://github.com/gmyFighting)
