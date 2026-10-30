# Titan Board OpenMV

**English** | [中文](README_zh.md)

OpenMV 5.0.1 and MicroPython for the RA8P1 Titan Board, running on RT-Thread.
The project provides OV5640 MIPI camera capture, OpenMV IDE preview, USB CDC/MSC,
SD storage and TFLM/Ethos-U55 integration. Dependencies are included in
`ThirdParty/`; the project uses this SDK's `rt-thread/` and `libraries/`.

## Hardware

| Resource | Configuration |
|---|---|
| CPU | Cortex-M85; Cortex-M33 is not started |
| External RAM | 32 MiB HyperRAM at `0x70000000` |
| Camera | OV5640 on J9, two MIPI lanes |
| Application USB | J3, USB HS CDC + MSC |
| Debug connection | J4, J-Link/SWD |
| Console | SCI8, P500 RX / P501 TX, 115200 baud, 8N1 |
| Storage | FAT/FAT32 SD card on SDHI0 |
| LEDs | Red P013, green P613, blue P012; active low |
| User key | P110; active low |

Python supports `board.LED`, GPIO/IRQ, I2C, SoftI2C, SoftSPI and software timers.
Hardware `machine.UART`, `PWM`, `SPI`, `ADC` and LCD/display are disabled.
See [pin assignments and supported interfaces](docs/peripherals.md).

## Build and download

Use Arm GNU Toolchain **13.3.Rel1**, Python **3.10+**, SCons, and native GCC,
GNU make and a POSIX shell for `mpy-cross`. Full setup and Studio import steps
are in [build instructions](docs/build.md).

In RT-Thread Studio, build with `make -j16 openmv-all`, then run the
**openmv-download** Make Target. From the Studio build console in `Debug/`:

```text
make -j16 openmv-download
```

Alternatively, from the project directory:

```text
python -m SCons -j8
python tools/download.py --jlink "C:/tools/JLink_v848/JLink.exe"
```

Replace the J-Link path with the local installation. The project selects
J-Link **v8.48**. Use the supplied download target/script for programming.

Both build methods produce:

| File | Destination |
|---|---|
| `firmware/rtthread.hex` | MCU, programmed through J-Link |
| `firmware/openmv-vision.bin` | SD card root directory |

**Always update both files from the same build.** Camera and image processing
require the matching vision file on the SD card, including live IDE preview.
Without it, basic USB/REPL access remains available.

## Run an example

1. With power off, connect the OV5640 to J9 and insert a FAT/FAT32 SD card.
2. Program the HEX and copy `openmv-vision.bin` to the SD card root. Connect J3
   to the computer to access the USB storage drive, or use a card reader.
3. Safely eject the drive, reset the board and connect OpenMV IDE through J3.
4. Run [board_smoke_test.py](examples/board_smoke_test.py), then
   [helloworld.py](examples/01_视觉/00_入门/helloworld.py).

The startup indicator is blue while initializing, green when the runtime and
vision image are ready, and red on an initialization/load error. Calling
`board.LED` gives the script control of the LEDs until the next soft reset.

Save an application as `main.py` in the SD root for automatic startup; optional
`boot.py` and `lib/` also reside there. `/rom` is a read-only alias of the SD
root. Finish and safely eject computer-side file access before board-side writes.

OpenMV IDE may identify the board as **AE3** for protocol compatibility. Update
with this project's files and download tool; Alif AE3 firmware is incompatible.

## Examples and dependencies

- [Python examples](examples/README.md)
- [Model deployment and licenses](models/README.md)
- [Dependency versions and licenses](ThirdParty/README.md)

The local `ra/`, `ra_cfg/` and `ra_gen/` contain the FSP 6.4 configuration.
Maintain these source files together; a RASC `configuration.xml` is not included.
