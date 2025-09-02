/* generated HAL source file - do not edit */
#include "hal_data.h"
ipc_instance_ctrl_t g_ipc1_ctrl;

            /** IPC configuration */
            const ipc_cfg_t g_ipc1_cfg =
            {
                .channel = 1,
                .p_callback = NULL,
#if defined(NULL)
                .p_context = NULL,
#else
                .p_context = (void *) &NULL,
#endif
                .ipl = (BSP_IRQ_DISABLED),
#if defined(VECTOR_NUMBER_IPC_IRQ1)
                .irq = VECTOR_NUMBER_IPC_IRQ1,
#else
                .irq = FSP_INVALID_VECTOR,
#endif
            };

            /* Instance structure to use this module. */
            const ipc_instance_t g_ipc1 =
            {
                .p_ctrl = &g_ipc1_ctrl,
                .p_cfg = &g_ipc1_cfg,
                .p_api = &g_ipc_on_ipc
            };
ipc_instance_ctrl_t g_ipc0_ctrl;

            /** IPC configuration */
            const ipc_cfg_t g_ipc0_cfg =
            {
                .channel = 0,
                .p_callback = g_ipc0_callback,
#if defined(NULL)
                .p_context = NULL,
#else
                .p_context = (void *) &NULL,
#endif
                .ipl = (5),
#if defined(VECTOR_NUMBER_IPC_IRQ0)
                .irq = VECTOR_NUMBER_IPC_IRQ0,
#else
                .irq = FSP_INVALID_VECTOR,
#endif
            };

            /* Instance structure to use this module. */
            const ipc_instance_t g_ipc0 =
            {
                .p_ctrl = &g_ipc0_ctrl,
                .p_cfg = &g_ipc0_cfg,
                .p_api = &g_ipc_on_ipc
            };
sci_b_uart_instance_ctrl_t     g_uart5_ctrl;

            sci_b_baud_setting_t               g_uart5_baud_setting =
            {
                /* Baud rate calculated with 0.160% error. */ .baudrate_bits_b.abcse = 0, .baudrate_bits_b.abcs = 0, .baudrate_bits_b.bgdm = 1, .baudrate_bits_b.cks = 0, .baudrate_bits_b.brr = 64, .baudrate_bits_b.mddr = (uint8_t) 256, .baudrate_bits_b.brme = false
            };

            /** UART extended configuration for UARTonSCI HAL driver */
            const sci_b_uart_extended_cfg_t g_uart5_cfg_extend =
            {
                .clock                = SCI_B_UART_CLOCK_INT,
                .rx_edge_start          = SCI_B_UART_START_BIT_FALLING_EDGE,
                .noise_cancel         = SCI_B_UART_NOISE_CANCELLATION_DISABLE,
                .rx_fifo_trigger        = SCI_B_UART_RX_FIFO_TRIGGER_MAX,
                .p_baud_setting         = &g_uart5_baud_setting,
                .flow_control           = SCI_B_UART_FLOW_CONTROL_RTS,
                #if 0xFF != 0xFF
                .flow_control_pin       = BSP_IO_PORT_FF_PIN_0xFF,
                #else
                .flow_control_pin       = (bsp_io_port_pin_t) UINT16_MAX,
                #endif
                .rs485_setting          = {
                    .enable = SCI_B_UART_RS485_DISABLE,
                    .polarity = SCI_B_UART_RS485_DE_POLARITY_HIGH,
                    .assertion_time = 1,
                    .negation_time = 1,
                }
            };

            /** UART interface configuration */
            const uart_cfg_t g_uart5_cfg =
            {
                .channel             = 5,
                .data_bits           = UART_DATA_BITS_8,
                .parity              = UART_PARITY_OFF,
                .stop_bits           = UART_STOP_BITS_1,
                .p_callback          = user_uart5_callback,
                .p_context           = NULL,
                .p_extend            = &g_uart5_cfg_extend,
#define RA_NOT_DEFINED (1)
#if (RA_NOT_DEFINED == RA_NOT_DEFINED)
                .p_transfer_tx       = NULL,
#else
                .p_transfer_tx       = &RA_NOT_DEFINED,
#endif
#if (RA_NOT_DEFINED == RA_NOT_DEFINED)
                .p_transfer_rx       = NULL,
#else
                .p_transfer_rx       = &RA_NOT_DEFINED,
#endif
#undef RA_NOT_DEFINED
                .rxi_ipl             = (12),
                .txi_ipl             = (12),
                .tei_ipl             = (12),
                .eri_ipl             = (12),
#if defined(VECTOR_NUMBER_SCI5_RXI)
                .rxi_irq             = VECTOR_NUMBER_SCI5_RXI,
#else
                .rxi_irq             = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_SCI5_TXI)
                .txi_irq             = VECTOR_NUMBER_SCI5_TXI,
#else
                .txi_irq             = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_SCI5_TEI)
                .tei_irq             = VECTOR_NUMBER_SCI5_TEI,
#else
                .tei_irq             = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_SCI5_ERI)
                .eri_irq             = VECTOR_NUMBER_SCI5_ERI,
#else
                .eri_irq             = FSP_INVALID_VECTOR,
#endif
            };

/* Instance structure to use this module. */
const uart_instance_t g_uart5 =
{
    .p_ctrl        = &g_uart5_ctrl,
    .p_cfg         = &g_uart5_cfg,
    .p_api         = &g_uart_on_sci_b
};
void g_hal_init(void) {
g_common_init();
}
