/* SPDX-License-Identifier: Apache-2.0 */
#include "py/mpconfig.h"
#if MICROPY_PY_DISPLAY
#include <string.h>
#include <rtthread.h>
#include "py/mperrno.h"
#include "hal_data.h"
#include "titan_lcd.h"
#include "machine_adc.h"
#include "machine_pwm.h"
#include "machine_uart.h"
#include "machine_spi.h"
#include "modmachine.h"
#include "ra8_pin_irq.h"

/* Keep the RASC output as the sole panel timing and pin configuration source.
 * The SDK LCD/G2D driver must not acquire the same GLCDC/PWM instances. */
#ifdef BSP_USING_LCD
#error "OpenMV RGBDisplay owns GLCDC; disable the SDK LCD driver"
#endif
_Static_assert(GLCDC_CFG_LAYER_1_ENABLE && !GLCDC_CFG_LAYER_2_ENABLE,
               "RGBDisplay requires one GLCDC graphics layer");
_Static_assert(DISPLAY_HSIZE_INPUT0 == TITAN_LCD_WIDTH && DISPLAY_VSIZE_INPUT0 == TITAN_LCD_HEIGHT &&
               DISPLAY_BITS_PER_PIXEL_INPUT0 == 16 && DISPLAY_BUFFER_STRIDE_PIXELS_INPUT0 == TITAN_LCD_WIDTH,
               "RGBDisplay requires the RASC 800x480 RGB565 framebuffer layout");
_Static_assert(sizeof(fb_background) == 2U * TITAN_LCD_WIDTH * TITAN_LCD_HEIGHT * 2U,
               "RGBDisplay requires exactly two RASC framebuffers");

#define LCD_TIMEOUT_MS 100U
#define LCD_NO_PENDING 2U
#define LCD_BACKLIGHT_PIN BSP_IO_PORT_03_PIN_03
#define LCD_RESET_PIN BSP_IO_PORT_00_PIN_11

/* All RGB pins, reset, and backlight. Restore the caller's pin mux on close. */
static const uint16_t lcd_pins[] = {
    BSP_IO_PORT_09_PIN_14, BSP_IO_PORT_09_PIN_15, BSP_IO_PORT_09_PIN_03, BSP_IO_PORT_09_PIN_02,
    BSP_IO_PORT_09_PIN_10, BSP_IO_PORT_09_PIN_11, BSP_IO_PORT_09_PIN_12, BSP_IO_PORT_09_PIN_13,
    BSP_IO_PORT_09_PIN_04, BSP_IO_PORT_02_PIN_07, BSP_IO_PORT_11_PIN_07, BSP_IO_PORT_11_PIN_06,
    BSP_IO_PORT_11_PIN_05, BSP_IO_PORT_11_PIN_01, BSP_IO_PORT_11_PIN_04, BSP_IO_PORT_11_PIN_03,
    BSP_IO_PORT_05_PIN_15, BSP_IO_PORT_08_PIN_06, BSP_IO_PORT_08_PIN_05, BSP_IO_PORT_05_PIN_13,
    LCD_RESET_PIN, LCD_BACKLIGHT_PIN,
};
static uint32_t saved_pins[sizeof(lcd_pins) / sizeof(lcd_pins[0])];
static bool reserved;
static bool opened;
static bool pins_saved;
static bool pwm_was_open;
static bool pwm_acquired;
static unsigned intensity;
static volatile unsigned front;
static volatile unsigned pending = LCD_NO_PENDING;
static volatile uint32_t completed;
static volatile uint32_t underflow_frames;

static bool expired(rt_tick_t start) {
    return (rt_tick_t)(rt_tick_get() - start) >= rt_tick_from_millisecond(LCD_TIMEOUT_MS);
}

bool titan_lcd_pin_owned(uint16_t pin) {
    if (!reserved) { return false; }
    for (unsigned i = 0; i < sizeof(lcd_pins) / sizeof(lcd_pins[0]); ++i) {
        if (lcd_pins[i] == pin) { return true; }
    }
    return false;
}

bool titan_lcd_timer_owned(unsigned channel) { return reserved && channel == 7; }
bool titan_lcd_is_open(void) { return opened; }
unsigned titan_lcd_get_backlight(void) { return intensity; }

