/* SPDX-License-Identifier: Apache-2.0
 * Titan Board external pin IRQs. RASC's IRQ4 vector is retained; the remaining
 * vectors are supplied by the project-local machine vector overlay.
 */
#include <hal_data.h>
#include <rthw.h>
#include <r_icu.h>
#include "ra8_gpio.h"
#include "ra8_pin_irq.h"
#include "titan_machine_vectors.h"

#define TITAN_IRQ_CHANNELS (32)
#define TITAN_IRQ_ALIAS_MAX (4)

typedef struct {
    rt_base_t pin;
    uint32_t config;
} titan_irq_alias_t;

typedef struct {
    icu_instance_ctrl_t ctrl;
    external_irq_cfg_t cfg;
    rt_base_t pin;
    uint32_t pin_config;
    void (*handler)(void *);
    void *args;
    rt_bool_t attached;
    rt_bool_t enabled;
    unsigned alias_count;
    titan_irq_alias_t aliases[TITAN_IRQ_ALIAS_MAX];
} titan_pin_irq_t;

static titan_pin_irq_t titan_irqs[TITAN_IRQ_CHANNELS];
static const icu_extended_cfg_t titan_irq_extend = {
    .filter_src = EXTERNAL_IRQ_DIGITAL_FILTER_PCLK_DIV,
};

/* RA8P1 pin IRQ aliases. Permanent Titan resources are guarded by machine_pin. */
int ra8_pin_irq_channel(rt_base_t pin) {
    switch (pin) {
        case 0x0105: return 0;                         /* IRQ0 */
        case 0x0201: return 4;                         /* MD/BOOT */
        case 0x040a: return 5;                         /* Camera I2C0 SCL */
        case 0x0000: case 0x0409: return 6;
        case 0x0001: case 0x0706: return 7;
        case 0x0002: case 0x0707: return 8;
        case 0x0004: return 9;
        case 0x0005: case 0x0709: return 10;
        case 0x0006: case 0x0708: return 11;
        case 0x0008: case 0x070f: case 0x0801: return 12;
        case 0x0009: case 0x000f: case 0x050e: case 0x070e: return 13;
        case 0x000c: case 0x080f: return 15;
        case 0x0106: case 0x080e: return 16;
        case 0x0102: return 17;
        case 0x0802: return 18;
        case 0x010a: return 20;                        /* Titan USER_KEY */
        case 0x0109: return 23;                        /* IRQ23 */
        case 0x0108: return 24;                        /* IRQ24 */
        case 0x0605: return 25;
        case 0x0604: return 26;
        case 0x000e: case 0x0603: return 27;
        case 0x0602: return 28;
        case 0x0601: return 29;
        default: return -1;
    }
}

static uint32_t titan_pin_config(rt_base_t pin) {
    /* PIDR is the changing input level, not writable configuration. */
    return R_PFS->PORT[pin >> 8].PIN[pin & 15].PmnPFS & ~R_PFS_PORT_PIN_PmnPFS_PIDR_Msk;
}

static fsp_err_t titan_pin_configure(rt_base_t pin, uint32_t config) {
    return R_IOPORT_PinCfg(&g_ioport_ctrl, (bsp_io_port_pin_t)pin, config);
}

static void titan_pin_irq_callback(external_irq_callback_args_t *args) {
    titan_pin_irq_t *irq = args->p_context;
    rt_interrupt_enter();
    if (irq && irq->attached && irq->enabled && irq->handler) {
        irq->handler(irq->args);
    }
    rt_interrupt_leave();
}

static void titan_restore_aliases(titan_pin_irq_t *irq) {
    while (irq->alias_count) {
        titan_irq_alias_t *alias = &irq->aliases[--irq->alias_count];
        /* Preserve a later GPIO/peripheral reconfiguration of this alias. */
        if (titan_pin_config(alias->pin) == (alias->config & ~IOPORT_CFG_IRQ_ENABLE)) {
            titan_pin_configure(alias->pin, alias->config);
        }
    }
}

