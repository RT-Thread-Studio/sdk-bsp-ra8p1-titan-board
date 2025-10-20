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
            [4] = glcdc_line_detect_isr, /* GLCDC LINE DETECT (Specified line) */
            [5] = drw_int_isr, /* DRW INT (DRW interrupt) */
            [6] = r_icu_isr, /* ICU IRQ28 (External pin interrupt 28) */
            [7] = iic_master_rxi_isr, /* IIC0 RXI (Receive data full) */
            [8] = iic_master_txi_isr, /* IIC0 TXI (Transmit data empty) */
            [9] = iic_master_tei_isr, /* IIC0 TEI (Transmit end) */
            [10] = iic_master_eri_isr, /* IIC0 ERI (Transfer error) */
        };
        #if BSP_FEATURE_ICU_HAS_IELSR
        const bsp_interrupt_event_t g_interrupt_event_link_select[BSP_ICU_VECTOR_NUM_ENTRIES] =
        {
            [0] = BSP_PRV_VECT_ENUM(EVENT_SCI8_RXI,GROUP0), /* SCI8 RXI (Receive data full) */
            [1] = BSP_PRV_VECT_ENUM(EVENT_SCI8_TXI,GROUP1), /* SCI8 TXI (Transmit data empty) */
            [2] = BSP_PRV_VECT_ENUM(EVENT_SCI8_TEI,GROUP2), /* SCI8 TEI (Transmit end) */
            [3] = BSP_PRV_VECT_ENUM(EVENT_SCI8_ERI,GROUP3), /* SCI8 ERI (Receive error) */
            [4] = BSP_PRV_VECT_ENUM(EVENT_GLCDC_LINE_DETECT,GROUP4), /* GLCDC LINE DETECT (Specified line) */
            [5] = BSP_PRV_VECT_ENUM(EVENT_DRW_INT,GROUP5), /* DRW INT (DRW interrupt) */
            [6] = BSP_PRV_VECT_ENUM(EVENT_ICU_IRQ28,GROUP6), /* ICU IRQ28 (External pin interrupt 28) */
            [7] = BSP_PRV_VECT_ENUM(EVENT_IIC0_RXI,GROUP7), /* IIC0 RXI (Receive data full) */
            [8] = BSP_PRV_VECT_ENUM(EVENT_IIC0_TXI,GROUP0), /* IIC0 TXI (Transmit data empty) */
            [9] = BSP_PRV_VECT_ENUM(EVENT_IIC0_TEI,GROUP1), /* IIC0 TEI (Transmit end) */
            [10] = BSP_PRV_VECT_ENUM(EVENT_IIC0_ERI,GROUP2), /* IIC0 ERI (Transfer error) */
        };
        #endif
        #endif