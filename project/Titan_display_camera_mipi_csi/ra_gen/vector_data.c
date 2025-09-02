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
            [4] = iic_master_rxi_isr, /* IIC0 RXI (Receive data full) */
            [5] = iic_master_txi_isr, /* IIC0 TXI (Transmit data empty) */
            [6] = iic_master_tei_isr, /* IIC0 TEI (Transmit end) */
            [7] = iic_master_eri_isr, /* IIC0 ERI (Transfer error) */
            [8] = glcdc_line_detect_isr, /* GLCDC LINE DETECT (Specified line) */
            [9] = drw_int_isr, /* DRW INT (DRW interrupt) */
            [10] = vin_status_isr, /* VIN IRQ (Interrupt Request) */
            [11] = vin_error_isr, /* VIN ERR (Interrupt Request for SYNC Error) */
            [12] = mipi_csi_rx_isr, /* MIPICSI RX (Receive interrupt) */
            [13] = mipi_csi_dl_isr, /* MIPICSI DL (Data Lane interrupt) */
            [14] = mipi_csi_vc_isr, /* MIPICSI VC (Virtual Channel interrupt) */
            [15] = mipi_csi_pm_isr, /* MIPICSI PM (Power Management interrupt) */
            [16] = mipi_csi_gst_isr, /* MIPICSI GST (Generic Short Packet interrupt) */
            [17] = gpt_counter_overflow_isr, /* GPT0 COUNTER OVERFLOW (Overflow) */
        };
        #if BSP_FEATURE_ICU_HAS_IELSR
        const bsp_interrupt_event_t g_interrupt_event_link_select[BSP_ICU_VECTOR_NUM_ENTRIES] =
        {
            [0] = BSP_PRV_VECT_ENUM(EVENT_SCI8_RXI,GROUP0), /* SCI8 RXI (Receive data full) */
            [1] = BSP_PRV_VECT_ENUM(EVENT_SCI8_TXI,GROUP1), /* SCI8 TXI (Transmit data empty) */
            [2] = BSP_PRV_VECT_ENUM(EVENT_SCI8_TEI,GROUP2), /* SCI8 TEI (Transmit end) */
            [3] = BSP_PRV_VECT_ENUM(EVENT_SCI8_ERI,GROUP3), /* SCI8 ERI (Receive error) */
            [4] = BSP_PRV_VECT_ENUM(EVENT_IIC0_RXI,GROUP4), /* IIC0 RXI (Receive data full) */
            [5] = BSP_PRV_VECT_ENUM(EVENT_IIC0_TXI,GROUP5), /* IIC0 TXI (Transmit data empty) */
            [6] = BSP_PRV_VECT_ENUM(EVENT_IIC0_TEI,GROUP6), /* IIC0 TEI (Transmit end) */
            [7] = BSP_PRV_VECT_ENUM(EVENT_IIC0_ERI,GROUP7), /* IIC0 ERI (Transfer error) */
            [8] = BSP_PRV_VECT_ENUM(EVENT_GLCDC_LINE_DETECT,GROUP0), /* GLCDC LINE DETECT (Specified line) */
            [9] = BSP_PRV_VECT_ENUM(EVENT_DRW_INT,GROUP1), /* DRW INT (DRW interrupt) */
            [10] = BSP_PRV_VECT_ENUM(EVENT_VIN_IRQ,GROUP2), /* VIN IRQ (Interrupt Request) */
            [11] = BSP_PRV_VECT_ENUM(EVENT_VIN_ERR,GROUP3), /* VIN ERR (Interrupt Request for SYNC Error) */
            [12] = BSP_PRV_VECT_ENUM(EVENT_MIPICSI_RX,GROUP4), /* MIPICSI RX (Receive interrupt) */
            [13] = BSP_PRV_VECT_ENUM(EVENT_MIPICSI_DL,GROUP5), /* MIPICSI DL (Data Lane interrupt) */
            [14] = BSP_PRV_VECT_ENUM(EVENT_MIPICSI_VC,GROUP6), /* MIPICSI VC (Virtual Channel interrupt) */
            [15] = BSP_PRV_VECT_ENUM(EVENT_MIPICSI_PM,GROUP7), /* MIPICSI PM (Power Management interrupt) */
            [16] = BSP_PRV_VECT_ENUM(EVENT_MIPICSI_GST,GROUP0), /* MIPICSI GST (Generic Short Packet interrupt) */
            [17] = BSP_PRV_VECT_ENUM(EVENT_GPT0_COUNTER_OVERFLOW,GROUP1), /* GPT0 COUNTER OVERFLOW (Overflow) */
        };
        #endif
        #endif