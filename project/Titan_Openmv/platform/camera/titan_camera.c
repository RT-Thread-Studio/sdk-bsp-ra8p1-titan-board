/* SPDX-License-Identifier: Apache-2.0 */
#include <string.h>
#include <rtthread.h>
#include <rtdevice.h>
#include "hal_data.h"
#include "common_data.h"
#include "titan_camera.h"
#include "titan_memory.h"

typedef struct { uint16_t reg; uint8_t value; } titan_sensor_reg_t;
#include "ov5640_mipi_regs.h"

/* Three VIN banks point directly to OpenMV-owned buffers. The sink owns
 * the free/ready/published queues; this layer only owns armed DMA addresses. */
static titan_camera_sink_t capture_sink;
static uint8_t *dma_banks[3];
static size_t dma_capacity;
static volatile uint32_t sequence, faults;
static volatile titan_camera_status_t capture_status;
static struct rt_completion frame_done;
static vin_instance_ctrl_t vin;
static capture_cfg_t capture_cfg;
static vin_extended_cfg_t vin_cfg;
static bool opened, streaming, clock_started;
/* R_VIN_Close gates the shared capture module even when returning an error. */
static bool module_stopped;

#define VIN_ERROR_MASK (R_VIN_INTS_FOS_Msk | R_VIN_INTS_ARES_Msk | \
    R_VIN_INTS_PRCLIPHES_Msk | R_VIN_INTS_PRCLIPVES_Msk | R_VIN_INTS_ROS_Msk)
#define CSI_LANE_ERROR_MASK (R_MIPI_CSI_DLST0_ESH_Msk | R_MIPI_CSI_DLST0_ESS_Msk | \
    R_MIPI_CSI_DLST0_ECT_Msk | R_MIPI_CSI_DLST0_EES_Msk)
#define CSI_VC_ERROR_MASK (R_MIPI_CSI_VCST0_MLF_Msk | R_MIPI_CSI_VCST0_ECD_Msk | \
    R_MIPI_CSI_VCST0_CRC_Msk | R_MIPI_CSI_VCST0_IDE_Msk | R_MIPI_CSI_VCST0_WCE_Msk | \
    R_MIPI_CSI_VCST0_FRS_Msk | R_MIPI_CSI_VCST0_FRD_Msk | R_MIPI_CSI_VCST0_OVF_Msk)

static int sensor_write(uint16_t reg, uint8_t value) {
    struct rt_i2c_bus_device *bus = (struct rt_i2c_bus_device *)rt_device_find(TITAN_CAMERA_I2C_BUS);
    uint8_t bytes[] = {reg >> 8, reg, value};
    struct rt_i2c_msg msg = {.addr = 0x3c, .flags = RT_I2C_WR, .buf = bytes, .len = sizeof(bytes)};
    if (bus && rt_i2c_transfer(bus, &msg, 1) == 1) { return 0; }
    rt_kprintf("[titan.csi] sensor write failed: reg=0x%04x value=0x%02x\n", reg, value);
    return -1;
}

static int sensor_read(uint16_t reg, uint8_t *value) {
    struct rt_i2c_bus_device *bus = (struct rt_i2c_bus_device *)rt_device_find(TITAN_CAMERA_I2C_BUS);
    uint8_t bytes[] = {reg >> 8, reg};
    struct rt_i2c_msg msgs[] = {
        {.addr = 0x3c, .flags = RT_I2C_WR | RT_I2C_NO_STOP, .buf = bytes, .len = sizeof(bytes)},
        {.addr = 0x3c, .flags = RT_I2C_RD, .buf = value, .len = 1}
    };
    return bus && rt_i2c_transfer(bus, msgs, 2) == 2 ? 0 : -1;
}

