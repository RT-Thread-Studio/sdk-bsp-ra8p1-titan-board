/* generated vector source file - do not edit */
        #include "bsp_api.h"
        /* Do not build these data structures if no interrupts are currently allocated because IAR will have build errors. */
        #if VECTOR_DATA_IRQ_COUNT > 0
        BSP_DONT_REMOVE const fsp_vector_t g_vector_table[BSP_ICU_VECTOR_NUM_ENTRIES] BSP_PLACE_IN_SECTION(BSP_SECTION_APPLICATION_VECTORS) =
        {
                        [0] = ceu_isr, /* CEU CEUI (CEU interrupt) */
            [1] = sci_b_uart_rxi_isr, /* SCI8 RXI (Receive data full) */
            [2] = sci_b_uart_txi_isr, /* SCI8 TXI (Transmit data empty) */
            [3] = sci_b_uart_tei_isr, /* SCI8 TEI (Transmit end) */
            [4] = sci_b_uart_eri_isr, /* SCI8 ERI (Receive error) */
            [5] = glcdc_line_detect_isr, /* GLCDC LINE DETECT (Specified line) */
            [6] = drw_int_isr, /* DRW INT (DRW interrupt) */
            [7] = r_icu_isr, /* ICU IRQ26 (External pin interrupt 26) */
            [8] = sci_b_uart_rxi_isr, /* SCI5 RXI (Receive data full) */
            [9] = sci_b_uart_txi_isr, /* SCI5 TXI (Transmit data empty) */
            [10] = sci_b_uart_tei_isr, /* SCI5 TEI (Transmit end) */
            [11] = sci_b_uart_eri_isr, /* SCI5 ERI (Receive error) */
            [12] = r_icu_isr, /* ICU IRQ20 (External pin interrupt 20) */
        };
        #if BSP_FEATURE_ICU_HAS_IELSR
        const bsp_interrupt_event_t g_interrupt_event_link_select[BSP_ICU_VECTOR_NUM_ENTRIES] =
        {
            [0] = BSP_PRV_VECT_ENUM(EVENT_CEU_CEUI,GROUP0), /* CEU CEUI (CEU interrupt) */
            [1] = BSP_PRV_VECT_ENUM(EVENT_SCI8_RXI,GROUP1), /* SCI8 RXI (Receive data full) */
            [2] = BSP_PRV_VECT_ENUM(EVENT_SCI8_TXI,GROUP2), /* SCI8 TXI (Transmit data empty) */
            [3] = BSP_PRV_VECT_ENUM(EVENT_SCI8_TEI,GROUP3), /* SCI8 TEI (Transmit end) */
            [4] = BSP_PRV_VECT_ENUM(EVENT_SCI8_ERI,GROUP4), /* SCI8 ERI (Receive error) */
            [5] = BSP_PRV_VECT_ENUM(EVENT_GLCDC_LINE_DETECT,GROUP5), /* GLCDC LINE DETECT (Specified line) */
            [6] = BSP_PRV_VECT_ENUM(EVENT_DRW_INT,GROUP6), /* DRW INT (DRW interrupt) */
            [7] = BSP_PRV_VECT_ENUM(EVENT_ICU_IRQ26,GROUP7), /* ICU IRQ26 (External pin interrupt 26) */
            [8] = BSP_PRV_VECT_ENUM(EVENT_SCI5_RXI,GROUP0), /* SCI5 RXI (Receive data full) */
            [9] = BSP_PRV_VECT_ENUM(EVENT_SCI5_TXI,GROUP1), /* SCI5 TXI (Transmit data empty) */
            [10] = BSP_PRV_VECT_ENUM(EVENT_SCI5_TEI,GROUP2), /* SCI5 TEI (Transmit end) */
            [11] = BSP_PRV_VECT_ENUM(EVENT_SCI5_ERI,GROUP3), /* SCI5 ERI (Receive error) */
            [12] = BSP_PRV_VECT_ENUM(EVENT_ICU_IRQ20,GROUP4), /* ICU IRQ20 (External pin interrupt 20) */
        };
        #endif
        #endif