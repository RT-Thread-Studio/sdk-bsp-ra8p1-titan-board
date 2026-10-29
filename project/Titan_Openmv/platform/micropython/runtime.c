#include "titan_ml_workspace.h"
#include "titan_lcd.h"
/* SPDX-License-Identifier: Apache-2.0 */
#include <rtthread.h>
#include <rtdevice.h>
#include <string.h>
#include "py/runtime.h"
#include "py/gc.h"
#include "py/stackctrl.h"
#include "py/mphal.h"
#include "extmod/vfs.h"
#include "extmod/vfs_posix.h"
#include "shared/runtime/pyexec.h"
#include "shared/runtime/gchelper.h"
#include "shared/runtime/interrupt_char.h"
#include "shared/readline/readline.h"
#include "shared/runtime/softtimer.h"
#include "omv_csi.h"
#include "omv_protocol.h"
#include "umalloc.h"
#include "ra8_memory.h"
#include "led.h"
#include "board_config.h"
#include "titan_storage.h"
#include "titan_vision.h"
#include "ra8_capture.h"
#include "machine_adc.h"
#include "machine_pwm.h"
#include "machine_uart.h"
#include "machine_spi.h"
#include "titan_protocol.h"
#include "ra8_vm_poll.h"
#include "ra8_usb_session.h"

/* CPU-only stack in DTCM. The VM limit is derived from this exact allocation. */
static uint8_t vm_stack[32 * 1024] __attribute__((section(".bss.dtcm"), aligned(8)));
static uint8_t fast_pool[80 * 1024] __attribute__((section(".bss.dtcm"), aligned(32)));
/* Keep tensor arenas in SRAM; large model files use HyperRAM GC. */
static uint8_t gc_heap[TITAN_OPENMV_SRAM_GC_BYTES] __attribute__((aligned(32)));
static uint8_t gc_heap_hyperram[2 * 1024 * 1024] __attribute__((section(".bss.omv_hyperram"), aligned(32)));
static uint8_t image_heap[8 * 1024 * 1024] __attribute__((section(".bss.omv_hyperram"), aligned(32)));
static uint8_t gc_heap_extra[RA8_GC_EXTRA_SIZE] __attribute__((section(".bss.omv_gc_extra"), aligned(32)));
static struct rt_thread vm_thread;
static const rt_base_t led_pins[] = OMV_BOARD_LED_PINS;
enum { LED_BOOT_VM = 1U, LED_BOOT_VISION = 2U };
static unsigned led_boot_pending, led_boot_failures;
static bool led_initialized, led_user_owned;

static void led_write(int led, int state) {
    rt_pin_write(led_pins[led], !!state ^ OMV_BOARD_LED_ACTIVE_LOW);
}
/* Call with interrupts masked: the vision worker and VM share these flags. */
static void led_boot_update(void) {
    if (!led_initialized || led_user_owned) { return; }
    int active = led_boot_failures ? LED_RED : led_boot_pending ? LED_BLUE : LED_GREEN;
    for (size_t i = 0; i < OMV_BOARD_LED_COUNT; ++i) { led_write(i, i == active); }
}
static void led_boot_complete(unsigned stage, bool ready) {
    rt_base_t level = rt_hw_interrupt_disable();
    led_boot_pending &= ~stage;
    if (ready) { led_boot_failures &= ~stage; }
    else { led_boot_failures |= stage; }
    led_boot_update();
    rt_hw_interrupt_enable(level);
}
void led_boot_vision(int ready) { led_boot_complete(LED_BOOT_VISION, ready != 0); }
static void led_boot_begin(void) {
    rt_base_t level = rt_hw_interrupt_disable();
    /* The verified vision image survives a soft reset; only restart the VM stage. */
    led_boot_pending |= LED_BOOT_VM;
    led_boot_failures &= ~LED_BOOT_VM;
    led_user_owned = false;
    led_boot_update();
    rt_hw_interrupt_enable(level);
}
static void led_boot_error(void) {
    rt_base_t level = rt_hw_interrupt_disable();
    led_boot_failures |= LED_BOOT_VM;
    /* Fatal/startup errors take precedence even after a script used the LEDs. */
    led_user_owned = false;
    led_boot_update();
    rt_hw_interrupt_enable(level);
}
void led_init(void) {
    for (size_t i = 0; i < OMV_BOARD_LED_COUNT; ++i) {
        rt_pin_mode(led_pins[i], PIN_MODE_OUTPUT);
        led_write(i, 0);
    }
    led_initialized = true;
    led_boot_pending = LED_BOOT_VM | LED_BOOT_VISION;
    led_boot_failures = 0;
    led_boot_begin();
}
void led_state(int led, int state) {
    if (led < 0 || led >= OMV_BOARD_LED_COUNT) { return; }
    rt_base_t level = rt_hw_interrupt_disable();
    if (!led_user_owned) {
        /* A script takes over the whole RGB indicator until the next soft reset. */
        for (size_t i = 0; i < OMV_BOARD_LED_COUNT; ++i) { led_write(i, 0); }
        led_user_owned = true;
    }
    led_write(led, state);
    rt_hw_interrupt_enable(level);
}