int titan_camera_clock_start(uint32_t frequency) {
    if (frequency != 25000000U) { return -1; }
    if (!clock_started) {
        fsp_err_t err = R_GPT_Open(&g_timer6_ctrl, &g_timer6_cfg);
        if (err != FSP_SUCCESS && err != FSP_ERR_ALREADY_OPEN) { return -1; }
    }
    timer_info_t info;
    if (R_GPT_InfoGet(&g_timer6_ctrl, &info) != FSP_SUCCESS ||
        !info.period_counts || g_timer6_cfg.mode != TIMER_MODE_PWM ||
        (uint64_t)frequency * info.period_counts != info.clock_frequency) {
        rt_kprintf("[titan.csi] GPT6A XCLK configuration does not match 25 MHz\n");
        return -1;
    }
    if (clock_started) { return 0; }
    if (R_GPT_Enable(&g_timer6_ctrl) || R_GPT_Start(&g_timer6_ctrl)) { return -1; }
    clock_started = true;
    return 0;
}

int titan_camera_sensor_reset(void) {
    if (titan_camera_close()) { return -1; }
    for (const titan_sensor_reg_t *p = titan_ov5640_mipi_regs; p->reg != CONFIG_TABLE_END_DETECT; p++) {
        if (p->reg == REQUEST_SOFTWARE_WAIT) { rt_thread_mdelay(p->value); }
        else if (sensor_write(p->reg, p->value)) { return -1; }
    }
    /* Keep the board's actual 25 MHz GPT6A output and tested two-lane PHY timing.
     * This sequence intentionally replaces the parallel-port upstream PLL.
     */
    static const titan_sensor_reg_t clocks[] = {
        {0x3035, 0x12}, {0x3036, 140}, {0x3037, 0x13}, {0x3108, 0x01},
        {0x3406, 0x00}, {0x4202, 0x0f},
        /* Keep the board-tested MIPI sensor/ISP phase. Changing this to the
         * parallel driver's 0x47 fixes geometry but produces a magenta RGB
         * image on this module. ra8_csi normalizes completed pixels instead;
         * logical hmirror/vflip controls never rewrite these sensor bits. */
        {0x3820, 0x41}, {0x3821, 0x01}
    };
    for (size_t i = 0; i < sizeof(clocks) / sizeof(clocks[0]); i++) {
        if (sensor_write(clocks[i].reg, clocks[i].value)) { return -1; }
    }
    /* Match the working SDK's ov5640_mipi_virtual_channel_set(): only bits
     * [7:6] select VC. Preserve the remaining MIPI control bits.
     */
    const vin_extended_cfg_t *cfg = (const vin_extended_cfg_t *)g_cam_vin_cfg.p_extend;
    uint8_t mipi_control;
    if (sensor_read(0x4814, &mipi_control)) {
        rt_kprintf("[titan.csi] sensor read failed: reg=0x4814\n");
        return -1;
    }
    mipi_control = (mipi_control & 0x3fU) | (cfg->input_ctrl.csi_mode_bits.virtual_channel << 6);
    if (sensor_write(0x4814, mipi_control)) { return -1; }
    rt_thread_mdelay(5);
    return 0;
}

int titan_camera_sensor_size(uint16_t width, uint16_t height) {
    if (!width || !height || width > 640 || height > 480 || (width & 1U)) { return -1; }
    return sensor_write(0x3808, width >> 8) || sensor_write(0x3809, width) ||
           sensor_write(0x380a, height >> 8) || sensor_write(0x380b, height) ? -1 : 0;
}

void titan_camera_quiesce_irq(void) {
    if (opened && !module_stopped) {
        R_VIN->FC_b.CC = 0;
        R_VIN->MC_b.ME = 0;
        __DSB();
    }
    streaming = false;
}

