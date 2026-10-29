/* SPDX-License-Identifier: Apache-2.0 */
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "py/runtime.h"
#include "py/stream.h"
#include "py/mphal.h"
#include "shared/runtime/interrupt_char.h"
#include "shared/runtime/softtimer.h"
#include "tusb.h"
#include "ra8_usb_session.h"
#include "ra8_usb_msc.h"
#include "omv_protocol.h"
#include "ra8_vm_poll.h"
#include "titan_vision.h"

static bool timers_pending;
static uint32_t timer_deadline;
static bool protocol_polling;
static struct rt_timer vm_poll_timer;
static mp_sched_node_t vm_poll_node;
static volatile bool vm_poll_ready;
static bool vm_poll_timer_created;
/* VM lifecycle only, with interrupts masked. mp_init preserves the static
 * scheduler list across soft reset. Never clear a node before unlinking it:
 * doing so truncates following callbacks and can leave a null callback queued.
 * A callback already removed by mp_sched_run_pending is harmless here. */
static void vm_poll_unlink(void) {
    mp_sched_node_t *previous = NULL;
    mp_sched_node_t *node = MP_STATE_VM(sched_head);
    while (node) {
        if (node == &vm_poll_node) {
            if (previous) {
                previous->next = node->next;
            } else {
                MP_STATE_VM(sched_head) = node->next;
            }
            if (MP_STATE_VM(sched_tail) == node) {
                MP_STATE_VM(sched_tail) = previous;
            }
            break;
        }
        previous = node;
        node = node->next;
    }
    vm_poll_node.callback = NULL;
    vm_poll_node.next = NULL;
    /* Preserve scheduler locks, Python callbacks and pending exceptions.
     * A harmless PENDING state is settled by the next scheduler pass. */
}

/* Schedule from RT-Thread's timer, but run Python/protocol work only in the VM.
 * The event-wait hook alone is never reached by a Python busy loop. */
