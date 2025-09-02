// #include "serial_servo.h"
// #include <string.h>
// #include "hal_data.h"
// #include <rtthread.h>

// /* 全局变量 */
// SerialServoControllerTypeDef serial_servo_controller;
// //extern osSemaphoreId_t serial_servo_rx_completeHandle;

// static int serial_write_and_read(SerialServoControllerTypeDef *self, SerialServoCmdTypeDef *frame, bool tx_only)
// {
//     int ret = 0;

//     g_uart5_ctrl.p_reg->CCR1 |= 0x00100000;
//     g_uart5_ctrl.p_reg->CCR0 &= 0xFFFFFFEE;
//     g_uart5_ctrl.p_reg->CCR0 |= 0x00000011;

//     // Transmit mode
//     g_uart5_ctrl.p_reg->CCR0_b.TE = 1;
//     g_uart5_ctrl.p_reg->CCR0_b.RE = 0;
// //    Write_TDR(&self->tx_frame); // Write command frame
// //    Wait_Transmission_Complete(); // Poll or interrupt-based
//     if (!tx_only) {
//         // Receive mode (asynchronous)
//         g_uart5_ctrl.p_reg->CCR0_b.TE = 0;
//         g_uart5_ctrl.p_reg->CCR0_b.RE = 1;
//         // Use dummy transmission for synchronous mode if needed
// //        if (synchronous_mode)
// //        {
// //            g_uart2_ctrl.p_reg->CCR0_b.TE = 1;
// //            g_uart2_ctrl.p_reg->CCR0_b.RE = 1;
// //            Write_TDR(dummy_data); // Generate clock, discard data
// //        }
// //        // Wait for reception (e.g., semaphore or timeout)
// //        if (osSemaphoreAcquire(rx_completeHandle, self->proc_timeout) != osOK) {
// //            ret = -1;
// //        }
//     }
//     // Disable UART
//     g_uart5_ctrl.p_reg->CCR0_b.TE = 0;
//     g_uart5_ctrl.p_reg->CCR0_b.RE = 0;
//     return ret;
// }

// extern sci_b_uart_instance_ctrl_t g_uart5_ctrl;

// #include "r_sci_b_uart.h"

// /* SCI CCR3 register bit offsets and masks */
// #define SCI_B_UART_CCR3_CHAR_OFFSET            (8U)
// #define SCI_B_UART_CCR3_CHAR_MASK              (0x00000300U)
// #define SCI_B_UART_CCR3_CKE_OFFSET             (24U)
// #define SCI_B_UART_CCR3_CKE_MASK               (0x03000000U)

// /* Default SCI Register values. */
// #define SCI_B_UART_CCR2_DEFAULT_VALUE          (0xFF00FF04U)
// #define SCI_B_UART_FCR_DEFAULT_VALUE           (0x1F1F0000U)

// /* SCI CCR1 register bit offsets and masks */
// #define SCI_B_UART_CCR1_SPB2_OFFSET            (4U)
// #define SCI_B_UART_CCR1_SPB2_MASK              (0x00000030U)
// #define SCI_B_UART_CCR1_PARITY_OFFSET          (8U)
// #define SCI_B_UART_CCR1_PARITY_MASK            (0x00000300U)
// #define SCI_B_UART_CCR1_FLOW_CTSRTS_MASK       (0x00000003U)

// /* SCI FCR register bit masks */
// #define SCI_B_UART_FCR_TRIGGER_MASK            (0xF)
// #define SCI_B_UART_FCR_RESET_TX_RX             (0x00808000U)

// static void r_sci_b_uart_fifo_cfg_b (sci_b_uart_instance_ctrl_t * const p_ctrl)
// {
//     if (0U != p_ctrl->fifo_depth)
//     {
//         uint32_t fcr = 0U;

//         /* Set the tx and rx reset bits */
//         p_ctrl->p_reg->FCR = (uint32_t) (SCI_B_UART_FCR_RESET_TX_RX);