static void camera_drain_error(const char *phase, fsp_err_t status_result) {
    rt_kprintf("[titan.csi] drain failed: %s (%u)\n", phase, (unsigned)status_result);
}
int titan_camera_close(void) {
    if (!opened) { return 0; }
    if (module_stopped) { return -1; }
    /* Keep sensor frame endings available while VIN stops. MA is not an
     * outstanding AXI count; use the same CA/OUTSTAND predicates as FSP. */
    titan_camera_quiesce_irq();
    uint32_t start = rt_tick_get_millisecond();
    while (R_VIN->MS_b.CA || R_VIN->MTCSTOP_b.OUTSTAND) {
        if ((uint32_t)(rt_tick_get_millisecond() - start) >= 1000U) {
            faults |= 0x80000000U;
            camera_drain_error("VIN", FSP_ERR_IN_USE);
            return -1;
        }
        rt_thread_mdelay(1);
    }
    if (sensor_write(0x4202, 0x0f)) {
        camera_drain_error("sensor-stream-off", FSP_ERR_ABORTED);
        return -1;
    }
    /* StatusGet reads and clears the sticky RX activity detector. */
    mipi_csi_status_t status = {0};
    fsp_err_t result = R_MIPI_CSI_StatusGet(&g_cam_mipi_csi_ctrl, &status);
    if (result != FSP_SUCCESS) { camera_drain_error("CSI-status", result); return -1; }
    start = rt_tick_get_millisecond();
    do {
        rt_thread_mdelay(100);
        result = R_MIPI_CSI_StatusGet(&g_cam_mipi_csi_ctrl, &status);
        if (result != FSP_SUCCESS) { camera_drain_error("CSI-status", result); return -1; }
        if (status.state == MIPI_CSI_STATE_IDLE && !R_MIPI_CSI->RXST_b.RACT) { break; }
    } while ((uint32_t)(rt_tick_get_millisecond() - start) < 1000U);
    if (status.state != MIPI_CSI_STATE_IDLE || R_MIPI_CSI->RXST_b.RACT ||
        R_VIN->MS_b.CA || R_VIN->MTCSTOP_b.OUTSTAND) {
        faults |= 0x80000000U;
        camera_drain_error("CSI-quiet", result);
        return -1;
    }
    /* All hardware drain checks are above. FSP disables IRQs and gates the
     * shared module before returning; VIN MMIO is no longer valid afterward,
     * including when Close returns an error. Retain owners on a real error. */
    module_stopped = true;
    result = R_VIN_Close(&vin);
    if (result != FSP_SUCCESS) {
        rt_kprintf("[titan.csi] FSP close failed status=%u (module stopped)\n", (unsigned)result);
        return -1;
    }
    opened = false;
    memset(dma_banks, 0, sizeof(dma_banks));
    memset(&capture_sink, 0, sizeof(capture_sink));
    return 0;
}

int titan_camera_open(const titan_camera_config_t *config, const titan_camera_sink_t *sink, size_t capacity) {
    if (!config || !sink || !sink->take_buffer || !sink->complete_buffer || !config->width || !config->height ||
        config->x + config->width > 640 || config->y + config->height > 480 ||
        ((config->x | config->width) & 1U)) { return -1; }
    if (titan_camera_close()) { return -1; }
    size_t required = (((size_t)config->width + 15U) & ~(size_t)15U) * config->height * 2U;
    if (capacity < required) { return -1; }
    capture_sink = *sink;
    dma_capacity = capacity;
    for (unsigned i = 0; i < 3; i++) {
        uint8_t *data = capture_sink.take_buffer(capture_sink.context);
        if (!data || ((uintptr_t)data & 127U) ||
            !titan_memory_is_bus_accessible(data, dma_capacity)) { return -1; }
        for (unsigned j = 0; j < i; j++) { if (data == dma_banks[j]) { return -1; } }
        dma_banks[i] = data;
    }
    capture_cfg = g_cam_vin_cfg;
    vin_cfg = *(const vin_extended_cfg_t *)g_cam_vin_cfg.p_extend;
    capture_cfg.p_extend = &vin_cfg;
    capture_cfg.p_callback = cam_vin_callback;
    capture_cfg.p_context = NULL;
    vin_cfg.input_ctrl.preclip.pixel_start = config->x;
    vin_cfg.input_ctrl.preclip.pixel_end = config->x + config->width - 1;
    vin_cfg.input_ctrl.preclip.line_start = config->y;
    vin_cfg.input_ctrl.preclip.line_end = config->y + config->height - 1;
    /* VIN stride is pixels, rounded to a 32-byte write boundary for 16bpp. */
    vin_cfg.input_ctrl.image_stride = (config->width + 15U) & ~15U;
    vin_cfg.input_ctrl.cfg_bits.color_space_convert_bypass = !config->rgb565;
    /* OpenMV expects RGB565 little-endian and YUV422 byte order Y0 U Y1 V.
     * Without BPS, the VIN 16-bit Y/C words arrive as U Y0 V Y1 in RAM;
     * selecting every even byte would display chroma stripes as grayscale.
     */
    vin_cfg.conversion_ctrl.data_mode_bits.output_data_byte_swap = true;
    vin_cfg.conversion_data.uds_clipping_bits.cl_hsize = config->width;
    vin_cfg.conversion_data.uds_clipping_bits.cl_vsize = config->height;
    vin_cfg.interrupt_cfg.status_enable_mask |= R_VIN_IE_FOE_Msk | R_VIN_IE_PRCLIPHEE_Msk | R_VIN_IE_PRCLIPVEE_Msk | R_VIN_IE_ROE_Msk | R_VIN_IE_AREE_Msk;
    vin_cfg.output_ctrl.use_runtime_buffer = false;
    vin_cfg.output_ctrl.image_buffer[0] = dma_banks[0];
    vin_cfg.output_ctrl.image_buffer[1] = dma_banks[1];
    vin_cfg.output_ctrl.image_buffer[2] = dma_banks[2];
    sequence = faults = 0;
    capture_status = (titan_camera_status_t){0};
    rt_completion_init(&frame_done);
    if (R_VIN_Open(&vin, &capture_cfg) != FSP_SUCCESS) {
        R_VIN->FC_b.CC = 0;
        R_VIN->MC_b.ME = 0;
        R_BSP_IrqDisable(vin_cfg.interrupt_cfg.status.irq);
        R_BSP_IrqDisable(vin_cfg.interrupt_cfg.error.irq);
        R_VIN->MB1 = R_VIN->MB2 = R_VIN->MB3 = 0;
        (void)R_MIPI_CSI_Close(&g_cam_mipi_csi_ctrl);
        memset(&capture_sink, 0, sizeof(capture_sink));
        return -1;
    }
    module_stopped = false;
    opened = true;
    return 0;
}