rt_err_t ra8_pin_irq_attach(rt_base_t pin, rt_uint8_t mode, void (*handler)(void *), void *args) {
    int channel = ra8_pin_irq_channel(pin);
    if (channel < 0 || !handler) { return -RT_EINVAL; }
    if (!ra8_gpio_is_ready()) { return -RT_EIO; }
    external_irq_trigger_t trigger;
    switch (mode) {
        case PIN_IRQ_MODE_RISING: trigger = EXTERNAL_IRQ_TRIGGER_RISING; break;
        case PIN_IRQ_MODE_FALLING: trigger = EXTERNAL_IRQ_TRIGGER_FALLING; break;
        case PIN_IRQ_MODE_RISING_FALLING: trigger = EXTERNAL_IRQ_TRIGGER_BOTH_EDGE; break;
        default: return -RT_EINVAL;
    }

    titan_pin_irq_t *irq = &titan_irqs[channel];
    IRQn_Type vector = TITAN_PIN_IRQ_VECTOR(channel);
    if (vector < 0 || vector >= BSP_ICU_VECTOR_NUM_ENTRIES) { return -RT_EINVAL; }
    rt_base_t level = rt_hw_interrupt_disable();
    /* FSP does not arbitrate separate control blocks for the same channel.
     * The ISR context catches a channel already owned by a board driver,
     * including a subsequently enabled RASC USER/BOOT IRQ4 instance.
     */
    if (irq->attached || R_FSP_IsrContextGet(vector) != RT_NULL) {
        rt_hw_interrupt_enable(level);
        return -RT_EBUSY;
    }

    irq->alias_count = 0;
    for (unsigned i = 0; i < g_bsp_pin_cfg.number_of_pins; ++i) {
        rt_base_t other = g_bsp_pin_cfg.p_pin_cfg_data[i].pin;
        if (other == pin || ra8_pin_irq_channel(other) != channel) { continue; }
        uint32_t config = titan_pin_config(other);
        if (!(config & IOPORT_CFG_IRQ_ENABLE)) { continue; }
        if (irq->alias_count == TITAN_IRQ_ALIAS_MAX) {
            titan_restore_aliases(irq);
            rt_hw_interrupt_enable(level);
            return -RT_EBUSY;
        }
        if (titan_pin_configure(other, config & ~IOPORT_CFG_IRQ_ENABLE) != FSP_SUCCESS) {
            titan_restore_aliases(irq);
            rt_hw_interrupt_enable(level);
            return -RT_EIO;
        }
        irq->aliases[irq->alias_count++] = (titan_irq_alias_t){other, config};
    }

    irq->pin = pin;
    irq->pin_config = titan_pin_config(pin);
    irq->handler = handler;
    irq->args = args;
    irq->enabled = RT_FALSE;
    irq->cfg = (external_irq_cfg_t) {
        .channel = (uint8_t)channel,
        .ipl = 12,
        .irq = vector,
        .trigger = trigger,
        .clock_source_div = EXTERNAL_IRQ_CLOCK_SOURCE_DIV_1,
        .filter_enable = false,
        .p_callback = titan_pin_irq_callback,
        .p_context = irq,
        .p_extend = &titan_irq_extend,
    };
    if (R_ICU_ExternalIrqOpen(&irq->ctrl, &irq->cfg) != FSP_SUCCESS) {
        titan_restore_aliases(irq);
        irq->handler = RT_NULL;
        irq->args = RT_NULL;
        rt_hw_interrupt_enable(level);
        return -RT_EIO;
    }
    irq->attached = RT_TRUE;
    rt_hw_interrupt_enable(level);
    return RT_EOK;
}

rt_err_t ra8_pin_irq_enable(rt_base_t pin, rt_uint8_t enabled) {
    int channel = ra8_pin_irq_channel(pin);
    if (channel < 0 || (enabled != PIN_IRQ_ENABLE && enabled != PIN_IRQ_DISABLE)) { return -RT_EINVAL; }
    titan_pin_irq_t *irq = &titan_irqs[channel];
    rt_base_t level = rt_hw_interrupt_disable();
    if (!irq->attached || irq->pin != pin) {
        rt_hw_interrupt_enable(level);
        return enabled == PIN_IRQ_DISABLE ? RT_EOK : -RT_EINVAL;
    }
    fsp_err_t result;
    if (enabled == PIN_IRQ_ENABLE) {
        /* IRQ inputs are digital GPIO inputs; retain the selected pull-up. */
        uint32_t config = titan_pin_config(pin);
        config &= ~(IOPORT_CFG_ANALOG_ENABLE | IOPORT_CFG_PERIPHERAL_PIN |
                    IOPORT_CFG_PORT_DIRECTION_OUTPUT | R_PFS_PORT_PIN_PmnPFS_PSEL_Msk);
        config |= IOPORT_CFG_IRQ_ENABLE;
        result = titan_pin_configure(pin, config);
        if (result == FSP_SUCCESS) {
            irq->enabled = RT_TRUE;
            result = R_ICU_ExternalIrqEnable(&irq->ctrl);
            if (result != FSP_SUCCESS) { irq->enabled = RT_FALSE; }
        }
    } else {
        irq->enabled = RT_FALSE;
        result = R_ICU_ExternalIrqDisable(&irq->ctrl);
    }
    rt_hw_interrupt_enable(level);
    return result == FSP_SUCCESS ? RT_EOK : -RT_EIO;
}

rt_err_t ra8_pin_irq_detach(rt_base_t pin) {
    int channel = ra8_pin_irq_channel(pin);
    if (channel < 0) { return -RT_EINVAL; }
    titan_pin_irq_t *irq = &titan_irqs[channel];
    rt_base_t level = rt_hw_interrupt_disable();
    if (!irq->attached || irq->pin != pin) {
        rt_hw_interrupt_enable(level);
        return RT_EOK;
    }
    irq->enabled = RT_FALSE;
    fsp_err_t result = R_ICU_ExternalIrqClose(&irq->ctrl);
    if (result == FSP_SUCCESS) {
        result = titan_pin_configure(pin, irq->pin_config);
        titan_restore_aliases(irq);
        irq->attached = RT_FALSE;
        irq->handler = RT_NULL;
        irq->args = RT_NULL;
    }
    rt_hw_interrupt_enable(level);
    return result == FSP_SUCCESS ? RT_EOK : -RT_EIO;
}