static int pin_config(uint16_t pin, uint32_t cfg) {
    return R_IOPORT_PinCfg(&g_ioport_ctrl, (bsp_io_port_pin_t)pin, cfg) == FSP_SUCCESS ? 0 : -MP_EIO;
}

static int restore_pins(void) {
    int result = 0;
    if (pins_saved) {
        for (unsigned i = 0; i < sizeof(lcd_pins) / sizeof(lcd_pins[0]); ++i) {
            if (pin_config(lcd_pins[i], saved_pins[i])) { result = -MP_EIO; }
        }
        if (!result) { pins_saved = false; }
    }
    return result;
}

/* Called with CPU interrupts masked, or from the GLCDC IRQ. The line IRQ is
 * at the end of the active area, not necessarily the Vsync that latches FLM2.
 * PVEN clearing is the proof that the old front buffer is no longer scanned. */
static void observe_latch(void) {
    if (opened && pending != LCD_NO_PENDING && !R_GLCDC->GR[0].VEN_b.PVEN) {
        front = pending;
        pending = LCD_NO_PENDING;
        completed++;
    }
}

void DisplayVsyncCallback(display_callback_args_t *args) {
    if (args->event == DISPLAY_EVENT_LINE_DETECTION) {
        observe_latch();
        if (R_GLCDC->SYSCNT.STMON_b.L1UNDF) {
            underflow_frames++;
            R_GLCDC->SYSCNT.STCLR_b.L1UNDFCLR = 1U;
        }
    }
}

void titan_lcd_line_isr(void) {
    rt_interrupt_enter();
    glcdc_line_detect_isr();
    rt_interrupt_leave();
}

static int wait_latch(void) {
    rt_tick_t start = rt_tick_get();
    for (;;) {
        rt_base_t level = rt_hw_interrupt_disable();
        observe_latch();
        bool ready = pending == LCD_NO_PENDING;
        rt_hw_interrupt_enable(level);
        if (ready) { return 0; }
        if (expired(start)) { return -MP_ETIMEDOUT; }
        rt_thread_mdelay(1);
    }
}


int titan_lcd_set_backlight(unsigned percent) {
    if (!opened || !pwm_acquired) { return -MP_ENODEV; }
    if (percent > 100U) { return -MP_EINVAL; }
    timer_info_t info;
    if (R_GPT_InfoGet(&g_timer7_ctrl, &info) != FSP_SUCCESS) { return -MP_EIO; }
    uint32_t counts = (uint32_t)(((uint64_t)info.period_counts * percent) / 100U);
    if (R_GPT_DutyCycleSet(&g_timer7_ctrl, counts, GPT_IO_PIN_GTIOCB) != FSP_SUCCESS) { return -MP_EIO; }
    fsp_err_t error = percent ? R_GPT_Start(&g_timer7_ctrl) : R_GPT_Stop(&g_timer7_ctrl);
    if (error != FSP_SUCCESS) { return -MP_EIO; }
    /* A stopped counter can leave a PWM output at its previous level. */
    if (pin_config(LCD_BACKLIGHT_PIN, percent ? (IOPORT_CFG_PERIPHERAL_PIN | IOPORT_PERIPHERAL_GPT1) :
                   (IOPORT_CFG_PORT_DIRECTION_OUTPUT | IOPORT_CFG_PORT_OUTPUT_LOW))) { return -MP_EIO; }
    intensity = percent;
    return 0;
}

static int release_pwm(void) {
    if (!pwm_acquired) { return 0; }
    if (g_timer7_ctrl.open) {
        if (R_GPT_Stop(&g_timer7_ctrl) != FSP_SUCCESS || R_GPT_Close(&g_timer7_ctrl) != FSP_SUCCESS) {
            return -MP_EIO;
        }
    }
    if (pwm_was_open && R_GPT_Open(&g_timer7_ctrl, &g_timer7_cfg) != FSP_SUCCESS) { return -MP_EIO; }
    pwm_acquired = false;
    return 0;
}

