/* SPDX-License-Identifier: Apache-2.0 */
#include <rtthread.h>
#include <rtdevice.h>
#include "py/mphal.h"
#include "py/runtime.h"
#include "shared/runtime/interrupt_char.h"
#include "tusb.h"
#include "omv_protocol.h"
/* Exported by the official core; its public header omits this declaration. */
void omv_protocol_reset(void);
#include "device/dcd.h"
#include "ra8_usb_session.h"
#include "ra8_usb_msc.h"

#if BOARD_DEVICE_RHPORT_NUM == 1
_Static_assert(BSP_CFG_XTAL_HZ == 24000000, "Titan Board USBHS PHY requires its 24 MHz board crystal");
#endif

static volatile bool line_dtr;
static volatile bool ide_selected;
static bool usb_started;
static volatile uint32_t session_epoch = 1;
static volatile uint32_t applied_epoch;

static void session_changed(void) {
    /* Callback/ISR safe: only publish transport state, never touch VM locks. */
    rt_base_t level = rt_hw_interrupt_disable();
    session_epoch++;
    rt_hw_interrupt_enable(level);
}
/* Suspend pauses the bus; it does not close CDC or replace the IDE session.
 * TinyUSB's tud_cdc_connected() includes !suspended, so it cannot be used as
 * the logical connection predicate for a partially queued protocol packet. */
static bool transport_session(void) {
    return usb_started && line_dtr && ide_selected &&
           applied_epoch == session_epoch && tud_mounted();
}
static bool transport_ready(void) {
    return transport_session() && !tud_suspended();
}
static bool serial_session(void) {
    return usb_started && line_dtr && !ide_selected &&
           applied_epoch == session_epoch && tud_mounted();
}
int ra8_usb_serial_active(void) {
    return serial_session() && !tud_suspended();
}
/* The USB task must notice Ctrl-C while a serial REPL script is busy.
 * Keep ordinary serial bytes separate from the IDE's binary protocol.
 */
static uint8_t serial_rx[PKG_TINYUSB_DEVICE_CDC_RX_BUFSIZE];
static size_t serial_head, serial_tail, serial_count;
static bool serial_receiving;
static void serial_receive(void) {
    /* Only the USB task drains raw CDC data. TinyUSB read/re-arm takes RTOS
     * mutexes, so never call it with interrupts masked. VM readers consume
     * only our ring, including when the endpoint is being re-armed here. */
    rt_base_t level = rt_hw_interrupt_disable();
    if (serial_receiving) {
        rt_hw_interrupt_enable(level);
        return;
    }
    serial_receiving = true;
    rt_hw_interrupt_enable(level);
    for (size_t received = 0; received < sizeof(serial_rx); ++received) {
        level = rt_hw_interrupt_disable();
        const uint32_t epoch = session_epoch;
        bool ready = ra8_usb_serial_active() && serial_count < sizeof(serial_rx);
        rt_hw_interrupt_enable(level);
        if (!ready || !tud_cdc_available()) { break; }
        int value = tud_cdc_read_char();
        if (value < 0) { break; }
        level = rt_hw_interrupt_disable();
        /* A bus reset can invalidate the session while read() runs. Never
         * publish those stale bytes or Ctrl-C into the replacement session. */
        if (epoch == session_epoch && ra8_usb_serial_active()) {
            if (mp_interrupt_char >= 0 && value == mp_interrupt_char) {
                mp_sched_keyboard_interrupt();
            } else {
                serial_rx[serial_head] = value;
                serial_head = (serial_head + 1) % sizeof(serial_rx);
                serial_count++;
            }
        }
        rt_hw_interrupt_enable(level);
    }
    level = rt_hw_interrupt_disable();
    serial_receiving = false;
    rt_hw_interrupt_enable(level);
}
int ra8_usb_serial_any(void) {
    rt_base_t level = rt_hw_interrupt_disable();
    int available = ra8_usb_serial_active() && serial_count != 0;
    rt_hw_interrupt_enable(level);
    return available;
}
int ra8_usb_serial_read(void) {
    rt_base_t level = rt_hw_interrupt_disable();
    int value = -1;
    if (ra8_usb_serial_active() && serial_count) {
        value = serial_rx[serial_tail];
        serial_tail = (serial_tail + 1) % sizeof(serial_rx);
        serial_count--;
    }
    rt_hw_interrupt_enable(level);
    return value;
}
void tud_cdc_rx_cb(uint8_t itf) {
    (void)itf;
    if (!line_dtr) {
        /* Late bytes from a closed IDE session must not become REPL Ctrl-C. */
        tud_cdc_read_flush();
    } else {
        serial_receive();
    }
}
static void serial_reset(void) {
    rt_base_t level = rt_hw_interrupt_disable();
    serial_head = serial_tail = serial_count = 0;
    rt_hw_interrupt_enable(level);
}
static struct rt_semaphore cdc_tx_ready;
static volatile bool cdc_tx_waiting;
/* RASC owns vector_data.c/.h. Only these ISR bodies belong to TinyUSB.
 * The FSP r_usb_basic/r_usb_pcdc sources are excluded by ra/SConscript.
 */
