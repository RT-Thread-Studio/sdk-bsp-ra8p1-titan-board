Here is the English translation of the code documentation you provided:

---

# IST8310

[中文页](README.md) | English

## Introduction

This software package provides a sensor driver for the IST8310 3-axis magnetometer from Isentek. The package is integrated with the RT-Thread I2C driver framework, enabling developers to quickly integrate and use the magnetometer.

## Supported Features

| Feature                     | Supported |
| --------------------------- | --------- |
| **Communication Interface** |           |
| I2C                         | √         |
| **Operating Mode**          |           |
| Polling                     | √         |
| Interrupt                   |           |
| **Power Mode**              |           |
| Power-down                  | √         |
| Normal                      | √         |
| **Data Output Rate**        | √         |
| **Measurement Range**       | √         |
| **Software Filtering**      | √         |
| **Multi-instance**          | √         |

## Usage Instructions

### Dependencies

* RT-Thread version 4.0.0 or later
* Sensor component
* I2C driver: IST8310 uses I2C for data communication, so the system must support I2C

### Obtaining the Package

To use the IST8310 package, enable it in the RT-Thread package manager:

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

**Enable ist8310 mag**: Enables the magnetometer
**Enable software filter**: Enables software filtering
**Sample period**: Sets the sampling interval (in milliseconds)
**Average times**: Sets the number of times to average readings for filtering

### Using the Package

The initialization function for the IST8310 package is as follows:

```c
ist8310_device_t ist8310_init(const char *i2c_bus_name);
```

This function should be called by the user and performs the following tasks:

* Device configuration and initialization
* Sensor device registration

#### Initialization Example

```c
#include <rtthread.h>
#include "ist8310.h"

ist8310_device_t dev = ist8310_init("i2c2");
if (dev == RT_NULL) {
    rt_kprintf("IST8310 init failed\n");
    return;
}
```

#### Data Reading Example

```c
/* Set magnetic declination (adjust according to actual location) */
ist8310_set_declination(dev, 0.15f);  /* e.g., 0.15 radians */

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

## Notes

1. Ensure the I2C bus is properly initialized before use.
2. For high-precision measurements, sensor calibration is recommended.
3. Software filtering consumes additional memory and CPU resources.

## Maintainer Information

Maintainer:

* [kurisaW](https://github.com/kurisaW)
* Homepage: [https://github.com/kurisaW/IST8310](https://github.com/kurisaW/IST8310)
