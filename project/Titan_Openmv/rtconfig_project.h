/* Project settings retained by RT-Thread menuconfig header generation. */
#ifndef TITAN_OPENMV_RTCONFIG_PROJECT_H
#define TITAN_OPENMV_RTCONFIG_PROJECT_H
#define TITAN_OPENMV_SRAM_GC_BYTES OPENMV_HEAP_SIZE
#define BSP_USING_OPENMV_ML
#define BSP_USING_OPENMV_NPU
#define OMV_ENABLE_TF
#if !defined(BSP_OPENMV_STORAGE_SD) || !defined(BSP_USING_SDHI0) || !defined(RT_USING_SDIO)
#error "Titan Board OpenMV requires SDHI0 storage"
#endif
#if defined(BSP_OPENMV_STORAGE_FLASH) || defined(BSP_USING_SDRAM)
#error "Mini W25Q64/SDRAM configuration is incompatible with Titan Board"
#endif
#if !defined(BSP_USING_HYPERRAM) || BSP_USING_HYPERRAM_SIZE != 0x02000000
#error "Titan Board OpenMV requires 32 MiB OSPI1 CS0 HyperRAM"
#endif
#if defined(BSP_USING_FILESYSTEM) || defined(BSP_USING_QSPI_FLASH) || defined(RT_USING_FAL)
#error "OpenMV owns FAT mounting and uses only the SD card"
#endif
#if !defined(RT_USING_DFS_V1)
#error "Titan Board OpenMV requires DFS v1"
#endif
#if defined(BSP_USING_RA8P1_USB) || defined(BSP_USING_CEU_CAMERA)
#error "OpenMV owns TinyUSB and MIPI CSI; disable generic USB/CEU drivers"
#endif
#if defined(BSP_START_SECONDARY_CORE)
#error "OpenMV owns the M85 memory layout; keep the M33 stopped"
#endif
#endif