/* Enumeration and the base VM do not wait for storage initialization or resources.
 * Provisioning/reads/CRC run in a worker below both USB and the VM. */
static void omv_bootstrap(void) {
    int result = titan_vision_start();
    if (result != RT_EOK) {
        led_boot_vision(false);
        rt_kprintf("Vision loader initialization failed: %d; basic Python remains available\n", result);
    }
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

/* Preserve boot.py/main.py ordering on the selected medium: run them only
 * after the asset is verified. The official IDE protocol remains serviced
 * while waiting. EXEC/STOP can interrupt this wait just like the REPL; never
 * execute an incompletely uploaded STDIN buffer during protocol polling.
 * An interactive serial REPL can take over while resources are missing. */
static bool omv_autorun_ready(void) {
    bool ready = false;
    nlr_buf_t nlr;
    if (nlr_push(&nlr) == 0) {
        mp_hal_set_interrupt_char(CHAR_CTRL_C);
        while (!titan_vision_is_ready() && titan_vision_needs_service() &&
               !ra8_usb_serial_active()) {
            mp_hal_poll();
            mp_handle_pending(true);
            rt_thread_mdelay(1);
        }
        ready = titan_vision_is_ready();
        nlr_pop();
    }
    ra8_vm_poll_recover();
    return ready;
}

void gc_collect(void) {
    gc_collect_start();
    gc_helper_collect_regs_and_stack();
    titan_protocol_gc_collect();
    soft_timer_gc_mark_all();
    gc_collect_end();
}
void nlr_jump_fail(void *value) { led_boot_error(); rt_kprintf("OpenMV fatal exception: %p\n", value); rt_hw_cpu_shutdown(); for (;;) {} }
void __fatal_error(const char *text) { led_boot_error(); rt_kprintf("OpenMV fatal: %s\n", text); rt_hw_cpu_shutdown(); for (;;) {} }

static void omv_entry(void *arg) {
    (void)arg;
    omv_bootstrap();
    uint32_t stack_top;
    mp_stack_set_top(&stack_top);
    mp_stack_set_limit(sizeof(vm_stack) - 2048);
    bool first_reset = true;
    for (;;) {
        led_boot_begin();
        titan_ml_workspace_auto_reset();
        gc_init(gc_heap, gc_heap + sizeof(gc_heap));
        gc_add(gc_heap_hyperram, gc_heap_hyperram + sizeof(gc_heap_hyperram));
        gc_add(gc_heap_extra, gc_heap_extra + sizeof(gc_heap_extra));
        mp_init();
        uma_init();
        uma_pool_add(image_heap, sizeof(image_heap), 0);
        uma_pool_add(fast_pool, sizeof(fast_pool), UMA_FAST | UMA_DTCM);
        framebuffer_init0();
        mp_obj_t vfs = mp_call_function_0(MP_OBJ_FROM_PTR(&mp_type_vfs_posix));
        mp_obj_t mount_args[] = {vfs, MP_OBJ_NEW_QSTR(MP_QSTR__slash_)};
        mp_vfs_mount(2, mount_args, (mp_map_t *)&mp_const_empty_map);
        /* RT-Thread mounts the selected FAT filesystem at /. Examples use /rom, so
         * provide a read-only alias of that same filesystem root. Both /model.tflite
         * and /rom/model.tflite resolve to the root-level uploaded model file.
         * The alias shares the same contents: VfsPosix resolves paths on each access.
         */
        mp_obj_t rom_path = mp_obj_new_str("/rom", 4);
        mp_obj_t rom_vfs = mp_call_function_0(MP_OBJ_FROM_PTR(&mp_type_vfs_posix));
        mp_obj_t rom_mount_args[] = {rom_vfs, rom_path};
        const mp_obj_t rom_kw_table[] = {MP_OBJ_NEW_QSTR(MP_QSTR_readonly), mp_const_true};
        mp_map_t rom_kw;
        mp_map_init_fixed_table(&rom_kw, 1, rom_kw_table);
        mp_vfs_mount(2, rom_mount_args, &rom_kw);
        mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR__slash_));
        mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR__slash_lib));
        int sensor_result = omv_csi_init();
        if (sensor_result != 0) {
            led_boot_error();
            rt_kprintf("Camera initialization failed: %d\n", sensor_result);
        }
        ra8_vm_poll_start();
        int protocol_result = titan_protocol_init_default();
        if (protocol_result != 0) {
            led_boot_error();
            rt_kprintf("OpenMV protocol initialization failed: %d\n", protocol_result);
            goto vm_cleanup;
        }
        led_boot_complete(LED_BOOT_VM, sensor_result == 0);
        /* Match the upstream runtime: execute IDE scripts outside stdin/REPL,
         * catch the interrupt used to leave the REPL, and don't restart an
         * autorun main.py after each IDE script or Ctrl-D soft reset.
         */
        bool boot_ready = titan_vision_is_ready();
        if (first_reset && !boot_ready) { boot_ready = omv_autorun_ready(); }
        if (boot_ready && pyexec_file_if_exists("/boot.py") && first_reset) {
            pyexec_file_if_exists("/main.py");
        }
        ra8_vm_poll_recover();
        while (!omv_protocol_exec_script()) {
            nlr_buf_t nlr;
            if (nlr_push(&nlr) == 0) {
                mp_hal_set_interrupt_char(CHAR_CTRL_C);
                int result = pyexec_mode_kind == PYEXEC_MODE_RAW_REPL ? pyexec_raw_repl() : pyexec_friendly_repl();
                nlr_pop();
                if (result != 0) { break; }
            }
            ra8_vm_poll_recover();
        }
        first_reset = false;