//  #if SCI_B_UART_CFG_RX_ENABLE
//   #if SCI_B_UART_CFG_DTC_SUPPORTED

//         /* If DTC is used keep the receive trigger at the default level of 0. */
//         if (NULL == p_ctrl->p_cfg->p_transfer_rx)
//   #endif
//         {
//             /* Otherwise, set receive trigger number as configured by the user. */
//             sci_b_uart_extended_cfg_t const * p_extend = p_ctrl->p_cfg->p_extend;

//             /* RTRG(Receive FIFO Data Trigger Number) controls when the RXI interrupt will be generated. If data is
//              * received but the trigger number is not met the RXI interrupt will be generated after 15 ETUs from
//              * the last stop bit in asynchronous mode. For more information see the FIFO Selected section of "Serial
//              * Data Reception in Asynchronous Mode" in the RA6T2 manual R01UH0951EJ0100 or the relevant section for
//              * the MCU being used. */
//             fcr |= (((p_ctrl->fifo_depth - 1U) & p_extend->rx_fifo_trigger) & SCI_B_UART_FCR_TRIGGER_MASK) <<
//                    R_SCI_B0_FCR_RTRG_Pos;
//         }

//         /* RTS asserts when the amount of received data stored in the fifo is equal or less than this value. */
//         fcr |= ((p_ctrl->fifo_depth - 1U) & SCI_B_UART_FCR_TRIGGER_MASK) << R_SCI_B0_FCR_RSTRG_Pos;
//  #endif

//         /* Set the FCR and reset the fifos. */
//         p_ctrl->p_reg->FCR |= fcr;
//     }
// }

// static void r_sci_b_uart_config_set_b (sci_b_uart_instance_ctrl_t * const p_ctrl, uart_cfg_t const * const p_cfg)
// {
//     sci_b_uart_extended_cfg_t * p_extend       = (sci_b_uart_extended_cfg_t *) p_cfg->p_extend;
//     sci_b_baud_setting_t      * p_baud_setting = p_extend->p_baud_setting;
//     uint32_t ccr2 = 0U;

//     /* Set Character length, Stop bit length, Start bit edge detection and SCI mode as Asynchronous mode. */
//     uint32_t ccr3 = (uint32_t) R_SCI_B0_CCR3_LSBF_Msk;
//     ccr3 |= ((uint32_t) p_cfg->data_bits << SCI_B_UART_CCR3_CHAR_OFFSET) & SCI_B_UART_CCR3_CHAR_MASK;
//     ccr3 |= ((uint32_t) p_cfg->stop_bits << R_SCI_B0_CCR3_STP_Pos) & R_SCI_B0_CCR3_STP_Msk;
//     ccr3 |= ((uint32_t) p_extend->rx_edge_start << R_SCI_B0_CCR3_RXDESEL_Pos) & R_SCI_B0_CCR3_RXDESEL_Msk;
//     ccr3 |= ((uint32_t) p_extend->rs485_setting.enable << R_SCI_B0_CCR3_DEN_Pos) & R_SCI_B0_CCR3_DEN_Msk;
//     ccr3 |= ((uint32_t) p_extend->clock << SCI_B_UART_CCR3_CKE_OFFSET) & SCI_B_UART_CCR3_CKE_MASK;
// #if SCI_B_UART_CFG_FIFO_SUPPORT
//     if (p_ctrl->fifo_depth > 0U)
//     {
//         ccr3 |= (1U << R_SCI_B0_CCR3_FM_Pos) & R_SCI_B0_CCR3_FM_Msk;
//     }
// #endif
//     p_ctrl->p_reg->CCR3 = ccr3;

//     if ((SCI_B_UART_CLOCK_EXT8X == p_extend->clock) || (SCI_B_UART_CLOCK_EXT16X == p_extend->clock))
//     {
//         /* Use external clock for baud rate */
//         ccr2 = SCI_B_UART_CCR2_DEFAULT_VALUE;

