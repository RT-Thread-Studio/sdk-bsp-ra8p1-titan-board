/* generated HAL source file - do not edit */
#include "hal_data.h"
/* Nominal and Data bit timing configuration */

can_bit_timing_cfg_t g_canfd1_bit_timing_cfg =
{
/* Actual bitrate: 500000 Hz. Actual sample point: 75 %. */
.baud_rate_prescaler = 1,
  .time_segment_1 = 119, .time_segment_2 = 40, .synchronization_jump_width = 4 };

#if BSP_FEATURE_CANFD_FD_SUPPORT
can_bit_timing_cfg_t g_canfd1_data_timing_cfg =
{
    /* Actual bitrate: 2000000 Hz. Actual sample point: 75 %. */
    .baud_rate_prescaler = 1,
    .time_segment_1 = 29,
    .time_segment_2 = 10,
    .synchronization_jump_width = 4
};
#endif

extern const canfd_afl_entry_t p_canfd1_afl[CANFD_CFG_AFL_CH1_RULE_NUM];

#define CANFD_CFG_COMMONFIFO0 (((0) << R_CANFD_CFDCFCC_CFE_Pos) | \
                                        ((0) << R_CANFD_CFDCFCC_CFRXIE_Pos) | \
                                        ((0) << R_CANFD_CFDCFCC_CFTXIE_Pos) | \
                                        ((0) << R_CANFD_CFDCFCC_CFPLS_Pos) | \
                                        ((0) << R_CANFD_CFDCFCC_CFM_Pos) | \
                                        ((0) << R_CANFD_CFDCFCC_CFITSS_Pos) | \
                                        ((0) << R_CANFD_CFDCFCC_CFITR_Pos) | \
                                        ((0)  << R_CANFD_CFDCFCC_CFIM_Pos) | \
                                        ((3U) << R_CANFD_CFDCFCC_CFIGCV_Pos) | \
                                        ((0) << R_CANFD_CFDCFCC_CFTML_Pos) | \
                                        ((3) << R_CANFD_CFDCFCC_CFDC_Pos) | \
                                        (0 << R_CANFD_CFDCFCC_CFITT_Pos))

/* Buffer RAM used: 640 bytes */
canfd_global_cfg_t g_canfd1_global_cfg =
{ .global_interrupts = (0x3), .global_config = ((R_CANFD_CFDGCFG_TPRI_Msk) | (0)
        | (BSP_CFG_CANFDCLK_SOURCE == BSP_CLOCKS_SOURCE_CLOCK_MAIN_OSC ? R_CANFD_CFDGCFG_DCS_Msk : 0U) | (0)
        | ((0) << R_CANFD_CFDGCFG_ITRCP_Pos)),
  .rx_mb_config = (0 | ((0) << R_CANFD_CFDRMNB_RMPLS_Pos)), .global_err_ipl = CANFD_CFG_GLOBAL_ERR_IPL, .rx_fifo_ipl =
          CANFD_CFG_RX_FIFO_IPL,
  .rx_fifo_config =
  { ((3U) << R_CANFD_CFDRFCC_RFIGCV_Pos) | ((3) << R_CANFD_CFDRFCC_RFDC_Pos) | ((0) << R_CANFD_CFDRFCC_RFPLS_Pos)
            | ((R_CANFD_CFDRFCC_RFIE_Msk | R_CANFD_CFDRFCC_RFIM_Msk)) | ((1)),
    ((3U) << R_CANFD_CFDRFCC_RFIGCV_Pos) | ((3) << R_CANFD_CFDRFCC_RFDC_Pos) | ((0) << R_CANFD_CFDRFCC_RFPLS_Pos)
            | ((R_CANFD_CFDRFCC_RFIE_Msk | R_CANFD_CFDRFCC_RFIM_Msk)) | ((1)) },
  .common_fifo_config =
  {
  CANFD_CFG_COMMONFIFO0 } };

canfd_extended_cfg_t g_canfd1_extended_cfg =
{ .p_afl = p_canfd1_afl, .txmb_txi_enable = ((1ULL << 0) | 0ULL), .error_interrupts = (0U),
#if BSP_FEATURE_CANFD_FD_SUPPORT
    .p_data_timing      = &g_canfd1_data_timing_cfg,
#else
  .p_data_timing = NULL,
#endif
  .delay_compensation = (1),
  .p_global_cfg = &g_canfd1_global_cfg, };

canfd_instance_ctrl_t g_canfd1_ctrl;
const can_cfg_t g_canfd1_cfg =
{ .channel = 1, .p_bit_timing = &g_canfd1_bit_timing_cfg, .p_callback = canfd1_callback, .p_extend =
          &g_canfd1_extended_cfg,
  .p_context = NULL, .ipl = (12),
#if defined(VECTOR_NUMBER_CAN1_COMFRX)
    .rx_irq             = VECTOR_NUMBER_CAN1_COMFRX,
#else
  .rx_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_CAN1_TX)
    .tx_irq             = VECTOR_NUMBER_CAN1_TX,