vm_cleanup:
        mp_hal_set_interrupt_char(-1);
        ra8_vm_poll_stop();
        titan_display_deinit_all();
        omv_csi_abort_all();
        if (ra8_capture_reset_barrier() != 0) {
            led_boot_error();
            rt_kprintf("OpenMV capture drain failed; resetting before heap reuse\n");
            NVIC_SystemReset();
            for (;;) {}
        }
        extern void ra8_timers_deinit_all(void);
        extern void ra8_pin_irqs_deinit_all(void);
        ra8_timers_deinit_all();
        ra8_pin_irqs_deinit_all();
        ra8_machine_uart_deinit_all();
        ra8_machine_spi_deinit_all();
        machine_adc_deinit_all();
        machine_pwm_deinit_all();
        omv_protocol_deinit();
        soft_timer_deinit();
        /* Hardware callbacks and protocol timers are stopped above. Finalise
         * Python files/models while their GC objects and UMA pools still exist,
         * before the next gc_init/uma_init discards the old VM heap.
         */
        gc_sweep_all();
        mp_deinit();
        if (protocol_result != 0) { return; }
    }
}
void hal_entry(void) {
    extern int tusb_board_init(void);
    extern int ra8_usb_start(void);
    led_init();
    if (tusb_board_init() != RT_EOK) {
        led_boot_error();
        rt_kprintf("OpenMV USB board initialization failed\n");
        return;
    }
    if (ra8_usb_start() != RT_EOK) {
        led_boot_error();
        rt_kprintf("TinyUSB initialization failed\n");
        return;
    }
    /* Return promptly so enumeration and resource provisioning can progress. */
    rt_err_t result = rt_thread_init(&vm_thread, "openmv", omv_entry, NULL,
                                    vm_stack, sizeof(vm_stack), 20, 10);
    if (result == RT_EOK) { result = rt_thread_startup(&vm_thread); }
    if (result != RT_EOK) {
        led_boot_error();
        rt_kprintf("OpenMV thread initialization failed: %d\n", result);
    }
}
