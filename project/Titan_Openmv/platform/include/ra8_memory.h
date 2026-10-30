/* SPDX-License-Identifier: Apache-2.0 */
#ifndef RA8_MEMORY_H
#define RA8_MEMORY_H

/* Shared by the VM, HyperRAM driver adapter and generated linker layouts.
 * The low 12 MiB holds VIN DMA buffers, image UMA, the original
 * GC pool and USB preview storage. The next 4 MiB at 0x70c00000 holds the
 * build-matched SD vision image. Titan Board physically has 32 MiB HyperRAM.
 * Models and their tensor arenas need an additional contiguous GC pool.
 */
#define RA8_HYPERRAM_START          0x70000000UL
#define RA8_HYPERRAM_SIZE           0x02000000UL
#define RA8_HYPERRAM_RESERVED_SIZE  0x01000000UL
#define RA8_GC_EXTRA_START       0x71000000UL
#define RA8_GC_EXTRA_SIZE        0x00a00000UL
#define RA8_RT_HYPERRAM_START       0x71a00000UL
#define RA8_RT_HYPERRAM_SIZE        0x00600000UL

#if (RA8_HYPERRAM_START + RA8_HYPERRAM_RESERVED_SIZE != RA8_GC_EXTRA_START) || \
    (RA8_GC_EXTRA_START + RA8_GC_EXTRA_SIZE != RA8_RT_HYPERRAM_START) || \
    (RA8_RT_HYPERRAM_START + RA8_RT_HYPERRAM_SIZE != RA8_HYPERRAM_START + RA8_HYPERRAM_SIZE)
#error "RA8 HyperRAM regions must partition the physical memory without overlap"
#endif
#if defined(BSP_USING_HYPERRAM_SIZE) && (BSP_USING_HYPERRAM_SIZE != RA8_HYPERRAM_SIZE)
#error "Review the RA8 platform memory layout for the configured HyperRAM size"
#endif

#endif