static void usb_interrupt(uint8_t rhport) {
    rt_interrupt_enter();
    // A new event may arrive while TinyUSB handles the current FIFO. Match
    // the upstream RA BSP: clear the current latch before servicing USB.
    R_BSP_IrqStatusClear(R_FSP_CurrentIrqGet());
    if (rhport == BOARD_DEVICE_RHPORT_NUM) { tud_int_handler(rhport); }
    rt_interrupt_leave();
}
void usbfs_interrupt_handler(void) { usb_interrupt(0); }
void usbfs_resume_handler(void) { usb_interrupt(0); }
void usbhs_interrupt_handler(void) { usb_interrupt(1); }
/* This RUSB2 driver copies FIFOs with the CPU; D0/D1 DMA IRQs stay disabled. */
void usbfs_d0fifo_handler(void) { R_BSP_IrqStatusClear(R_FSP_CurrentIrqGet()); }
void usbfs_d1fifo_handler(void) { R_BSP_IrqStatusClear(R_FSP_CurrentIrqGet()); }
void usbhs_d0fifo_handler(void) { R_BSP_IrqStatusClear(R_FSP_CurrentIrqGet()); }
void usbhs_d1fifo_handler(void) { R_BSP_IrqStatusClear(R_FSP_CurrentIrqGet()); }
static void usb_thread(void *arg) {
    (void)arg;
    for (;;) {
        /* SD I/O runs in its own worker. Service completion on this task so
         * class state/buffers cannot race a queued bus or BOT reset. */
        tud_task_ext(1, false);
        ra8_usb_msc_poll();
        /* Also drain data held until the VM applies a new session epoch, or
         * until it frees ring capacity; neither case needs another RX IRQ. */
        serial_receive();
    }
}
int ra8_usb_start(void) {
    if (rt_sem_init(&cdc_tx_ready, "usb_tx", 0, RT_IPC_FLAG_PRIO) != RT_EOK) { return -1; }
    if (ra8_usb_msc_init() != RT_EOK) { return -RT_ENOMEM; }
#if BOARD_DEVICE_RHPORT_NUM == 1
    R_BSP_IrqCfg(USBHS_USB_INT_RESUME_IRQn, 10, NULL);
#else
    R_BSP_IrqCfg(USBFS_INT_IRQn, 10, NULL);
    R_BSP_IrqCfg(USBFS_RESUME_IRQn, 10, NULL);
#endif
    const tusb_rhport_init_t init = {.role = TUSB_ROLE_DEVICE,
        .speed = BOARD_DEVICE_RHPORT_SPEED == OPT_MODE_HIGH_SPEED ?
                 TUSB_SPEED_HIGH : TUSB_SPEED_FULL};
    if (!tusb_init(BOARD_DEVICE_RHPORT_NUM, &init)) { return -1; }
#if BOARD_DEVICE_RHPORT_NUM == 1
    /* RA8P1 UM Rev.1.30 section 38.2.2: all BUSWAIT reserved bits
     * must be zero (older FSP code writes 0x0f00 here). BWAIT=7 gives
     * 9 PCLKA cycles = 72 ns at 125 MHz: >=41 ns and <=60 MB/s at
     * 32-bit FIFO width. Set it before the USB task processes requests.
     */
    R_USB_HS0->BUSWAIT = 0x0007;
#endif
    usb_started = true;
    rt_thread_t thread = rt_thread_create("tusb", usb_thread, NULL, 4096, 15, 10);
    return thread ? rt_thread_startup(thread) : -RT_ENOMEM;
}
static size_t usb_size(const omv_protocol_channel_t *channel) {
    return transport_ready() ? tud_cdc_available() : 0;
}
static bool usb_active(const omv_protocol_channel_t *channel) { (void)channel; return transport_session(); }
static void usb_wait_tx(uint32_t epoch, uint32_t milliseconds, bool raw, bool pause_only) {
    cdc_tx_waiting = true;
    bool session = raw ? serial_session() : transport_session();
    /* Recheck after publishing the waiter: resume/complete may have happened
     * just before this flag. Never sleep through a known-ready transition. */
    if (epoch == session_epoch && session &&
        (tud_suspended() || (!pause_only && !tud_cdc_write_available()))) {
        rt_sem_take(&cdc_tx_ready, rt_tick_from_millisecond(milliseconds));
    }
    cdc_tx_waiting = false;
}
static int usb_flush(const omv_protocol_channel_t *channel) {
    (void)channel;
    const uint32_t epoch = session_epoch;
    const uint32_t start = mp_hal_ticks_ms();
    while (epoch == session_epoch && transport_session()) {
        uint32_t elapsed = mp_hal_ticks_ms() - start;
        if (elapsed >= 1500) { return -1; }
        if (!tud_suspended()) { return (int)tud_cdc_write_flush(); }
        uint32_t remaining = 1500 - elapsed;
        usb_wait_tx(epoch, remaining < 10 ? remaining : 10, false, true);
    }
    return -1;
}
static int usb_read(const omv_protocol_channel_t *channel, uint32_t offset, size_t size, void *data) {
    if (!transport_ready()) { return 0; }
    return tud_cdc_read(data, size);
}
static int usb_write(const omv_protocol_channel_t *channel, uint32_t offset, size_t size, const void *data) {
    (void)channel; (void)offset;
    size_t sent = 0;
    const uint32_t start = mp_hal_ticks_ms();
    const uint32_t epoch = session_epoch;
    while (sent < size && epoch == session_epoch && transport_session()) {
        if (mp_hal_ticks_ms() - start >= 1500) {
            break;
        }
        if (!tud_suspended()) {
            sent += tud_cdc_write((const uint8_t *)data + sent, size - sent);
            if (epoch == session_epoch && transport_ready()) { tud_cdc_write_flush(); }
        }
        if (sent == size) { break; }
        uint32_t elapsed = mp_hal_ticks_ms() - start;
        if (elapsed >= 1500) { break; }
        uint32_t remaining = 1500 - elapsed;
        usb_wait_tx(epoch, remaining < 10 ? remaining : 10, false, false);
    }
    return sent;
}
static bool ide_baudrate(uint32_t baud) {
    /* IDE retries at 12 Mbps if the OS rejects the first line-coding value. */
    return baud == OMV_PROTOCOL_MAGIC_BAUDRATE || baud == 12000000;
}
static void discard_old_rx(void) {
    serial_reset();
    if (usb_started && tud_mounted()) {
        tud_cdc_read_flush();
    }
}
void tud_cdc_line_coding_cb(uint8_t itf, const cdc_line_coding_t *coding) {
    (void)itf;
    bool selected = ide_baudrate(coding->bit_rate);
    if (selected != ide_selected) {
        ide_selected = selected;
        session_changed();
        discard_old_rx();
    }
}
void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts) {
    (void)itf; (void)rts;
    cdc_line_coding_t coding;
    tud_cdc_get_line_coding(&coding);
    bool selected = ide_baudrate(coding.bit_rate);
    bool changed = dtr != line_dtr || selected != ide_selected;
    line_dtr = dtr;
    ide_selected = selected;
    if (changed) { session_changed(); }
    if (!dtr) {
        /* TX FIFO may still be owned by an IN transfer. Do not clear it here.
         * Its tail can drain on reopen; bus reset performs a complete reset. */
        discard_old_rx();
    }
    if (cdc_tx_waiting) { rt_sem_release(&cdc_tx_ready); }
}
void tud_cdc_tx_complete_cb(uint8_t itf) {
    (void)itf;
    if (cdc_tx_waiting) { rt_sem_release(&cdc_tx_ready); }
}
void tud_mount_cb(void) {
    line_dtr = false;
    serial_reset();
    session_changed();
}
void tud_umount_cb(void) {
    line_dtr = false;
    serial_reset();
    session_changed();
    if (cdc_tx_waiting) { rt_sem_release(&cdc_tx_ready); }
}
void tud_resume_cb(void) {
    if (cdc_tx_waiting) { rt_sem_release(&cdc_tx_ready); }
}
void tud_event_hook_cb(uint8_t rhport, uint32_t eventid, bool in_isr) {
    (void)in_isr;
    if (rhport != BOARD_DEVICE_RHPORT_NUM) { return; }
    if (eventid == DCD_EVENT_BUS_RESET_START || eventid == DCD_EVENT_BUS_RESET_END ||
        eventid == DCD_EVENT_UNPLUGGED) {
        ra8_usb_msc_reset();
        line_dtr = false;
        serial_reset();
        session_changed();
    }
}
void ra8_usb_session_poll(void) {
    /* VM polling owns protocol resets; USB callbacks only publish the epoch.
     * Apply the official reset before enabling the new transport session.
     * Preserve newly arrived RX bytes so the host can send SYNC normally. */
    const uint32_t epoch = session_epoch;
    if (applied_epoch == epoch) { return; }
    omv_protocol_reset();
    applied_epoch = epoch;
}
size_t ra8_usb_serial_write(const char *data, size_t size) {
    const uint32_t epoch = session_epoch;
    const uint32_t start = mp_hal_ticks_ms();
    size_t sent = 0;
    while (sent < size && epoch == session_epoch && serial_session()) {
        if (mp_hal_ticks_ms() - start >= 500) { break; }
        if (!tud_suspended()) {
            sent += tud_cdc_write(data + sent, size - sent);
            if (epoch == session_epoch && ra8_usb_serial_active()) { tud_cdc_write_flush(); }
        }
        if (sent == size) { break; }
        uint32_t elapsed = mp_hal_ticks_ms() - start;
        if (elapsed >= 500) { break; }
        uint32_t remaining = 500 - elapsed;
        usb_wait_tx(epoch, remaining < 10 ? remaining : 10, true, false);
    }
    return sent;
}
const omv_protocol_channel_t omv_usb_channel = {
    .id = OMV_PROTOCOL_CHANNEL_ID_TRANSPORT, .name = "usb",
    .flags = OMV_PROTOCOL_CHANNEL_FLAG_PHYSICAL, .size = usb_size,
    .read = usb_read, .write = usb_write, .flush = usb_flush, .is_active = usb_active
};
