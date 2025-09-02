/* generated HAL source file - do not edit */
#include "hal_data.h"
sci_b_uart_instance_ctrl_t     g_uart8_ctrl;

            sci_b_baud_setting_t               g_uart8_baud_setting =
            {
                /* Baud rate calculated with 0.160% error. */ .baudrate_bits_b.abcse = 0, .baudrate_bits_b.abcs = 0, .baudrate_bits_b.bgdm = 1, .baudrate_bits_b.cks = 0, .baudrate_bits_b.brr = 64, .baudrate_bits_b.mddr = (uint8_t) 256, .baudrate_bits_b.brme = false
            };

            /** UART extended configuration for UARTonSCI HAL driver */
            const sci_b_uart_extended_cfg_t g_uart8_cfg_extend =
            {
                .clock                = SCI_B_UART_CLOCK_INT,
                .rx_edge_start          = SCI_B_UART_START_BIT_FALLING_EDGE,
                .noise_cancel         = SCI_B_UART_NOISE_CANCELLATION_DISABLE,
                .rx_fifo_trigger        = SCI_B_UART_RX_FIFO_TRIGGER_MAX,
                .p_baud_setting         = &g_uart8_baud_setting,
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
            const uart_cfg_t g_uart8_cfg =
            {
                .channel             = 8,
                .data_bits           = UART_DATA_BITS_8,
                .parity              = UART_PARITY_OFF,
                .stop_bits           = UART_STOP_BITS_1,
                .p_callback          = user_uart8_callback,
                .p_context           = NULL,
                .p_extend            = &g_uart8_cfg_extend,
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
#if defined(VECTOR_NUMBER_SCI8_RXI)
                .rxi_irq             = VECTOR_NUMBER_SCI8_RXI,
#else
                .rxi_irq             = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_SCI8_TXI)
                .txi_irq             = VECTOR_NUMBER_SCI8_TXI,
#else
                .txi_irq             = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_SCI8_TEI)
                .tei_irq             = VECTOR_NUMBER_SCI8_TEI,
#else
                .tei_irq             = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_SCI8_ERI)
                .eri_irq             = VECTOR_NUMBER_SCI8_ERI,
#else
                .eri_irq             = FSP_INVALID_VECTOR,
#endif
            };

/* Instance structure to use this module. */
const uart_instance_t g_uart8 =
{
    .p_ctrl        = &g_uart8_ctrl,
    .p_cfg         = &g_uart8_cfg,
    .p_api         = &g_uart_on_sci_b
};
rmac_instance_ctrl_t g_ether0_ctrl;

        	uint8_t g_ether0_mac_address[6] = { 0x00,0x11,0x22,0x33,0x44,0x55 };

            layer3_switch_descriptor_t           g_ether0_tx_descriptor_array0[16];rmac_queue_info_t g_ether0_tx_queue_list[1] =
 {
{ .queue_cfg={.array_length       = 16,
.p_descriptor_array = g_ether0_tx_descriptor_array0,
.ports              = (1 << 0 ),
.type               = LAYER3_SWITCH_QUEUE_TYPE_TX,
.write_back_mode    = LAYER3_SWITCH_WRITE_BACK_MODE_FULL,
.descriptor_format  = LAYER3_SWITCH_DISCRIPTOR_FORMTAT_EXTENDED,
}},
};
            layer3_switch_descriptor_t           g_ether0_rx_descriptor_array0[48];rmac_queue_info_t g_ether0_rx_queue_list[1] =
 {
{ .queue_cfg={.array_length       = 48,
.p_descriptor_array = g_ether0_rx_descriptor_array0,
.ports              = (1 << 0),
.type               = LAYER3_SWITCH_QUEUE_TYPE_RX,
.write_back_mode    = LAYER3_SWITCH_WRITE_BACK_MODE_FULL,
.descriptor_format  = LAYER3_SWITCH_DISCRIPTOR_FORMTAT_EXTENDED,
}},
};

            const rmac_extended_cfg_t g_ether0_extended_cfg_t =
            {
                .p_ether_switch      = &g_layer3_switch0,
                .tx_queue_num        = 1,
                .rx_queue_num        = 1,

                .p_tx_queue_list     = g_ether0_tx_queue_list,
                .p_rx_queue_list     = g_ether0_rx_queue_list,
#if defined(VECTOR_NUMBER_ETHER_RMPI0)
                .rmpi_irq                = VECTOR_NUMBER_ETHER_RMPI0,
#else
                .rmpi_irq                = FSP_INVALID_VECTOR,
#endif
                .rmpi_ipl                = (BSP_IRQ_DISABLED),
            };
            uint8_t g_ether0_ether_buffer0[1536];