int titan_camera_start(void) {
    if (!opened || module_stopped) { return -1; }
    if (streaming) { return 0; }
    if (R_VIN_CaptureStart(&vin, NULL) != FSP_SUCCESS) { return -1; }
    streaming = true;
    rt_thread_mdelay(5);
    if (sensor_write(0x4202, 0x00)) { titan_camera_close(); return -1; }
    return 0;
}

void cam_vin_callback(capture_callback_args_t *args) {
    rt_interrupt_enter();
    capture_status.vin_irq = args->interrupt_status;
    capture_status.vin_status = args->event_status;
    uint32_t error = args->interrupt_status & VIN_ERROR_MASK;
    if (args->event == VIN_EVENT_ERROR || error) {
        capture_status.vin_errors++;
        faults |= error ? error : 0x40000000U;
        rt_completion_done(&frame_done);
    } else if (args->interrupt_status & R_VIN_INTS_FMS_Msk) {
        unsigned finished = (args->event_status & R_VIN_MS_FMS_Msk) >> R_VIN_MS_FMS_Pos;
        /* FBS identifies the latest frame, not the currently written bank.
         * In a normal completed frame FMS == FBS while MA may remain set.
         * Treating that combination as "busy" discards every valid frame.
         *
         * Exchange the just-completed bank while the three-bank ring moves
         * to its next bank. Mask preemption across the live-status check and
         * MB write; discard a stale ISR snapshot rather than publishing it.
         */
        rt_base_t level = rt_hw_interrupt_disable();
        uint32_t current_status = R_VIN->MS;
        const uint32_t bank_mask = R_VIN_MS_FMS_Msk | R_VIN_MS_FBS_Msk;
        bool stale = ((current_status ^ args->event_status) & bank_mask) != 0;
        if (stale) { capture_status.stale_frames++; }
        if (finished < 3 && !stale && capture_sink.take_buffer) {
            volatile uint32_t *mailbox = finished == 0 ? &R_VIN->MB1 : (finished == 1 ? &R_VIN->MB2 : &R_VIN->MB3);
            uint8_t *completed = dma_banks[finished];
            if (completed == args->p_buffer && (uint32_t)(uintptr_t)completed == *mailbox) {
                uint8_t *replacement = capture_sink.take_buffer(capture_sink.context);
                bool valid = replacement && !((uintptr_t)replacement & 127U) &&
                    titan_memory_is_bus_accessible(replacement, dma_capacity);
                for (unsigned i = 0; valid && i < 3; i++) {
                    if (replacement == dma_banks[i]) { valid = false; }
                }
                if (valid) {
                    *mailbox = (uint32_t)(uintptr_t)replacement;
                    dma_banks[finished] = replacement;
                    __DSB();
                    sequence++;
                    capture_sink.complete_buffer(capture_sink.context, completed, sequence);
                    rt_completion_done(&frame_done);
                } else if (replacement) {
                    faults |= 0x10000000U;
                    rt_completion_done(&frame_done);
                } else {
                    capture_status.starved_frames++;
                }
            } else {
                capture_status.stale_frames++;
            }
        }
        rt_hw_interrupt_enable(level);
    }
    rt_interrupt_leave();
}

