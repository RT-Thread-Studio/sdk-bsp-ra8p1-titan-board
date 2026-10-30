/* SPDX-License-Identifier: Apache-2.0 */
#ifndef TITAN_TUSB_OS_CUSTOM_H
#define TITAN_TUSB_OS_CUSTOM_H
#include <rtthread.h>
#include <rthw.h>
#ifdef RT_USING_SMP
#error "Review the Titan TinyUSB IRQ lock before enabling SMP"
#endif
#ifdef TUSB_OSAL_RTTHREAD_H_
#error "Select this RT-Thread adapter through TinyUSB OPT_OS_CUSTOM"
#endif

/* Reuse the official RT-Thread task/semaphore/mutex/queue operations.
 * Keep its two incompatible definitions private, then supply only those APIs. */
#define osal_spinlock_t tinyusb_rtthread_spinlock_t
#define osal_spin_init tinyusb_rtthread_spin_init
#define osal_spin_deinit tinyusb_rtthread_spin_deinit
#define osal_spin_lock tinyusb_rtthread_spin_lock
#define osal_spin_unlock tinyusb_rtthread_spin_unlock
#define osal_queue_create tinyusb_rtthread_queue_create
#include "osal/osal_rtthread.h"
#undef osal_spinlock_t
#undef osal_spin_init
#undef osal_spin_deinit
#undef osal_spin_lock
#undef osal_spin_unlock
#undef osal_queue_create

#ifdef __cplusplus
extern "C" {
#endif

typedef struct { rt_base_t level; } osal_spinlock_t;
static inline void osal_spin_init(osal_spinlock_t *ctx) { ctx->level = 0; }
static inline void osal_spin_deinit(osal_spinlock_t *ctx) { (void)ctx; }
static inline void osal_spin_lock(osal_spinlock_t *ctx, bool in_isr) {
    (void)in_isr;
    ctx->level = rt_hw_interrupt_disable();
}
static inline void osal_spin_unlock(osal_spinlock_t *ctx, bool in_isr) {
    (void)in_isr;
    rt_hw_interrupt_enable(ctx->level);
}

/* depth stays the requested message count. RT-Thread also needs aligned
 * payloads and a message header for each queue entry. */
#undef OSAL_QUEUE_DEF
#define OSAL_QUEUE_DEF(_int_set, _name, _depth, _type) \
    static uint8_t _name##_##buf[RT_MQ_BUF_SIZE(sizeof(_type), (_depth))] TU_ATTR_ALIGNED(RT_ALIGN_SIZE); \
    osal_queue_def_t _name = { .depth = (_depth), .item_sz = sizeof(_type), .buf = _name##_##buf };

static inline osal_queue_t osal_queue_create(osal_queue_def_t *qdef) {
    size_t size = RT_MQ_BUF_SIZE(qdef->item_sz, qdef->depth);
    if (rt_mq_init(&qdef->sq, "tusb", qdef->buf, qdef->item_sz, size, RT_IPC_FLAG_PRIO) != RT_EOK) {
        return NULL;
    }
    return &qdef->sq;
}

#ifdef __cplusplus
}
#endif
#endif