#else
  .tx_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_CAN1_CHERR)
    .error_irq             = VECTOR_NUMBER_CAN1_CHERR,
#else
  .error_irq = FSP_INVALID_VECTOR,
#endif
        };
/* Instance structure to use this module. */
const can_instance_t g_canfd1 =
{ .p_ctrl = &g_canfd1_ctrl, .p_cfg = &g_canfd1_cfg, .p_api = &g_canfd_on_canfd };
/* Nominal and Data bit timing configuration */

can_bit_timing_cfg_t g_canfd0_bit_timing_cfg =
{
/* Actual bitrate: 500000 Hz. Actual sample point: 75 %. */
.baud_rate_prescaler = 1,
  .time_segment_1 = 119, .time_segment_2 = 40, .synchronization_jump_width = 4 };

#if BSP_FEATURE_CANFD_FD_SUPPORT
can_bit_timing_cfg_t g_canfd0_data_timing_cfg =
{
    /* Actual bitrate: 2000000 Hz. Actual sample point: 75 %. */
    .baud_rate_prescaler = 1,
    .time_segment_1 = 29,
    .time_segment_2 = 10,
    .synchronization_jump_width = 4
};
#endif

extern const canfd_afl_entry_t p_canfd0_afl[CANFD_CFG_AFL_CH0_RULE_NUM];

#define CANFD_CFG_COMMONFIFO0 (((0) << R_CANFD_CFDCFCC_CFE_Pos) | \
                                        ((0) << R_CANFD_CFDCFCC_CFRXIE_Pos) | \
                                        ((0) << R_CANFD_CFDCFCC_CFTXIE_Pos) | \
                                        ((0) << R_CANFD_CFDCFCC_CFPLS_Pos) | \
                                        ((0) << R_CANFD_CFDCFCC_CFM_Pos) | \
                                        ((0) << R_CANFD_CFDCFCC_CFITSS_Pos) | \
                                        ((0) << R_CANFD_CFDCFCC_CFITR_Pos) | \
                                        ((0)  << R_CANFD_CFDCFCC_CFIM_Pos) | \
                                        ((3U) << R_CANFD_CFDCFCC_CFIGCV_Pos) | \
                                        ((0) << R_CANFD_CFDCFCC_CFTML_Pos) | \
                                        ((3) << R_CANFD_CFDCFCC_CFDC_Pos) | \
                                        (0 << R_CANFD_CFDCFCC_CFITT_Pos))

/* Buffer RAM used: 320 bytes */
canfd_global_cfg_t g_canfd0_global_cfg =
{ .global_interrupts = (0x3), .global_config = ((R_CANFD_CFDGCFG_TPRI_Msk) | (0)
        | (BSP_CFG_CANFDCLK_SOURCE == BSP_CLOCKS_SOURCE_CLOCK_MAIN_OSC ? R_CANFD_CFDGCFG_DCS_Msk : 0U) | (0)
        | ((0) << R_CANFD_CFDGCFG_ITRCP_Pos)),
  .rx_mb_config = (0 | ((0) << R_CANFD_CFDRMNB_RMPLS_Pos)), .global_err_ipl = CANFD_CFG_GLOBAL_ERR_IPL, .rx_fifo_ipl =
          CANFD_CFG_RX_FIFO_IPL,
  .rx_fifo_config =
  { ((3U) << R_CANFD_CFDRFCC_RFIGCV_Pos) | ((3) << R_CANFD_CFDRFCC_RFDC_Pos) | ((0) << R_CANFD_CFDRFCC_RFPLS_Pos)
            | ((R_CANFD_CFDRFCC_RFIE_Msk | R_CANFD_CFDRFCC_RFIM_Msk)) | ((1)),
    ((3U) << R_CANFD_CFDRFCC_RFIGCV_Pos) | ((3) << R_CANFD_CFDRFCC_RFDC_Pos) | ((0) << R_CANFD_CFDRFCC_RFPLS_Pos)
            | ((R_CANFD_CFDRFCC_RFIE_Msk | R_CANFD_CFDRFCC_RFIM_Msk)) | ((0)) },
  .common_fifo_config =
  {
  CANFD_CFG_COMMONFIFO0 } };

canfd_extended_cfg_t g_canfd0_extended_cfg =
{ .p_afl = p_canfd0_afl, .txmb_txi_enable = ((1ULL << 0) | 0ULL), .error_interrupts = (0U),
#if BSP_FEATURE_CANFD_FD_SUPPORT
    .p_data_timing      = &g_canfd0_data_timing_cfg,
#else
  .p_data_timing = NULL,
#endif
  .delay_compensation = (1),
  .p_global_cfg = &g_canfd0_global_cfg, };