void cam_mipi_csi_callback(mipi_csi_callback_args_t *args) {
    rt_interrupt_enter();
    uint32_t status = 0;
    if (args->event == MIPI_CSI_EVENT_DATA_LANE) {
        uint32_t lane_status = args->event_data.data_lane_status.mask;
        if (args->event_idx < 2) { capture_status.lane_status[args->event_idx] = lane_status; }
        if (lane_status & (R_MIPI_CSI_DLST0_EUL_Msk | R_MIPI_CSI_DLST0_RUL_Msk)) {
            capture_status.lane_events++;
        }
        /* DLST also reports ULPS entry/exit and the current power state.
         * Those notifications are not transfer errors. */
        status = lane_status & CSI_LANE_ERROR_MASK;
    } else if (args->event == MIPI_CSI_EVENT_VIRTUAL_CHANNEL) {
        uint32_t vc_status = args->event_data.virtual_channel_status.mask;
        /* FSP reports receiver-wide malformed/2-bit ECC errors at index FF. */
        if (args->event_idx == 0xff || args->event_idx == vin_cfg.input_ctrl.csi_mode_bits.virtual_channel) {
            if (vc_status) { capture_status.vc_status = vc_status; }
            /* A corrected single-bit ECC event remains useful diagnostics,
             * but the repaired header must not stop an otherwise good stream. */
            if (vc_status & R_MIPI_CSI_VCST0_ECC_Msk) { capture_status.corrected_ecc++; }
            status = vc_status & CSI_VC_ERROR_MASK;
        }
    }
    if (status) {
        capture_status.csi_errors++;
        faults |= 0x20000000U | status;
        rt_completion_done(&frame_done);
    }
    rt_interrupt_leave();
}

void titan_camera_wait(unsigned milliseconds) {
    rt_tick_t ticks = rt_tick_from_millisecond(milliseconds);
    (void)rt_completion_wait(&frame_done, ticks ? ticks : 1);
}

uint32_t titan_camera_error(void) { return faults; }

void titan_camera_get_status(titan_camera_status_t *status) {
    rt_base_t level = rt_hw_interrupt_disable();
    *status = capture_status;
    status->frames = sequence;
    status->faults = faults;
    status->opened = opened;
    status->streaming = streaming;
    rt_hw_interrupt_enable(level);
}

void titan_camera_report_error(const char *reason, unsigned elapsed_ms) {
    titan_camera_status_t status;
    titan_camera_get_status(&status);
    /* Task context only. Do not print, allocate or read gated MMIO in an ISR. */
    rt_kprintf("[titan.csi] %s: wait=%u ms frames=%u faults=0x%08x\n",
        reason, elapsed_ms, (unsigned)status.frames, (unsigned)status.faults);
    rt_kprintf("[titan.csi] VIN irq=0x%08x ms=0x%08x errors=%u stale=%u starved=%u\n",
        (unsigned)status.vin_irq, (unsigned)status.vin_status, (unsigned)status.vin_errors,
        (unsigned)status.stale_frames, (unsigned)status.starved_frames);
    rt_kprintf("[titan.csi] CSI lane0=0x%08x lane1=0x%08x vc=0x%08x errors=%u ecc_fixed=%u ulps=%u\n",
        (unsigned)status.lane_status[0], (unsigned)status.lane_status[1], (unsigned)status.vc_status,
        (unsigned)status.csi_errors, (unsigned)status.corrected_ecc, (unsigned)status.lane_events);
}
