/* SPDX-License-Identifier: Apache-2.0
 * Keep RASC's global IOPORT configuration intact when changing one GPIO.
 */
#include <rtdevice.h>
#include <rthw.h>
#include <hal_data.h>
#include "ra8_gpio.h"

static int ra8_gpio_valid(rt_base_t pin) {
    return pin >= 0 && pin <= 0x0d0f && (pin & 0xff) <= 15;
}

rt_bool_t ra8_gpio_is_ready(void) {
    /* RASC/board startup owns Open; FSP parameter checks are disabled in this
     * release, so PinCfg/PinWrite would otherwise report success before it.
     */
    return g_ioport_ctrl.open != 0;
}

rt_base_t ra8_gpio_pin_get(const char *name) {
    if (!name || rt_strlen(name) != 4 || (name[0] != 'P' && name[0] != 'p') ||
        name[2] < '0' || name[2] > '1' || name[3] < '0' || name[3] > '9') {
        return -1;
    }
    unsigned port;
    if (name[1] >= '0' && name[1] <= '9') { port = name[1] - '0'; }
    else if (name[1] == 'A' || name[1] == 'a') { port = 10; }
    else if (name[1] >= 'A' && name[1] <= 'D') { port = 10 + name[1] - 'A'; }
    else if (name[1] >= 'a' && name[1] <= 'd') { port = 10 + name[1] - 'a'; }
    else { return -1; }
    unsigned pin = (name[2] - '0') * 10 + name[3] - '0';
    return pin <= 15 ? (rt_base_t)((port << 8) | pin) : -1;
}

rt_err_t ra8_gpio_configure(rt_base_t pin, rt_uint8_t mode, int value, rt_bool_t pullup) {
    if (!ra8_gpio_valid(pin)) { return -RT_EINVAL; }
    if (!ra8_gpio_is_ready()) { return -RT_EIO; }
    uint32_t config;
    switch (mode) {
        case PIN_MODE_INPUT: config = IOPORT_CFG_PORT_DIRECTION_INPUT; break;
        case PIN_MODE_INPUT_PULLUP: config = IOPORT_CFG_PULLUP_ENABLE; break;
        case PIN_MODE_OUTPUT: config = IOPORT_CFG_PORT_DIRECTION_OUTPUT; break;
        case PIN_MODE_OUTPUT_OD: config = IOPORT_CFG_PORT_DIRECTION_OUTPUT | IOPORT_CFG_NMOS_ENABLE; break;
        /* RA8 has an internal pull-up but no corresponding pull-down. */
        default: return -RT_EINVAL;
    }
    if (pullup) { config |= IOPORT_CFG_PULLUP_ENABLE; }
    rt_base_t level = rt_hw_interrupt_disable();
    /* Preserve the output latch, rather than the external input level. This
     * also honors a value written before switching from input to output.
     */
    config |= value < 0 ? (R_PFS->PORT[pin >> 8].PIN[pin & 15].PmnPFS & IOPORT_CFG_PORT_OUTPUT_HIGH) :
                         (value ? IOPORT_CFG_PORT_OUTPUT_HIGH : IOPORT_CFG_PORT_OUTPUT_LOW);
    fsp_err_t result = R_IOPORT_PinCfg(&g_ioport_ctrl, (bsp_io_port_pin_t)pin, config);
    rt_hw_interrupt_enable(level);
    return result == FSP_SUCCESS ? RT_EOK : -RT_EIO;
}
