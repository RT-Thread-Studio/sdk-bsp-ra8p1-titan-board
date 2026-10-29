/* SPDX-License-Identifier: Apache-2.0 */
#ifndef TITAN_CAMERA_H
#define TITAN_CAMERA_H
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define TITAN_CAMERA_RESET_PIN (0x040b)
#define TITAN_CAMERA_I2C_BUS "i2c0"
#define TITAN_CAMERA_MAX_WIDTH (640U)
#define TITAN_CAMERA_MAX_HEIGHT (480U)

typedef struct {
    uint16_t x, y, width, height;
    bool rgb565;
} titan_camera_config_t;

/* IRQ-safe sink. A returned buffer is reserved exclusively for VIN; a
 * completed buffer is detached from all mailboxes before notification. */
typedef struct {
    void *context;
    uint8_t *(*take_buffer)(void *context);
    void (*complete_buffer)(void *context, uint8_t *data, uint32_t sequence);
} titan_camera_sink_t;

/* Diagnostic counters for the most recently opened capture session. Retained
 * after close so a Python exception does not erase the original failure. */
typedef struct {
    uint32_t frames, faults;
    uint32_t vin_errors, csi_errors, corrected_ecc, lane_events;
    uint32_t stale_frames, starved_frames;
    uint32_t vin_irq, vin_status, lane_status[2], vc_status;
    bool opened, streaming;
} titan_camera_status_t;

int titan_camera_clock_start(uint32_t frequency);
int titan_camera_sensor_reset(void);
int titan_camera_sensor_size(uint16_t width, uint16_t height);
int titan_camera_open(const titan_camera_config_t *config, const titan_camera_sink_t *sink, size_t capacity);
int titan_camera_start(void);
int titan_camera_close(void);
void titan_camera_quiesce_irq(void);
void titan_camera_wait(unsigned milliseconds);
uint32_t titan_camera_error(void);
void titan_camera_get_status(titan_camera_status_t *status);
void titan_camera_report_error(const char *reason, unsigned elapsed_ms);
#endif
