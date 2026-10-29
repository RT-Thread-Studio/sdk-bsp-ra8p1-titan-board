/* SPDX-License-Identifier: Apache-2.0 */
#include <string.h>
#include "board_config.h"
#include "omv_gpio.h"
#include "omv_i2c.h"
#include "titan_camera.h"

const struct ra8_pin ra8_camera_reset_pin = {TITAN_CAMERA_RESET_PIN};

void omv_gpio_init0(void) {}
void omv_gpio_config(omv_gpio_t pin, uint32_t mode, uint32_t pull, uint32_t speed, uint32_t af) {
    if (!pin) { return; }
    if (mode == OMV_GPIO_MODE_INPUT && pull == OMV_GPIO_PULL_UP) { mode = PIN_MODE_INPUT_PULLUP; }
    if (mode == OMV_GPIO_MODE_INPUT && pull == OMV_GPIO_PULL_DOWN) { mode = PIN_MODE_INPUT_PULLDOWN; }
    rt_pin_mode(pin->pin, mode);
}
void omv_gpio_deinit(omv_gpio_t pin) { if (pin) { rt_pin_mode(pin->pin, PIN_MODE_INPUT); } }
bool omv_gpio_read(omv_gpio_t pin) { return pin && rt_pin_read(pin->pin); }
void omv_gpio_write(omv_gpio_t pin, bool value) { if (pin) { rt_pin_write(pin->pin, value); } }
void omv_gpio_clock_enable(omv_gpio_t pin, bool enable) { (void)pin; (void)enable; }
void omv_gpio_irq_register(omv_gpio_t pin, omv_gpio_callback_t callback, void *data) {
    if (pin) { rt_pin_attach_irq(pin->pin, PIN_IRQ_MODE_RISING, callback, data); }
}
void omv_gpio_irq_enable(omv_gpio_t pin, bool enable) {
    if (pin) { rt_pin_irq_enable(pin->pin, enable ? PIN_IRQ_ENABLE : PIN_IRQ_DISABLE); }
}

int omv_i2c_init(omv_i2c_t *bus, uint32_t id, uint32_t speed) {
    memset(bus, 0, sizeof(*bus));
    bus->inst = (struct rt_i2c_bus_device *)rt_device_find(TITAN_CAMERA_I2C_BUS);
    bus->id = id;
    bus->speed = speed;
    bus->initialized = bus->inst != NULL;
    return bus->initialized ? 0 : -1;
}
int omv_i2c_deinit(omv_i2c_t *bus) { bus->initialized = 0; bus->pending_len = 0; return 0; }
int omv_i2c_enable(omv_i2c_t *bus, bool enable) { bus->initialized = enable && bus->inst; return bus->inst ? 0 : -1; }
int omv_i2c_scan(omv_i2c_t *bus, uint8_t *list, uint8_t size) {
    if (!bus->initialized || !bus->inst) { return -1; }
    /* OV5640 needs a 16-bit SCCB register address; do not perform a blind read
     * whose result depends on the sensor's previous address pointer. */
    uint8_t reg[] = {0x30, 0x0a};
    uint8_t id[2] = {0};
    struct rt_i2c_msg msgs[] = {
        {.addr=0x3c, .flags=RT_I2C_WR | RT_I2C_NO_STOP, .len=2, .buf=reg},
        {.addr=0x3c, .flags=RT_I2C_RD, .len=2, .buf=id}
    };
    if (rt_i2c_transfer(bus->inst, msgs, 2) != 2) {
        rt_kprintf("[titan.csi] OV5640 ID read failed at i2c0/0x3c reg=0x300a\n");
        return 0;
    }
    if (id[0] != 0x56 || id[1] != 0x40) { return 0; }
    if (!size) { return 0x78; }
    list[0] = 0x78;
    return 1;
}

int omv_i2c_write(omv_i2c_t *bus, uint8_t address, uint8_t *data, uint32_t size, uint32_t flags) {
    if (!bus->initialized) { return -1; }
    if (flags & (OMV_I2C_XFER_NO_STOP | OMV_I2C_XFER_SUSPEND)) {
        if (bus->pending_len + size > sizeof(bus->pending)) { bus->pending_len = 0; return -1; }
        memcpy(bus->pending + bus->pending_len, data, size);
        bus->pending_len += size;
        return 0;
    }
    if (size > (uint32_t)UINT16_MAX - bus->pending_len) { bus->pending_len = 0; return -1; }
    uint8_t local[16];
    uint8_t *joined = NULL;
    uint32_t total = size + bus->pending_len;
    uint8_t *payload = data;
    if (bus->pending_len) {
        joined = total <= sizeof(local) ? local : rt_malloc(total);
        if (!joined) { bus->pending_len = 0; return -1; }
        memcpy(joined, bus->pending, bus->pending_len);
        memcpy(joined + bus->pending_len, data, size);
        payload = joined;
    }
    bus->pending_len = 0;
    struct rt_i2c_msg msg = {.addr=address >> 1, .flags=RT_I2C_WR, .len=total, .buf=payload};
    int result = rt_i2c_transfer(bus->inst, &msg, 1) == 1 ? 0 : -1;
    if (joined && joined != local) { rt_free(joined); }
    return result;
}

int omv_i2c_read(omv_i2c_t *bus, uint8_t address, uint8_t *data, uint32_t size, uint32_t flags) {
    if (!bus->initialized) { return -1; }
    /* This camera bus uses OV5640's 16-bit register pointers. Reject the
     * shared-address GC2145 probe before any hardware transaction; the
     * upstream read_reg helper reports zero and can continue with OV5640.
     * Plain reads, other devices and valid 16-bit register reads are unchanged. */
    if ((address >> 1) == 0x3c && bus->pending_len == 1) {
        bus->pending_len = 0;
        return -1;
    }
    struct rt_i2c_msg msgs[2];
    int count = 0;
    if (bus->pending_len) {
        msgs[count++] = (struct rt_i2c_msg){address >> 1, RT_I2C_WR | RT_I2C_NO_STOP, bus->pending_len, bus->pending};
    }
    msgs[count++] = (struct rt_i2c_msg){address >> 1, RT_I2C_RD, size, data};
    bus->pending_len = 0;
    return rt_i2c_transfer(bus->inst, msgs, count) == count ? 0 : -1;
}
int omv_i2c_gencall(omv_i2c_t *bus, uint8_t cmd) {
    return omv_i2c_write(bus, 0, &cmd, 1, OMV_I2C_XFER_NO_FLAGS);
}
int omv_i2c_pulse_scl(omv_i2c_t *bus) {
    bus->pending_len = 0;
    return 0;
}
