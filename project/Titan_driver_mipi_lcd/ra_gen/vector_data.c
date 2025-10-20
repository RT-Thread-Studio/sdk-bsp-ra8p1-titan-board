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
            [5] = mipi_dsi_seq0_isr, /* MIPIDSI SEQ0 (Sequence operation channel 0 interrupt) */
            [6] = mipi_dsi_seq1_isr, /* MIPIDSI SEQ1 (Sequence operation channel 1 interrupt) */
            [7] = mipi_dsi_vin1_isr, /* MIPIDSI VIN1 (Video-Input operation channel1 interrupt) */
            [8] = mipi_dsi_rcv_isr, /* MIPIDSI RCV (DSI packet receive interrupt) */
            [9] = mipi_dsi_ferr_isr, /* MIPIDSI FERR (DSI fatal error interrupt) */
            [10] = mipi_dsi_ppi_isr, /* MIPIDSI PPI (DSI D-PHY PPI interrupt) */
            [11] = drw_int_isr, /* DRW INT (DRW interrupt) */
        };
        #if BSP_FEATURE_ICU_HAS_IELSR
        const bsp_interrupt_event_t g_interrupt_event_link_select[BSP_ICU_VECTOR_NUM_ENTRIES] =
        {
            [0] = BSP_PRV_VECT_ENUM(EVENT_SCI8_RXI,GROUP0), /* SCI8 RXI (Receive data full) */
            [1] = BSP_PRV_VECT_ENUM(EVENT_SCI8_TXI,GROUP1), /* SCI8 TXI (Transmit data empty) */
            [2] = BSP_PRV_VECT_ENUM(EVENT_SCI8_TEI,GROUP2), /* SCI8 TEI (Transmit end) */
            [3] = BSP_PRV_VECT_ENUM(EVENT_SCI8_ERI,GROUP3), /* SCI8 ERI (Receive error) */
            [4] = BSP_PRV_VECT_ENUM(EVENT_GLCDC_LINE_DETECT,GROUP4), /* GLCDC LINE DETECT (Specified line) */
            [5] = BSP_PRV_VECT_ENUM(EVENT_MIPIDSI_SEQ0,GROUP5), /* MIPIDSI SEQ0 (Sequence operation channel 0 interrupt) */
            [6] = BSP_PRV_VECT_ENUM(EVENT_MIPIDSI_SEQ1,GROUP6), /* MIPIDSI SEQ1 (Sequence operation channel 1 interrupt) */
            [7] = BSP_PRV_VECT_ENUM(EVENT_MIPIDSI_VIN1,GROUP7), /* MIPIDSI VIN1 (Video-Input operation channel1 interrupt) */
            [8] = BSP_PRV_VECT_ENUM(EVENT_MIPIDSI_RCV,GROUP0), /* MIPIDSI RCV (DSI packet receive interrupt) */
            [9] = BSP_PRV_VECT_ENUM(EVENT_MIPIDSI_FERR,GROUP1), /* MIPIDSI FERR (DSI fatal error interrupt) */
            [10] = BSP_PRV_VECT_ENUM(EVENT_MIPIDSI_PPI,GROUP2), /* MIPIDSI PPI (DSI D-PHY PPI interrupt) */
            [11] = BSP_PRV_VECT_ENUM(EVENT_DRW_INT,GROUP3), /* DRW INT (DRW interrupt) */
        };
        #endif
        #endif