/* generated vector header file - do not edit */
#ifndef VECTOR_DATA_H
#define VECTOR_DATA_H
#ifdef __cplusplus
        extern "C" {
        #endif
/* Number of interrupts allocated */
#ifndef VECTOR_DATA_IRQ_COUNT
#define VECTOR_DATA_IRQ_COUNT    (22)
#endif
/* ISR prototypes */
void sci_b_uart_rxi_isr(void);
void sci_b_uart_txi_isr(void);
void sci_b_uart_tei_isr(void);
void sci_b_uart_eri_isr(void);
void adc_b_limclpi_isr(void);
void adc_b_err0_isr(void);
void adc_b_err1_isr(void);
void adc_b_resovf0_isr(void);
void adc_b_resovf1_isr(void);
void adc_b_calend0_isr(void);
void adc_b_calend1_isr(void);
void adc_b_adi0_isr(void);
void adc_b_adi1_isr(void);
void adc_b_adi2_isr(void);
void adc_b_adi3_isr(void);
void adc_b_adi4_isr(void);
void adc_b_fifoovf_isr(void);
void adc_b_fiforeq0_isr(void);
void adc_b_fiforeq1_isr(void);
void adc_b_fiforeq2_isr(void);
void adc_b_fiforeq3_isr(void);
void adc_b_fiforeq4_isr(void);

/* Vector table allocations */
#define VECTOR_NUMBER_SCI8_RXI ((IRQn_Type) 0) /* SCI8 RXI (Receive data full) */
#define SCI8_RXI_IRQn          ((IRQn_Type) 0) /* SCI8 RXI (Receive data full) */
#define VECTOR_NUMBER_SCI8_TXI ((IRQn_Type) 1) /* SCI8 TXI (Transmit data empty) */
#define SCI8_TXI_IRQn          ((IRQn_Type) 1) /* SCI8 TXI (Transmit data empty) */
#define VECTOR_NUMBER_SCI8_TEI ((IRQn_Type) 2) /* SCI8 TEI (Transmit end) */
#define SCI8_TEI_IRQn          ((IRQn_Type) 2) /* SCI8 TEI (Transmit end) */
#define VECTOR_NUMBER_SCI8_ERI ((IRQn_Type) 3) /* SCI8 ERI (Receive error) */
#define SCI8_ERI_IRQn          ((IRQn_Type) 3) /* SCI8 ERI (Receive error) */
#define VECTOR_NUMBER_ADC_LIMCLPI ((IRQn_Type) 4) /* ADC LIMCLPI (Limiter clip interrupt with the limit table 0 to 7) */
#define ADC_LIMCLPI_IRQn          ((IRQn_Type) 4) /* ADC LIMCLPI (Limiter clip interrupt with the limit table 0 to 7) */
#define VECTOR_NUMBER_ADC_ERR0 ((IRQn_Type) 5) /* ADC ERR0 (A/D converter unit 0 Error) */
#define ADC_ERR0_IRQn          ((IRQn_Type) 5) /* ADC ERR0 (A/D converter unit 0 Error) */
#define VECTOR_NUMBER_ADC_ERR1 ((IRQn_Type) 6) /* ADC ERR1 (A/D converter unit 1 Error) */
#define ADC_ERR1_IRQn          ((IRQn_Type) 6) /* ADC ERR1 (A/D converter unit 1 Error) */
#define VECTOR_NUMBER_ADC_RESOVF0 ((IRQn_Type) 7) /* ADC RESOVF0 (A/D conversion overflow on A/D converter unit 0) */
#define ADC_RESOVF0_IRQn          ((IRQn_Type) 7) /* ADC RESOVF0 (A/D conversion overflow on A/D converter unit 0) */
#define VECTOR_NUMBER_ADC_RESOVF1 ((IRQn_Type) 8) /* ADC RESOVF1 (A/D conversion overflow on A/D converter unit 1) */
#define ADC_RESOVF1_IRQn          ((IRQn_Type) 8) /* ADC RESOVF1 (A/D conversion overflow on A/D converter unit 1) */
#define VECTOR_NUMBER_ADC_CALEND0 ((IRQn_Type) 9) /* ADC CALEND0 (End of calibration of A/D converter unit 0) */
#define ADC_CALEND0_IRQn          ((IRQn_Type) 9) /* ADC CALEND0 (End of calibration of A/D converter unit 0) */
#define VECTOR_NUMBER_ADC_CALEND1 ((IRQn_Type) 10) /* ADC CALEND1 (End of calibration of A/D converter unit 1) */
#define ADC_CALEND1_IRQn          ((IRQn_Type) 10) /* ADC CALEND1 (End of calibration of A/D converter unit 1) */
#define VECTOR_NUMBER_ADC_ADI0 ((IRQn_Type) 11) /* ADC ADI0 (End of A/D scanning operation(Gr.0)) */
#define ADC_ADI0_IRQn          ((IRQn_Type) 11) /* ADC ADI0 (End of A/D scanning operation(Gr.0)) */
#define VECTOR_NUMBER_ADC_ADI1 ((IRQn_Type) 12) /* ADC ADI1 (End of A/D scanning operation(Gr.1)) */
#define ADC_ADI1_IRQn          ((IRQn_Type) 12) /* ADC ADI1 (End of A/D scanning operation(Gr.1)) */
#define VECTOR_NUMBER_ADC_ADI2 ((IRQn_Type) 13) /* ADC ADI2 (End of A/D scanning operation(Gr.2)) */
#define ADC_ADI2_IRQn          ((IRQn_Type) 13) /* ADC ADI2 (End of A/D scanning operation(Gr.2)) */
#define VECTOR_NUMBER_ADC_ADI3 ((IRQn_Type) 14) /* ADC ADI3 (End of A/D scanning operation(Gr.3)) */
#define ADC_ADI3_IRQn          ((IRQn_Type) 14) /* ADC ADI3 (End of A/D scanning operation(Gr.3)) */
#define VECTOR_NUMBER_ADC_ADI4 ((IRQn_Type) 15) /* ADC ADI4 (End of A/D scanning operation(Gr.4)) */
#define ADC_ADI4_IRQn          ((IRQn_Type) 15) /* ADC ADI4 (End of A/D scanning operation(Gr.4)) */
#define VECTOR_NUMBER_ADC_FIFOOVF ((IRQn_Type) 16) /* ADC FIFOOVF (FIFO data overflow) */
#define ADC_FIFOOVF_IRQn          ((IRQn_Type) 16) /* ADC FIFOOVF (FIFO data overflow) */
#define VECTOR_NUMBER_ADC_FIFOREQ0 ((IRQn_Type) 17) /* ADC FIFOREQ0 (FIFO data read request interrupt(Gr.0)) */
#define ADC_FIFOREQ0_IRQn          ((IRQn_Type) 17) /* ADC FIFOREQ0 (FIFO data read request interrupt(Gr.0)) */
#define VECTOR_NUMBER_ADC_FIFOREQ1 ((IRQn_Type) 18) /* ADC FIFOREQ1 (FIFO data read request interrupt(Gr.1)) */
#define ADC_FIFOREQ1_IRQn          ((IRQn_Type) 18) /* ADC FIFOREQ1 (FIFO data read request interrupt(Gr.1)) */
#define VECTOR_NUMBER_ADC_FIFOREQ2 ((IRQn_Type) 19) /* ADC FIFOREQ2 (FIFO data read request interrupt(Gr.2)) */
#define ADC_FIFOREQ2_IRQn          ((IRQn_Type) 19) /* ADC FIFOREQ2 (FIFO data read request interrupt(Gr.2)) */
#define VECTOR_NUMBER_ADC_FIFOREQ3 ((IRQn_Type) 20) /* ADC FIFOREQ3 (FIFO data read request interrupt(Gr.3)) */
#define ADC_FIFOREQ3_IRQn          ((IRQn_Type) 20) /* ADC FIFOREQ3 (FIFO data read request interrupt(Gr.3)) */
#define VECTOR_NUMBER_ADC_FIFOREQ4 ((IRQn_Type) 21) /* ADC FIFOREQ4 (FIFO data read request interrupt(Gr.4)) */
#define ADC_FIFOREQ4_IRQn          ((IRQn_Type) 21) /* ADC FIFOREQ4 (FIFO data read request interrupt(Gr.4)) */
/* The number of entries required for the ICU vector table. */
#define BSP_ICU_VECTOR_NUM_ENTRIES (22)

#ifdef __cplusplus
        }
        #endif
#endif /* VECTOR_DATA_H */
