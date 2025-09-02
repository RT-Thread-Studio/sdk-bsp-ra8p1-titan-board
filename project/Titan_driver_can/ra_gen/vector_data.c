/* generated vector source file - do not edit */
#include "bsp_api.h"
/* Do not build these data structures if no interrupts are currently allocated because IAR will have build errors. */
#if VECTOR_DATA_IRQ_COUNT > 0
        BSP_DONT_REMOVE const fsp_vector_t g_vector_table[BSP_ICU_VECTOR_NUM_ENTRIES] BSP_PLACE_IN_SECTION(BSP_SECTION_APPLICATION_VECTORS) =
        {
                        [0] = sci_b_uart_rxi_isr, /* SCI8 RXI (Receive data full) */
            [1] = sci_b_uart_txi_isr, /* SCI8 TXI (Transmit data empty) */
            [2] = sci_b_uart_tei_isr, /* SCI8 TEI (Transmit end) */
            [3] = sci_b_uart_eri_isr, /* SCI8 ERI (Receive error) */
            [4] = canfd_error_isr, /* CAN0 CHERR (Channel  error) */
            [5] = canfd_channel_tx_isr, /* CAN0 TX (Transmit interrupt) */
            [6] = canfd_common_fifo_rx_isr, /* CAN0 COMFRX (Common FIFO receive interrupt) */
            [7] = canfd_error_isr, /* CAN GLERR (Global error) */
            [8] = canfd_rx_fifo_isr, /* CAN RXF (Global receive FIFO interrupt) */
            [9] = canfd_error_isr, /* CAN1 CHERR (Channel  error) */
            [10] = canfd_channel_tx_isr, /* CAN1 TX (Transmit interrupt) */
            [11] = canfd_common_fifo_rx_isr, /* CAN1 COMFRX (Common FIFO receive interrupt) */
        };
        #if BSP_FEATURE_ICU_HAS_IELSR
        const bsp_interrupt_event_t g_interrupt_event_link_select[BSP_ICU_VECTOR_NUM_ENTRIES] =
        {
            [0] = BSP_PRV_VECT_ENUM(EVENT_SCI8_RXI,GROUP0), /* SCI8 RXI (Receive data full) */
            [1] = BSP_PRV_VECT_ENUM(EVENT_SCI8_TXI,GROUP1), /* SCI8 TXI (Transmit data empty) */
            [2] = BSP_PRV_VECT_ENUM(EVENT_SCI8_TEI,GROUP2), /* SCI8 TEI (Transmit end) */
            [3] = BSP_PRV_VECT_ENUM(EVENT_SCI8_ERI,GROUP3), /* SCI8 ERI (Receive error) */
            [4] = BSP_PRV_VECT_ENUM(EVENT_CAN0_CHERR,GROUP4), /* CAN0 CHERR (Channel  error) */
            [5] = BSP_PRV_VECT_ENUM(EVENT_CAN0_TX,GROUP5), /* CAN0 TX (Transmit interrupt) */
            [6] = BSP_PRV_VECT_ENUM(EVENT_CAN0_COMFRX,GROUP6), /* CAN0 COMFRX (Common FIFO receive interrupt) */
            [7] = BSP_PRV_VECT_ENUM(EVENT_CAN_GLERR,GROUP7), /* CAN GLERR (Global error) */
            [8] = BSP_PRV_VECT_ENUM(EVENT_CAN_RXF,GROUP0), /* CAN RXF (Global receive FIFO interrupt) */
            [9] = BSP_PRV_VECT_ENUM(EVENT_CAN1_CHERR,GROUP1), /* CAN1 CHERR (Channel  error) */
            [10] = BSP_PRV_VECT_ENUM(EVENT_CAN1_TX,GROUP2), /* CAN1 TX (Transmit interrupt) */
            [11] = BSP_PRV_VECT_ENUM(EVENT_CAN1_COMFRX,GROUP3), /* CAN1 COMFRX (Common FIFO receive interrupt) */
        };
        #endif
        #endif
