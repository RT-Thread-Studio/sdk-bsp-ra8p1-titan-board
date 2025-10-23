/* generated vector source file - do not edit */
        #include "bsp_api.h"
        /* Do not build these data structures if no interrupts are currently allocated because IAR will have build errors. */
        #if VECTOR_DATA_IRQ_COUNT > 0
        BSP_DONT_REMOVE const fsp_vector_t g_vector_table[BSP_ICU_VECTOR_NUM_ENTRIES] BSP_PLACE_IN_SECTION(BSP_SECTION_APPLICATION_VECTORS) =
        {
                        [0] = layer3_switch_gwdi_isr, /* ETHER GWDI0 (GWCA Data Interrupt 0) */
            [1] = layer3_switch_eaei_isr, /* ETHER EAEI0 (ETHA0 Error Interrupt) */
            [2] = layer3_switch_eaei_isr, /* ETHER EAEI1 (ETHA1 Error Interrupt) */
            [3] = sci_b_uart_rxi_isr, /* SCI8 RXI (Receive data full) */
            [4] = sci_b_uart_txi_isr, /* SCI8 TXI (Transmit data empty) */
            [5] = sci_b_uart_tei_isr, /* SCI8 TEI (Transmit end) */
            [6] = sci_b_uart_eri_isr, /* SCI8 ERI (Receive error) */
        };
        #if BSP_FEATURE_ICU_HAS_IELSR
        const bsp_interrupt_event_t g_interrupt_event_link_select[BSP_ICU_VECTOR_NUM_ENTRIES] =
        {
            [0] = BSP_PRV_VECT_ENUM(EVENT_ETHER_GWDI0,GROUP0), /* ETHER GWDI0 (GWCA Data Interrupt 0) */
            [1] = BSP_PRV_VECT_ENUM(EVENT_ETHER_EAEI0,GROUP1), /* ETHER EAEI0 (ETHA0 Error Interrupt) */
            [2] = BSP_PRV_VECT_ENUM(EVENT_ETHER_EAEI1,GROUP2), /* ETHER EAEI1 (ETHA1 Error Interrupt) */
            [3] = BSP_PRV_VECT_ENUM(EVENT_SCI8_RXI,GROUP3), /* SCI8 RXI (Receive data full) */
            [4] = BSP_PRV_VECT_ENUM(EVENT_SCI8_TXI,GROUP4), /* SCI8 TXI (Transmit data empty) */
            [5] = BSP_PRV_VECT_ENUM(EVENT_SCI8_TEI,GROUP5), /* SCI8 TEI (Transmit end) */
            [6] = BSP_PRV_VECT_ENUM(EVENT_SCI8_ERI,GROUP6), /* SCI8 ERI (Receive error) */
        };
        #endif
        #endif