static void vm_poll_task(mp_sched_node_t *node) {
    (void)node;
    if (vm_poll_ready) { mp_hal_poll(); }
}
static void vm_poll_tick(void *arg) {
    (void)arg;
    rt_base_t level = rt_hw_interrupt_disable();
    if (vm_poll_ready) { mp_sched_schedule_node(&vm_poll_node, vm_poll_task); }
    rt_hw_interrupt_enable(level);
}
void ra8_vm_poll_recover(void) { protocol_polling = false; }
void ra8_vm_poll_start(void) {
    rt_base_t level = rt_hw_interrupt_disable();
    vm_poll_ready = false;
    if (vm_poll_timer_created) { rt_timer_stop(&vm_poll_timer); }
    vm_poll_unlink();
    protocol_polling = false;
    timers_pending = false;
    if (!vm_poll_timer_created) {
        rt_timer_init(&vm_poll_timer, "omv_poll", vm_poll_tick, NULL,
                      rt_tick_from_millisecond(10), RT_TIMER_FLAG_PERIODIC);
        vm_poll_timer_created = true;
    }
    vm_poll_ready = true;
    rt_timer_start(&vm_poll_timer);
    rt_hw_interrupt_enable(level);
}
void ra8_vm_poll_stop(void) {
    rt_base_t level = rt_hw_interrupt_disable();
    vm_poll_ready = false;
    if (vm_poll_timer_created) { rt_timer_stop(&vm_poll_timer); }
    vm_poll_unlink();
    timers_pending = false;
    protocol_polling = false;
    rt_hw_interrupt_enable(level);
}
uint32_t soft_timer_get_ms(void) { return mp_hal_ticks_ms(); }
void soft_timer_schedule_at_ms(uint32_t deadline) { timer_deadline = deadline; timers_pending = true; }
static void poll_cleanup(void *arg) {
    (void)arg;
    protocol_polling = false;
}
void mp_hal_poll(void) {
    if (protocol_polling || !vm_poll_ready) { return; }
    nlr_jump_callback_node_t cleanup;
    nlr_push_jump_callback(&cleanup, poll_cleanup);
    protocol_polling = true;
    /* Session changes belong to the VM, before timers/protocol can read stale
     * packets. The guard also covers timer callbacks and exception unwinding. */
    ra8_usb_session_poll();
    if (timers_pending && (int32_t)(mp_hal_ticks_ms() - timer_deadline) >= 0) {
        timers_pending = false;
        soft_timer_handler();
    }
    if (vm_poll_ready && omv_protocol_is_active()) {
        omv_protocol_poll();
    }
    nlr_pop_jump_callback(true);
    /* yield() only serves this priority. Let SD/vision workers progress even
     * during CPU-only Python, after servicing the IDE on this VM thread. */
    if (ra8_usb_msc_needs_service() || titan_vision_needs_service()) { rt_thread_mdelay(1); }
    else { rt_thread_yield(); }
}
mp_uint_t mp_hal_ticks_ms(void) { return (mp_uint_t)((uint64_t)rt_tick_get() * 1000 / RT_TICK_PER_SECOND); }
mp_uint_t mp_hal_ticks_us(void) {
    rt_base_t level = rt_hw_interrupt_disable();
    uint32_t ticks = rt_tick_get();
    uint32_t pending = SCB->ICSR & SCB_ICSR_PENDSTSET_Msk;
    uint32_t count = SysTick->VAL;
    uint32_t pending_after = SCB->ICSR & SCB_ICSR_PENDSTSET_Msk;
    if (pending != pending_after) { count = SysTick->VAL; }
    if (pending_after) { ticks++; }
    uint32_t elapsed = SysTick->LOAD - count;
    rt_hw_interrupt_enable(level);
    return (mp_uint_t)((uint64_t)ticks * 1000000 / RT_TICK_PER_SECOND
                      + (uint64_t)elapsed * 1000000 / SystemCoreClock);
}
mp_uint_t mp_hal_ticks_cpu(void) { return DWT->CYCCNT; }
uint32_t ra8_random_seed(void) { return DWT->CYCCNT ^ rt_tick_get(); }
long random(void) { return rand(); }
uint32_t rng_randint(uint32_t minimum, uint32_t maximum) {
    if (maximum <= minimum) { return minimum; }
    uint32_t value = ((uint32_t)rand() << 16) ^ (uint32_t)rand();
    return minimum + (uint32_t)(value % ((uint64_t)maximum - minimum + 1));
}
uint64_t mp_hal_time_ns(void) { return (uint64_t)time(NULL) * 1000000000ULL; }
void mp_hal_delay_us(mp_uint_t us) { rt_hw_us_delay(us); }
void mp_hal_delay_ms(mp_uint_t ms) {
    mp_uint_t start = mp_hal_ticks_ms();
    while ((mp_uint_t)(mp_hal_ticks_ms() - start) < ms) { mp_handle_pending(true); mp_hal_poll(); rt_thread_mdelay(1); }
}
extern int ra8_usb_serial_any(void);
extern int ra8_usb_serial_read(void);
int mp_hal_stdin_rx_any(void) { return !omv_protocol_is_active() && ra8_usb_serial_any(); }
int mp_hal_stdin_rx_chr(void) {
    for (;;) {
        if (mp_hal_stdin_rx_any()) { int value = ra8_usb_serial_read(); if (value >= 0) { return value; } }
        mp_handle_pending(true);
        mp_hal_poll();
        rt_thread_mdelay(1);
    }
}
mp_uint_t __real_mp_hal_stdout_tx_strn(const char *str, size_t len) {
    /* Do not emit raw text into an IDE connection waiting for protocol SYNC. */
    return ra8_usb_serial_write(str, len);
}

mp_uint_t mp_hal_stdout_tx_strn(const char *str, size_t len) {
    extern mp_uint_t __wrap_mp_hal_stdout_tx_strn(const char *, mp_uint_t);
    return __wrap_mp_hal_stdout_tx_strn(str, len);
}
uintptr_t __real_mp_hal_stdio_poll(uintptr_t flags) {
    return (flags & MP_STREAM_POLL_WR) | (mp_hal_stdin_rx_any() ? (flags & MP_STREAM_POLL_RD) : 0);
}
uintptr_t mp_hal_stdio_poll(uintptr_t flags) {
    extern uintptr_t __wrap_mp_hal_stdio_poll(uintptr_t);
    return __wrap_mp_hal_stdio_poll(flags);
}
void mp_hal_get_mac(int idx, uint8_t buf[6]) { memcpy(buf, (void *)OMV_BOARD_UID_ADDR, 6); buf[0] = (buf[0] & 0xfe) | 2; buf[5] += idx; }
