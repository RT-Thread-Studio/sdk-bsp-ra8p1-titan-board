/*
 * Copyright (c) 2006-2025, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author        Notes
 * 2025-05-21     kurisaW       first version
 */


#ifndef RPMSG_PLATFORM_H_
#define RPMSG_PLATFORM_H_

#include <stdint.h>

#if (_RA_CORE == CPU0) && defined(SOC_SERIES_R7KA8P1)
    #include <rtthread.h>

    extern volatile int rp_flag;
    extern rt_mailbox_t my_rpmsg_mb;
#endif

#ifndef VRING_ALIGN
#define VRING_ALIGN (0x1000U)
//#define VRING_ALIGN (0x10U)
#endif

/* contains pool of descriptos and two circular buffers */
#if 0
#ifndef VRING_SIZE
#define VRING_SIZE (0x1000UL)
#endif
#endif

#define VRING_SIZE1 (RL_BUFFER_COUNT * sizeof(struct vring_desc))
#define VRING_SIZE2 (VRING_SIZE1 + sizeof(struct vring_avail) + (RL_BUFFER_COUNT * sizeof(uint16_t)) + sizeof(uint16_t))
#define VRING_SIZE3 ((VRING_SIZE2 + VRING_ALIGN - 1UL) & ~(VRING_ALIGN - 1UL))
#define VRING_SIZE4 (VRING_SIZE3 + sizeof(struct vring_used) + (RL_BUFFER_COUNT * sizeof(struct vring_used_elem)) + sizeof(uint16_t))
#define VRING_SIZE (((int32_t)VRING_SIZE4))

/* define shared memory space for VRINGS per one channel */
#define RL_VRING_OVERHEAD (2UL * VRING_SIZE)

#define RL_GET_VQ_ID(link_id, queue_id) (((queue_id)&0x1U) | (((link_id) << 1U) & 0xFFFFFFFEU))
#define RL_GET_LINK_ID(id)              (((id)&0xFFFFFFFEU) >> 1U)
#define RL_GET_Q_ID(id)                 ((id)&0x1U)

/* link_id: 4 bit master cpu_id and 4 bit remote_id */
#define RL_GET_R_CPU_ID(id)             (RL_GET_LINK_ID(id) & 0xFU)
#define RL_GET_M_CPU_ID(id)             ((RL_GET_LINK_ID(id) & 0xF0U) >> 4U)
#define RL_PLATFORM_SET_LINK_ID(_M, _R) (((_M << 4U) & 0xF0U) | (_R & 0xFU))

#define IPC0STA0_ADDRESS (volatile uint32_t *)0x400200C0
#define IPC0CLR0_ADDRESS (volatile uint32_t *)0x400200D0   //clear from CPU1 to CPU0.

#define IPC1STA0_ADDRESS (volatile uint32_t *)0x40020100
#define IPC1CLR0_ADDRESS (volatile uint32_t *)0x40020110  //clear from CPU0 to CPU1.

#define RPMSG_LITE_LINK_ID (0U)

#define RL_PLATFORM_HIGHEST_LINK_ID     (0xFFU)
#define RL_RPMSG_MAGIC                  (0x524D5347U)

/* Max supported ISR counts */
#define ISR_COUNT (32U)

/*!
 * Structure to keep track of registered ISR's.
 */
struct isr_info
{
    struct virtqueue *vq;
};

extern struct isr_info isr_table[ISR_COUNT];

/* platform interrupt related functions */
int32_t platform_init_interrupt(uint32_t vector_id, void *isr_data);
int32_t platform_deinit_interrupt(uint32_t vector_id);
int32_t platform_interrupt_enable(uint32_t vector_id);
int32_t platform_interrupt_disable(uint32_t vector_id);
int32_t platform_in_isr(void);
void platform_notify(uint32_t vector_id);

/* platform low-level time-delay (busy loop) */
void platform_time_delay(uint32_t num_msec);

/* platform memory functions */
void platform_map_mem_region(uint32_t vrt_addr, uint32_t phy_addr, uint32_t size, uint32_t flags);
void platform_cache_all_flush_invalidate(void);
void platform_cache_disable(void);
uint32_t platform_vatopa(void *addr);
void *platform_patova(uintptr_t addr);

/* platform init/deinit */
int32_t platform_init(void);
int32_t platform_deinit(void);

#endif /* RPMSG_PLATFORM_H_ */