uint8_t g_ether0_ether_buffer1[1536];
uint8_t g_ether0_ether_buffer2[1536];
uint8_t g_ether0_ether_buffer3[1536];
uint8_t g_ether0_ether_buffer4[1536];
uint8_t g_ether0_ether_buffer5[1536];
uint8_t g_ether0_ether_buffer6[1536];
uint8_t g_ether0_ether_buffer7[1536];
uint8_t g_ether0_ether_buffer8[1536];
uint8_t g_ether0_ether_buffer9[1536];
uint8_t g_ether0_ether_buffer10[1536];
uint8_t g_ether0_ether_buffer11[1536];
uint8_t g_ether0_ether_buffer12[1536];
uint8_t g_ether0_ether_buffer13[1536];
uint8_t g_ether0_ether_buffer14[1536];
uint8_t g_ether0_ether_buffer15[1536];
uint8_t g_ether0_ether_buffer16[1536];
uint8_t g_ether0_ether_buffer17[1536];
uint8_t g_ether0_ether_buffer18[1536];
uint8_t g_ether0_ether_buffer19[1536];
uint8_t g_ether0_ether_buffer20[1536];
uint8_t g_ether0_ether_buffer21[1536];
uint8_t g_ether0_ether_buffer22[1536];
uint8_t g_ether0_ether_buffer23[1536];
uint8_t g_ether0_ether_buffer24[1536];
uint8_t g_ether0_ether_buffer25[1536];
uint8_t g_ether0_ether_buffer26[1536];
uint8_t g_ether0_ether_buffer27[1536];
uint8_t g_ether0_ether_buffer28[1536];
uint8_t g_ether0_ether_buffer29[1536];
uint8_t g_ether0_ether_buffer30[1536];
uint8_t g_ether0_ether_buffer31[1536];
uint8_t g_ether0_ether_buffer32[1536];
uint8_t g_ether0_ether_buffer33[1536];
uint8_t g_ether0_ether_buffer34[1536];
uint8_t g_ether0_ether_buffer35[1536];
uint8_t g_ether0_ether_buffer36[1536];
uint8_t g_ether0_ether_buffer37[1536];
uint8_t g_ether0_ether_buffer38[1536];
uint8_t g_ether0_ether_buffer39[1536];
uint8_t g_ether0_ether_buffer40[1536];
uint8_t g_ether0_ether_buffer41[1536];
uint8_t g_ether0_ether_buffer42[1536];
uint8_t g_ether0_ether_buffer43[1536];
uint8_t g_ether0_ether_buffer44[1536];
uint8_t g_ether0_ether_buffer45[1536];
uint8_t g_ether0_ether_buffer46[1536];
uint8_t g_ether0_ether_buffer47[1536];
uint8_t g_ether0_ether_buffer48[1536];
uint8_t g_ether0_ether_buffer49[1536];
uint8_t g_ether0_ether_buffer50[1536];
uint8_t g_ether0_ether_buffer51[1536];
uint8_t g_ether0_ether_buffer52[1536];
uint8_t g_ether0_ether_buffer53[1536];
uint8_t g_ether0_ether_buffer54[1536];
uint8_t g_ether0_ether_buffer55[1536];
uint8_t g_ether0_ether_buffer56[1536];
uint8_t g_ether0_ether_buffer57[1536];
uint8_t g_ether0_ether_buffer58[1536];
uint8_t g_ether0_ether_buffer59[1536];
uint8_t g_ether0_ether_buffer60[1536];
uint8_t g_ether0_ether_buffer61[1536];

            uint8_t *pp_g_ether0_ether_buffers[62] = {
(uint8_t *) &g_ether0_ether_buffer0[0],
(uint8_t *) &g_ether0_ether_buffer1[0],
(uint8_t *) &g_ether0_ether_buffer2[0],
(uint8_t *) &g_ether0_ether_buffer3[0],
(uint8_t *) &g_ether0_ether_buffer4[0],
(uint8_t *) &g_ether0_ether_buffer5[0],
(uint8_t *) &g_ether0_ether_buffer6[0],
(uint8_t *) &g_ether0_ether_buffer7[0],
(uint8_t *) &g_ether0_ether_buffer8[0],
(uint8_t *) &g_ether0_ether_buffer9[0],
(uint8_t *) &g_ether0_ether_buffer10[0],
(uint8_t *) &g_ether0_ether_buffer11[0],
(uint8_t *) &g_ether0_ether_buffer12[0],
(uint8_t *) &g_ether0_ether_buffer13[0],
(uint8_t *) &g_ether0_ether_buffer14[0],
(uint8_t *) &g_ether0_ether_buffer15[0],
(uint8_t *) &g_ether0_ether_buffer16[0],
(uint8_t *) &g_ether0_ether_buffer17[0],
(uint8_t *) &g_ether0_ether_buffer18[0],
(uint8_t *) &g_ether0_ether_buffer19[0],
(uint8_t *) &g_ether0_ether_buffer20[0],
(uint8_t *) &g_ether0_ether_buffer21[0],
(uint8_t *) &g_ether0_ether_buffer22[0],
(uint8_t *) &g_ether0_ether_buffer23[0],
(uint8_t *) &g_ether0_ether_buffer24[0],
(uint8_t *) &g_ether0_ether_buffer25[0],
(uint8_t *) &g_ether0_ether_buffer26[0],
(uint8_t *) &g_ether0_ether_buffer27[0],
(uint8_t *) &g_ether0_ether_buffer28[0],
(uint8_t *) &g_ether0_ether_buffer29[0],
(uint8_t *) &g_ether0_ether_buffer30[0],
(uint8_t *) &g_ether0_ether_buffer31[0],
(uint8_t *) &g_ether0_ether_buffer32[0],
(uint8_t *) &g_ether0_ether_buffer33[0],
(uint8_t *) &g_ether0_ether_buffer34[0],
(uint8_t *) &g_ether0_ether_buffer35[0],
(uint8_t *) &g_ether0_ether_buffer36[0],
(uint8_t *) &g_ether0_ether_buffer37[0],
(uint8_t *) &g_ether0_ether_buffer38[0],
(uint8_t *) &g_ether0_ether_buffer39[0],
(uint8_t *) &g_ether0_ether_buffer40[0],
(uint8_t *) &g_ether0_ether_buffer41[0],
(uint8_t *) &g_ether0_ether_buffer42[0],
(uint8_t *) &g_ether0_ether_buffer43[0],
(uint8_t *) &g_ether0_ether_buffer44[0],
(uint8_t *) &g_ether0_ether_buffer45[0],
(uint8_t *) &g_ether0_ether_buffer46[0],
(uint8_t *) &g_ether0_ether_buffer47[0],
(uint8_t *) &g_ether0_ether_buffer48[0],
(uint8_t *) &g_ether0_ether_buffer49[0],
(uint8_t *) &g_ether0_ether_buffer50[0],
(uint8_t *) &g_ether0_ether_buffer51[0],
(uint8_t *) &g_ether0_ether_buffer52[0],
(uint8_t *) &g_ether0_ether_buffer53[0],
(uint8_t *) &g_ether0_ether_buffer54[0],
(uint8_t *) &g_ether0_ether_buffer55[0],
(uint8_t *) &g_ether0_ether_buffer56[0],
(uint8_t *) &g_ether0_ether_buffer57[0],
(uint8_t *) &g_ether0_ether_buffer58[0],
(uint8_t *) &g_ether0_ether_buffer59[0],
(uint8_t *) &g_ether0_ether_buffer60[0],
(uint8_t *) &g_ether0_ether_buffer61[0],
};
            const ether_cfg_t g_ether0_cfg =
            {
                .channel            = 0,
                .zerocopy           = ETHER_ZEROCOPY_DISABLE,
                .multicast          = ETHER_MULTICAST_ENABLE,
                .promiscuous        = ETHER_PROMISCUOUS_ENABLE,
                .flow_control       = ETHER_FLOW_CONTROL_DISABLE,
                .padding            = ETHER_PADDING_DISABLE,
                .padding_offset     = 0,
                .broadcast_filter   = 0,
                .p_mac_address      = g_ether0_mac_address,

                .num_tx_descriptors = 15,
                .num_rx_descriptors = 47,

                .pp_ether_buffers   = pp_g_ether0_ether_buffers,

                .ether_buffer_size  = 1536,

                .irq                = FSP_INVALID_VECTOR,

                .p_callback         = user_ether0_callback,
                .p_context          = NULL,
                .p_extend           = &g_ether0_extended_cfg_t,
            };

/* Instance structure to use this module. */
const ether_instance_t g_ether0 =
{
    .p_ctrl        = &g_ether0_ctrl,
    .p_cfg         = &g_ether0_cfg,
    .p_api         = &g_ether_on_rmac,
};
void g_hal_init(void) {
g_common_init();
}
