"""Inject CSI/VIN events into the production callbacks on a native host.

Run with Python 3.10+ and a native GCC (MPY_HOST_CC may select it).
This checks error classification and buffer ownership, not sensor timing.
"""
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT))
from tools.host_tools import configure_host_tools


def function(source, name):
    start = source.index('void ' + name + '(')
    brace = source.index('{', start)
    depth = 1
    end = brace + 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end]


def main():
    source = (ROOT / 'platform/camera/titan_camera.c').read_text()
    device = (ROOT / 'ra/fsp/src/bsp/cmsis/Device/RENESAS/Include/R7KA8P1KF_core0.h').read_text()
    definitions = '\n'.join(line.strip() for line in device.splitlines()
                            if re.match(r'\s*#define R_(VIN_MS_|VIN_INTS_|MIPI_CSI_DLST0_|MIPI_CSI_VCST0_)', line))
    masks = source[source.index('#define VIN_ERROR_MASK'):source.index('static int sensor_write')]
    callbacks = '\n'.join(function(source, name) for name in (
        'cam_vin_callback', 'cam_mipi_csi_callback', 'titan_camera_get_status'))
    harness = r'''
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "titan_camera.h"

typedef int rt_base_t;
enum { VIN_EVENT_NOTIFY, VIN_EVENT_ERROR };
enum { MIPI_CSI_EVENT_DATA_LANE, MIPI_CSI_EVENT_VIRTUAL_CHANNEL, MIPI_CSI_EVENT_POWER };
typedef struct {
    int event;
    uint32_t interrupt_status, event_status;
    uint8_t *p_buffer;
} capture_callback_args_t;
typedef struct {
    int event;
    uint8_t event_idx;
    union { struct { uint32_t mask; } data_lane_status, virtual_channel_status; } event_data;
} mipi_csi_callback_args_t;
static struct { uint32_t MS, MB1, MB2, MB3; } registers;
#define R_VIN (&registers)
#define __DSB() ((void)0)
static struct { struct { struct { unsigned virtual_channel; } csi_mode_bits; } input_ctrl; } vin_cfg;
static titan_camera_sink_t capture_sink;
static uint8_t *dma_banks[3];
static size_t dma_capacity = 128;
static volatile uint32_t sequence, faults;
static volatile titan_camera_status_t capture_status;
static bool opened = true, streaming = true;
static int frame_done, irq_depth, lock_depth;
static void rt_interrupt_enter(void) { irq_depth++; }
static void rt_interrupt_leave(void) { assert(irq_depth == 1); irq_depth--; }
static rt_base_t rt_hw_interrupt_disable(void) { return lock_depth++; }
static void rt_hw_interrupt_enable(rt_base_t level) { assert(lock_depth == level + 1); lock_depth = level; }
static void rt_completion_done(int *completion) { (*completion)++; }
static bool titan_memory_is_bus_accessible(void *data, size_t length) { return data && length == 128; }

/* PRODUCTION CALLBACKS */

static void reset_events(void) {
    faults = sequence = frame_done = 0;
    capture_status = (titan_camera_status_t){0};
    vin_cfg.input_ctrl.csi_mode_bits.virtual_channel = 0;
}
static void csi_event(int event, uint8_t index, uint32_t status) {
    mipi_csi_callback_args_t args = {.event=event, .event_idx=index};
    args.event_data.data_lane_status.mask = status;
    cam_mipi_csi_callback(&args);
    assert(!irq_depth && !lock_depth);
}
static void test_csi_notifications(void) {
    reset_events();
    csi_event(MIPI_CSI_EVENT_DATA_LANE, 0, R_MIPI_CSI_DLST0_EUL_Msk | R_MIPI_CSI_DLST0_ULP_Msk);
    csi_event(MIPI_CSI_EVENT_DATA_LANE, 1, R_MIPI_CSI_DLST0_RUL_Msk);
    csi_event(MIPI_CSI_EVENT_VIRTUAL_CHANNEL, 0, R_MIPI_CSI_VCST0_ECC_Msk | R_MIPI_CSI_VCST0_FER_Msk);
    csi_event(MIPI_CSI_EVENT_VIRTUAL_CHANNEL, 0xff, 0);
    assert(!faults && !frame_done && !capture_status.csi_errors);
    assert(capture_status.lane_events == 2 && capture_status.corrected_ecc == 1);
    assert(capture_status.vc_status & R_MIPI_CSI_VCST0_ECC_Msk);
    /* Other virtual channels must not poison the selected camera stream. */
    csi_event(MIPI_CSI_EVENT_VIRTUAL_CHANNEL, 2, R_MIPI_CSI_VCST0_CRC_Msk);
    assert(!faults);
}
static void test_real_csi_errors(void) {
    const uint32_t lane_errors[] = {R_MIPI_CSI_DLST0_ESH_Msk, R_MIPI_CSI_DLST0_ESS_Msk,
        R_MIPI_CSI_DLST0_ECT_Msk, R_MIPI_CSI_DLST0_EES_Msk};
    for (unsigned i = 0; i < sizeof(lane_errors)/sizeof(*lane_errors); i++) {
        reset_events();
        csi_event(MIPI_CSI_EVENT_DATA_LANE, i % 2, lane_errors[i] | R_MIPI_CSI_DLST0_EUL_Msk);
        assert(faults == (0x20000000U | lane_errors[i]));
        assert(frame_done == 1 && capture_status.csi_errors == 1);
        csi_event(MIPI_CSI_EVENT_DATA_LANE, 0, R_MIPI_CSI_DLST0_RUL_Msk);
        assert(faults == (0x20000000U | lane_errors[i]));
    }
    const uint32_t vc_errors[] = {R_MIPI_CSI_VCST0_MLF_Msk, R_MIPI_CSI_VCST0_ECD_Msk,
        R_MIPI_CSI_VCST0_CRC_Msk, R_MIPI_CSI_VCST0_IDE_Msk, R_MIPI_CSI_VCST0_WCE_Msk,
        R_MIPI_CSI_VCST0_FRS_Msk, R_MIPI_CSI_VCST0_FRD_Msk, R_MIPI_CSI_VCST0_OVF_Msk};
    for (unsigned i = 0; i < sizeof(vc_errors)/sizeof(*vc_errors); i++) {
        reset_events();
        csi_event(MIPI_CSI_EVENT_VIRTUAL_CHANNEL, i < 2 ? 0xff : 0, vc_errors[i]);
        assert(faults == (0x20000000U | vc_errors[i]));
        assert(frame_done == 1 && capture_status.csi_errors == 1);
    }
}
_Alignas(128) static uint8_t buffers[5][128];
static bool free_buffers[5], starve;
static uint8_t *ready, *used;
static uint8_t *take(void *context) {
    (void)context;
    if (starve) { return NULL; }
    if (ready) { uint8_t *p = ready; ready = NULL; return p; }
    for (unsigned i = 0; i < 5; i++) {
        if (free_buffers[i]) { free_buffers[i] = false; return buffers[i]; }
    }
    return NULL;
}
static void complete(void *context, uint8_t *data, uint32_t serial) {
    (void)context;
    assert(!ready && data != used && serial == sequence);
    for (unsigned i = 0; i < 3; i++) { assert(data != dma_banks[i]); }
    ready = data;
}
static void release_used(void) {
    if (used) { free_buffers[(used - &buffers[0][0]) / 128] = true; used = NULL; }
}
static void frame(unsigned bank, bool stale) {
    uint32_t state = (bank << R_VIN_MS_FMS_Pos) | (bank << R_VIN_MS_FBS_Pos);
    capture_callback_args_t args = {.event=VIN_EVENT_NOTIFY,
        .interrupt_status=R_VIN_INTS_FMS_Msk, .event_status=state, .p_buffer=dma_banks[bank]};
    registers.MS = stale ? state ^ (1U << R_VIN_MS_FBS_Pos) : state;
    cam_vin_callback(&args);
    assert(!irq_depth && !lock_depth);
}
static void test_continuous_capture(void) {
    reset_events();
    ready = used = NULL;
    for (unsigned i = 0; i < 5; i++) { free_buffers[i] = i >= 3; }
    for (unsigned i = 0; i < 3; i++) { dma_banks[i] = buffers[i]; }
    registers.MB1 = (uint32_t)(uintptr_t)dma_banks[0];
    registers.MB2 = (uint32_t)(uintptr_t)dma_banks[1];
    registers.MB3 = (uint32_t)(uintptr_t)dma_banks[2];
    capture_sink = (titan_camera_sink_t){.take_buffer=take, .complete_buffer=complete};
    /* A slow consumer plus benign CSI events must keep the DMA ring running. */
    for (unsigned i = 0; i < 30000; i++) {
        csi_event(MIPI_CSI_EVENT_DATA_LANE, i % 2, R_MIPI_CSI_DLST0_RUL_Msk);
        if (i % 100 == 0) { csi_event(MIPI_CSI_EVENT_VIRTUAL_CHANNEL, 0, R_MIPI_CSI_VCST0_ECC_Msk); }
        frame(i % 3, false);
        if (i % 7 == 0) { release_used(); used = ready; ready = NULL; }
        assert(sequence == i + 1 && !faults);
    }
    frame(0, true);
    assert(capture_status.stale_frames == 1 && sequence == 30000);
    starve = true;
    frame(0, false);
    assert(capture_status.starved_frames == 1 && sequence == 30000 && !faults);
    starve = false;
    frame(0, false);
    assert(sequence == 30001 && !faults);
    capture_callback_args_t error = {.event=VIN_EVENT_NOTIFY,
        .interrupt_status=R_VIN_INTS_FMS_Msk | R_VIN_INTS_FOS_Msk};
    cam_vin_callback(&error);
    assert(faults == R_VIN_INTS_FOS_Msk && capture_status.vin_errors == 1 && sequence == 30001);
    titan_camera_status_t status;
    opened = streaming = false;
    titan_camera_get_status(&status);
    assert(status.frames == sequence && status.faults == faults && !status.opened && !status.streaming);
}
int main(void) {
    test_csi_notifications();
    test_real_csi_errors();
    test_continuous_capture();
    puts("PASS: benign ULPS/ECC, real CSI errors, VC filtering, 30000 DMA frames, stale/starved/error ownership");
    return 0;
}
'''
    harness = harness.replace('/* PRODUCTION CALLBACKS */', definitions + '\n' + masks + '\n' + callbacks)
    env = os.environ.copy()
    configure_host_tools(env, str(ROOT))
    compiler = env.get('MPY_HOST_CC') or shutil.which('gcc', path=env['PATH'])
    if not compiler:
        raise RuntimeError('A native GCC is required for camera callback regression tests')
    with tempfile.TemporaryDirectory(prefix='titan-camera-events-') as directory:
        source_file = Path(directory) / 'camera_events.c'
        executable = Path(directory) / ('camera_events.exe' if os.name == 'nt' else 'camera_events')
        source_file.write_text(harness)
        subprocess.run([compiler, '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror',
                        '-I' + str(ROOT/'platform/camera'), str(source_file), '-o', str(executable)],
                       check=True, env=env)
        subprocess.run([str(executable)], check=True, env=env)


if __name__ == '__main__':
    main()