//         if (SCI_B_UART_CLOCK_EXT8X == p_extend->clock)
//         {
//             /* Set baud rate as (external clock / 8) */
//             ccr2 |= 1U << R_SCI_B0_CCR2_ABCS_Pos;
//         }
//     }
//     else
//     {
//         /* Set the baud rate settings for the internal baud rate generator. */
//         ccr2 |= p_baud_setting->baudrate_bits;
//     }

//     p_ctrl->p_reg->CCR2 = ccr2;

//     /* Configure flow control pin. */
//     uint32_t ccr1 = ((uint32_t) (p_extend->flow_control << R_SCI_B0_CCR1_CTSE_Pos) & SCI_B_UART_CCR1_FLOW_CTSRTS_MASK);

//     /* TXD pin is at high level when TE is 0. */
//     ccr1 |= (3U << SCI_B_UART_CCR1_SPB2_OFFSET) & SCI_B_UART_CCR1_SPB2_MASK;

//     if (0 != p_cfg->parity)
//     {
//         ccr1 |=
//             (((UART_PARITY_EVEN ==
//                p_cfg->parity) ? 1U : 3U) << SCI_B_UART_CCR1_PARITY_OFFSET) & SCI_B_UART_CCR1_PARITY_MASK;
//     }

//     ccr1 |= ((uint32_t) p_extend->noise_cancel << R_SCI_B0_CCR1_NFEN_Pos) &
//             R_SCI_B0_CCR1_NFEN_Msk;
//     p_ctrl->p_reg->CCR1 = ccr1;

//     p_ctrl->p_reg->CCR4 = 0U;

// #if SCI_B_UART_CFG_FIFO_SUPPORT

//     /* Configure FIFO related registers. */
//     r_sci_b_uart_fifo_cfg_b(p_ctrl);
// #else

//     /* If fifo support is disabled and the current channel supports fifo set FCR to default */
//     if (BSP_FEATURE_SCI_UART_FIFO_CHANNELS & (1U << p_cfg->channel))
//     {
//         p_ctrl->p_reg->FCR = SCI_B_UART_FCR_DEFAULT_VALUE;
//     }
// #endif

//     /* Configure RS-485 DE assertion settings. */
//     uint32_t dcr = ((uint32_t) (p_extend->rs485_setting.polarity << R_SCI_B0_DCR_DEPOL_Pos)) & R_SCI_B0_DCR_DEPOL_Msk;
//     dcr |= ((uint32_t) p_extend->rs485_setting.assertion_time << R_SCI_B0_DCR_DEAST_Pos) &
//            R_SCI_B0_DCR_DEAST_Msk;
//     dcr |= ((uint32_t) p_extend->rs485_setting.negation_time << R_SCI_B0_DCR_DENGT_Pos) &
//            R_SCI_B0_DCR_DENGT_Msk;
//     p_ctrl->p_reg->DCR = dcr;
// }

// void serial_servo_init(void)
// {
//     serial_servo_controller_object_init(&serial_servo_controller);
//     serial_servo_controller.proc_timeout = 5;
//     serial_servo_controller.serial_write_and_read = serial_write_and_read;

// //    uint32_t ccr0 = R_SCI_B0_CCR0_IDSEL_Msk;
// //
// //    g_uart2_ctrl.p_reg->CCR0 = ccr0;

//     g_uart5_ctrl.p_reg->CCR1 |= 0x00100000;
//     g_uart5_ctrl.p_reg->CCR0 &= 0xFFFFFFEE;
//     g_uart5_ctrl.p_reg->CCR0 |= 0x00000011;

//     // Configure UART for half-duplex
// //    g_uart2_ctrl.p_reg->CCR1_b.SHARPS = 1; // Enable half-duplex mode
// //    // Disable TE and RE initially
// //    g_uart2_ctrl.p_reg->CCR0_b.TE = 0;
// //    g_uart2_ctrl.p_reg->CCR0_b.RE = 0;

// //    r_sci_b_uart_config_set_b(&g_uart2_ctrl, &g_uart2_cfg);
//     // Enable RX interrupt
// //    R_BSP_IrqStatusClear(5);
// }
