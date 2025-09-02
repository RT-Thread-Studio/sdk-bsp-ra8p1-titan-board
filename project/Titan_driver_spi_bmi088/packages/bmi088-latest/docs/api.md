# API 说明

在 RT-Thread 上编程，需要用到 bmi088 6 轴传感器时，使用 bmi088 软件包就可以轻松完成传感器的配置以及传感器数据的读取，本章介绍 bmi088 软件包提供的常用 API。

### 初始化函数

```{.c}
struct bmi08x_dev *bmi088_init(const char *acc_spi_name, const char *gyro_spi_name)
```

使用指定的spi通信设备名初始化bmi088 ，并返回控制句柄。

| 参数              | 描述                                |
|:------------------|:------------------------------------|
|acc_spi_name               | 用于同 bmi088 加速度计通信的设备名（支持SPI 设备） |
|gyro_spi_name               | 用于同 bmi088 陀螺仪通信的设备名（支持SPI 设备） |
| **返回**          | **描述**                                |
|struct bmi08x_dev  *                  | bmi08x_dev 结构体的指针，它在调用 bmi08x_dev 库的其他函数时使用 |
|RT_NULL                 | 失败                                |

### 反初始化函数

```{.c}
void bmi088_deinit(struct bmi08x_dev *dev);
```

释放 bmi088 设备占据的内存空间

| 参数     | 描述                        |
| :------- | :-------------------------- |
| dev      | bmi08x_dev 结构体的指针 |
| **返回** | **描述**                    |
| 无返回值 |                             |


### 读取陀螺仪数据   

```{.c}
rt_size_t bmi088_get_gyro(struct bmi08x_dev *dev, struct bmi088_data *buf);
```

读取陀螺仪数据 （单位： rad/s）

| 参数     | 描述                                    |
| :------- | :-------------------------------------- |
| dev      | bmi08x_dev 结构体的指针                 |
| buf      | 存储 bmi088 3轴陀螺仪数据结构体的指针   |
| **返回** | **描述**                                |
| rt_size_t| 读取数据数                              |
|  0       | 读取失败                                |

3 轴陀螺仪数据的结构体定义如下

```{.c}
struct bmi088_3axes
{
    rt_int16_t x;
    rt_int16_t y;
    rt_int16_t z;
};
```

### 读取加速度计数据

```{.c}
rt_size_t bmi088_get_accel(struct bmi08x_dev *dev, struct bmi088_data *buf);
```

读取加速度计数据 （单位： m/s^2）

| 参数     | 描述                                    |
| :------- | :-------------------------------------- |
| dev      | bmi08x_dev 结构体的指针                 |
| buf      | 存储 bmi088 3轴加速度数据结构体的指针   |
| **返回** | **描述**                                |
| rt_size_t| 读取数据数                              |
|  0       | 读取失败                                |

3 轴加速度计数据的结构体定义如下

```{.c}
struct bmi088_3axes
{
    rt_int16_t x;
    rt_int16_t y;
    rt_int16_t z;
};
```