int titan_lcd_close(void) {
    if (!reserved) { return 0; }
    if (opened) {
        int result = titan_lcd_set_backlight(0);
        if (result) { return result; }
        result = wait_latch();
        if (result) { return result; }
    }
    rt_tick_t start = rt_tick_get();
    while (g_display0_ctrl.state == DISPLAY_STATE_DISPLAYING) {
        fsp_err_t error = R_GLCDC_Stop(&g_display0_ctrl);
        if (error == FSP_SUCCESS) { break; }
        if (error != FSP_ERR_INVALID_UPDATE_TIMING) { return -MP_EIO; }
        if (expired(start)) { return -MP_ETIMEDOUT; }
        rt_thread_mdelay(1);
    }
    while (g_display0_ctrl.state != DISPLAY_STATE_CLOSED) {
        fsp_err_t error = R_GLCDC_Close(&g_display0_ctrl);
        if (error == FSP_SUCCESS) { break; }
        if (error != FSP_ERR_INVALID_UPDATE_TIMING) { return -MP_EIO; }
        if (expired(start)) { return -MP_ETIMEDOUT; }
        rt_thread_mdelay(1);
    }
    opened = false;
    pending = LCD_NO_PENDING;
    if (g_display0_cfg.line_detect_irq >= 0) {
        R_BSP_IrqDisable(g_display0_cfg.line_detect_irq);
        R_BSP_IrqStatusClear(g_display0_cfg.line_detect_irq);
        NVIC_ClearPendingIRQ(g_display0_cfg.line_detect_irq);
    }
    int result = release_pwm();
    if (!result) { result = restore_pins(); }
    if (result) { return result; }
    intensity = 0;
    reserved = false;
    return 0;
}

void titan_lcd_deinit_all(void) {
    if (titan_lcd_close() != 0) {
        /* Never hand GPIO/PWM ownership back while scanout might still run. */
        rt_kprintf("LCD shutdown failed; resetting before VM heap reuse\n");
        NVIC_SystemReset();
        for (;;) {}
    }
}

int titan_lcd_open(void) {
    if (reserved || g_display0_ctrl.state != DISPLAY_STATE_CLOSED) { return -MP_EBUSY; }
    if (!g_ioport_ctrl.open) { return -MP_ENODEV; }
    for (unsigned i = 0; i < sizeof(lcd_pins) / sizeof(lcd_pins[0]); ++i) {
        unsigned pin = lcd_pins[i];
        if (ra8_machine_pin_irq_owned(pin) || ra8_adc_pin_owned(pin) || ra8_pwm_pin_owned(pin) ||
            ra8_uart_pin_owned(pin) || ra8_spi_pin_owned(pin)) { return -MP_EBUSY; }
    }
    pwm_was_open = g_timer7_ctrl.open != 0;
    if (pwm_was_open) {
        timer_status_t status;
        if (R_GPT_StatusGet(&g_timer7_ctrl, &status) != FSP_SUCCESS) { return -MP_EIO; }
        if (status.state == TIMER_STATE_COUNTING || g_timer7_ctrl.p_cfg != &g_timer7_cfg) { return -MP_EBUSY; }
    }
    reserved = true;
    for (unsigned i = 0; i < sizeof(lcd_pins) / sizeof(lcd_pins[0]); ++i) {
        uint16_t pin = lcd_pins[i];
        saved_pins[i] = R_PFS->PORT[pin >> 8].PIN[pin & 15].PmnPFS & ~R_PFS_PORT_PIN_PmnPFS_PIDR_Msk;
    }
    pins_saved = true;
    int result = 0;
    for (unsigned i = 0; i < sizeof(lcd_pins) / sizeof(lcd_pins[0]); ++i) {
        /* P011 is the panel reset GPIO used by the SDK board example; unlike
         * RGB/PWM pins it is intentionally absent from the RASC pin table. */
        if (lcd_pins[i] == LCD_RESET_PIN) {
            result = pin_config(LCD_RESET_PIN, IOPORT_CFG_PORT_DIRECTION_OUTPUT | IOPORT_CFG_PORT_OUTPUT_LOW);
            if (result) { goto fail; }
            continue;
        }
        bool found = false;
        for (unsigned j = 0; j < g_bsp_pin_cfg.number_of_pins; ++j) {
            if (g_bsp_pin_cfg.p_pin_cfg_data[j].pin == lcd_pins[i]) {
                result = pin_config(lcd_pins[i], g_bsp_pin_cfg.p_pin_cfg_data[j].pin_cfg);
                found = true;
                break;
            }
        }
        if (!found) { result = -MP_ENODEV; }
        if (result) { goto fail; }
    }
    if (!pwm_was_open && R_GPT_Open(&g_timer7_ctrl, &g_timer7_cfg) != FSP_SUCCESS) { result = -MP_EIO; goto fail; }
    pwm_acquired = true;
    if (R_GPT_DutyCycleSet(&g_timer7_ctrl, 0, GPT_IO_PIN_GTIOCB) != FSP_SUCCESS ||
        R_GPT_Stop(&g_timer7_ctrl) != FSP_SUCCESS) { result = -MP_EIO; goto fail; }
    if (pin_config(LCD_BACKLIGHT_PIN, IOPORT_CFG_PORT_DIRECTION_OUTPUT | IOPORT_CFG_PORT_OUTPUT_LOW)) {
        result = -MP_EIO; goto fail;
    }
    if (pin_config(LCD_RESET_PIN, IOPORT_CFG_PORT_DIRECTION_OUTPUT | IOPORT_CFG_PORT_OUTPUT_LOW)) {
        result = -MP_EIO; goto fail;
    }
    rt_thread_mdelay(10);
    if (R_IOPORT_PinWrite(&g_ioport_ctrl, LCD_RESET_PIN, BSP_IO_LEVEL_HIGH) != FSP_SUCCESS) {
        result = -MP_EIO; goto fail;
    }
    rt_thread_mdelay(20);
    memset(fb_background, 0, sizeof(fb_background));
    /* FSP maps this .hyperram_noinit_nocache range as non-cacheable at startup. */
    __DSB();
    front = 0;
    pending = LCD_NO_PENDING;
    completed = 0;
    underflow_frames = 0;
    if (R_GLCDC_Open(&g_display0_ctrl, &g_display0_cfg) != FSP_SUCCESS) { result = -MP_EIO; goto fail; }
    opened = true;
    if (R_GLCDC_Start(&g_display0_ctrl) != FSP_SUCCESS) { result = -MP_EIO; goto fail; }
    intensity = 0;
    return 0;
fail:
    /* Static scanout buffers survive error unwinding; retain ownership if
     * shutdown fails, and let the VM-reset barrier retry or reset the MCU. */
    (void)titan_lcd_close();
    return result;
}

