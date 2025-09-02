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
            [4] = adc_b_limclpi_isr, /* ADC LIMCLPI (Limiter clip interrupt with the limit table 0 to 7) */
            [5] = adc_b_err0_isr, /* ADC ERR0 (A/D converter unit 0 Error) */
            [6] = adc_b_err1_isr, /* ADC ERR1 (A/D converter unit 1 Error) */
            [7] = adc_b_resovf0_isr, /* ADC RESOVF0 (A/D conversion overflow on A/D converter unit 0) */
            [8] = adc_b_resovf1_isr, /* ADC RESOVF1 (A/D conversion overflow on A/D converter unit 1) */
            [9] = adc_b_calend0_isr, /* ADC CALEND0 (End of calibration of A/D converter unit 0) */
            [10] = adc_b_calend1_isr, /* ADC CALEND1 (End of calibration of A/D converter unit 1) */
            [11] = adc_b_adi0_isr, /* ADC ADI0 (End of A/D scanning operation(Gr.0)) */
            [12] = adc_b_adi1_isr, /* ADC ADI1 (End of A/D scanning operation(Gr.1)) */
            [13] = adc_b_adi2_isr, /* ADC ADI2 (End of A/D scanning operation(Gr.2)) */
            [14] = adc_b_adi3_isr, /* ADC ADI3 (End of A/D scanning operation(Gr.3)) */
            [15] = adc_b_adi4_isr, /* ADC ADI4 (End of A/D scanning operation(Gr.4)) */
            [16] = adc_b_fifoovf_isr, /* ADC FIFOOVF (FIFO data overflow) */
            [17] = adc_b_fiforeq0_isr, /* ADC FIFOREQ0 (FIFO data read request interrupt(Gr.0)) */
            [18] = adc_b_fiforeq1_isr, /* ADC FIFOREQ1 (FIFO data read request interrupt(Gr.1)) */
            [19] = adc_b_fiforeq2_isr, /* ADC FIFOREQ2 (FIFO data read request interrupt(Gr.2)) */
            [20] = adc_b_fiforeq3_isr, /* ADC FIFOREQ3 (FIFO data read request interrupt(Gr.3)) */
            [21] = adc_b_fiforeq4_isr, /* ADC FIFOREQ4 (FIFO data read request interrupt(Gr.4)) */
        };
        #if BSP_FEATURE_ICU_HAS_IELSR
        const bsp_interrupt_event_t g_interrupt_event_link_select[BSP_ICU_VECTOR_NUM_ENTRIES] =
        {
            [0] = BSP_PRV_VECT_ENUM(EVENT_SCI8_RXI,GROUP0), /* SCI8 RXI (Receive data full) */
            [1] = BSP_PRV_VECT_ENUM(EVENT_SCI8_TXI,GROUP1), /* SCI8 TXI (Transmit data empty) */
            [2] = BSP_PRV_VECT_ENUM(EVENT_SCI8_TEI,GROUP2), /* SCI8 TEI (Transmit end) */
            [3] = BSP_PRV_VECT_ENUM(EVENT_SCI8_ERI,GROUP3), /* SCI8 ERI (Receive error) */
            [4] = BSP_PRV_VECT_ENUM(EVENT_ADC_LIMCLPI,GROUP4), /* ADC LIMCLPI (Limiter clip interrupt with the limit table 0 to 7) */
            [5] = BSP_PRV_VECT_ENUM(EVENT_ADC_ERR0,GROUP5), /* ADC ERR0 (A/D converter unit 0 Error) */
            [6] = BSP_PRV_VECT_ENUM(EVENT_ADC_ERR1,GROUP6), /* ADC ERR1 (A/D converter unit 1 Error) */
            [7] = BSP_PRV_VECT_ENUM(EVENT_ADC_RESOVF0,GROUP7), /* ADC RESOVF0 (A/D conversion overflow on A/D converter unit 0) */
            [8] = BSP_PRV_VECT_ENUM(EVENT_ADC_RESOVF1,GROUP0), /* ADC RESOVF1 (A/D conversion overflow on A/D converter unit 1) */
            [9] = BSP_PRV_VECT_ENUM(EVENT_ADC_CALEND0,GROUP1), /* ADC CALEND0 (End of calibration of A/D converter unit 0) */
            [10] = BSP_PRV_VECT_ENUM(EVENT_ADC_CALEND1,GROUP2), /* ADC CALEND1 (End of calibration of A/D converter unit 1) */
            [11] = BSP_PRV_VECT_ENUM(EVENT_ADC_ADI0,GROUP3), /* ADC ADI0 (End of A/D scanning operation(Gr.0)) */
            [12] = BSP_PRV_VECT_ENUM(EVENT_ADC_ADI1,GROUP4), /* ADC ADI1 (End of A/D scanning operation(Gr.1)) */
            [13] = BSP_PRV_VECT_ENUM(EVENT_ADC_ADI2,GROUP5), /* ADC ADI2 (End of A/D scanning operation(Gr.2)) */
            [14] = BSP_PRV_VECT_ENUM(EVENT_ADC_ADI3,GROUP6), /* ADC ADI3 (End of A/D scanning operation(Gr.3)) */
            [15] = BSP_PRV_VECT_ENUM(EVENT_ADC_ADI4,GROUP7), /* ADC ADI4 (End of A/D scanning operation(Gr.4)) */
            [16] = BSP_PRV_VECT_ENUM(EVENT_ADC_FIFOOVF,GROUP0), /* ADC FIFOOVF (FIFO data overflow) */
            [17] = BSP_PRV_VECT_ENUM(EVENT_ADC_FIFOREQ0,GROUP1), /* ADC FIFOREQ0 (FIFO data read request interrupt(Gr.0)) */
            [18] = BSP_PRV_VECT_ENUM(EVENT_ADC_FIFOREQ1,GROUP2), /* ADC FIFOREQ1 (FIFO data read request interrupt(Gr.1)) */
            [19] = BSP_PRV_VECT_ENUM(EVENT_ADC_FIFOREQ2,GROUP3), /* ADC FIFOREQ2 (FIFO data read request interrupt(Gr.2)) */
            [20] = BSP_PRV_VECT_ENUM(EVENT_ADC_FIFOREQ3,GROUP4), /* ADC FIFOREQ3 (FIFO data read request interrupt(Gr.3)) */
            [21] = BSP_PRV_VECT_ENUM(EVENT_ADC_FIFOREQ4,GROUP5), /* ADC FIFOREQ4 (FIFO data read request interrupt(Gr.4)) */
        };
        #endif
        #endif