canfd_instance_ctrl_t g_canfd0_ctrl;
const can_cfg_t g_canfd0_cfg =
{ .channel = 0, .p_bit_timing = &g_canfd0_bit_timing_cfg, .p_callback = canfd0_callback, .p_extend =
          &g_canfd0_extended_cfg,
  .p_context = NULL, .ipl = (12),
#if defined(VECTOR_NUMBER_CAN0_COMFRX)
    .rx_irq             = VECTOR_NUMBER_CAN0_COMFRX,
#else
  .rx_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_CAN0_TX)
    .tx_irq             = VECTOR_NUMBER_CAN0_TX,
#else
  .tx_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_CAN0_CHERR)
    .error_irq             = VECTOR_NUMBER_CAN0_CHERR,
#else
  .error_irq = FSP_INVALID_VECTOR,
#endif
        };
/* Instance structure to use this module. */
const can_instance_t g_canfd0 =
{ .p_ctrl = &g_canfd0_ctrl, .p_cfg = &g_canfd0_cfg, .p_api = &g_canfd_on_canfd };
sci_b_uart_instance_ctrl_t g_uart8_ctrl;

sci_b_baud_setting_t g_uart8_baud_setting =
        {
        /* Baud rate calculated with 0.160% error. */.baudrate_bits_b.abcse = 0,
          .baudrate_bits_b.abcs = 0, .baudrate_bits_b.bgdm = 1, .baudrate_bits_b.cks = 0, .baudrate_bits_b.brr = 64, .baudrate_bits_b.mddr =
                  (uint8_t) 256,
          .baudrate_bits_b.brme = false };

/** UART extended configuration for UARTonSCI HAL driver */
const sci_b_uart_extended_cfg_t g_uart8_cfg_extend =
{ .clock = SCI_B_UART_CLOCK_INT, .rx_edge_start = SCI_B_UART_START_BIT_FALLING_EDGE, .noise_cancel =
          SCI_B_UART_NOISE_CANCELLATION_DISABLE,
  .rx_fifo_trigger = SCI_B_UART_RX_FIFO_TRIGGER_MAX, .p_baud_setting = &g_uart8_baud_setting, .flow_control =
          SCI_B_UART_FLOW_CONTROL_RTS,
#if 0xFF != 0xFF
                .flow_control_pin       = BSP_IO_PORT_FF_PIN_0xFF,
                #else
  .flow_control_pin = (bsp_io_port_pin_t) UINT16_MAX,
#endif
  .rs485_setting =
  { .enable = SCI_B_UART_RS485_DISABLE,
    .polarity = SCI_B_UART_RS485_DE_POLARITY_HIGH,
    .assertion_time = 1,
    .negation_time = 1, } };

/** UART interface configuration */
const uart_cfg_t g_uart8_cfg =
{ .channel = 8, .data_bits = UART_DATA_BITS_8, .parity = UART_PARITY_OFF, .stop_bits = UART_STOP_BITS_1, .p_callback =
          user_uart8_callback,
  .p_context = NULL, .p_extend = &g_uart8_cfg_extend,
#define RA_NOT_DEFINED (1)
#if (RA_NOT_DEFINED == RA_NOT_DEFINED)
  .p_transfer_tx = NULL,
#else
                .p_transfer_tx       = &RA_NOT_DEFINED,
#endif
#if (RA_NOT_DEFINED == RA_NOT_DEFINED)
  .p_transfer_rx = NULL,
#else
                .p_transfer_rx       = &RA_NOT_DEFINED,
#endif
#undef RA_NOT_DEFINED
  .rxi_ipl = (12),
  .txi_ipl = (12), .tei_ipl = (12), .eri_ipl = (12),
#if defined(VECTOR_NUMBER_SCI8_RXI)
                .rxi_irq             = VECTOR_NUMBER_SCI8_RXI,
#else
  .rxi_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_SCI8_TXI)
                .txi_irq             = VECTOR_NUMBER_SCI8_TXI,
#else
  .txi_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_SCI8_TEI)
                .tei_irq             = VECTOR_NUMBER_SCI8_TEI,
#else
  .tei_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_SCI8_ERI)
                .eri_irq             = VECTOR_NUMBER_SCI8_ERI,
#else
  .eri_irq = FSP_INVALID_VECTOR,
#endif
        };

/* Instance structure to use this module. */
const uart_instance_t g_uart8 =
{ .p_ctrl = &g_uart8_ctrl, .p_cfg = &g_uart8_cfg, .p_api = &g_uart_on_sci_b };
void g_hal_init(void)
{
    g_common_init ();
}