uint16_t *titan_lcd_draw_buffer(void) {
    if (!opened || pending != LCD_NO_PENDING) { return NULL; }
    return (uint16_t *)fb_background[front ^ 1U];
}

int titan_lcd_present(void) {
    if (!opened) { return -MP_ENODEV; }
    if (pending != LCD_NO_PENDING) { return -MP_EBUSY; }
    __DSB();
    rt_tick_t start = rt_tick_get();
    for (;;) {
        rt_base_t level = rt_hw_interrupt_disable();
        unsigned back = front ^ 1U;
        fsp_err_t error = R_GLCDC_BufferChange(&g_display0_ctrl, fb_background[back], DISPLAY_FRAME_LAYER_1);
        if (error == FSP_SUCCESS) { pending = back; }
        rt_hw_interrupt_enable(level);
        if (error == FSP_SUCCESS) { return wait_latch(); }
        if (error != FSP_ERR_INVALID_UPDATE_TIMING) { return -MP_EIO; }
        if (expired(start)) { return -MP_ETIMEDOUT; }
        rt_thread_mdelay(1);
    }
}


#else
#include <stddef.h>
#include <errno.h>
#include "titan_lcd.h"
int titan_lcd_open(void) { return -ENOSYS; }
int titan_lcd_close(void) { return 0; }
bool titan_lcd_is_open(void) { return false; }
uint16_t *titan_lcd_draw_buffer(void) { return NULL; }
int titan_lcd_present(void) { return -ENOSYS; }
int titan_lcd_set_backlight(unsigned value) { (void)value; return -ENOSYS; }
unsigned titan_lcd_get_backlight(void) { return 0; }
void titan_lcd_deinit_all(void) {}
bool titan_lcd_pin_owned(uint16_t pin) { (void)pin; return false; }
bool titan_lcd_timer_owned(unsigned channel) { (void)channel; return false; }
void titan_lcd_line_isr(void) {}
#endif
