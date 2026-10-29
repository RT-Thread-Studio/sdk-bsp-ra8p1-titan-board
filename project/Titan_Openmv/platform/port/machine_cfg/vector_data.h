/* Project-maintained machine vector allocation, paired with vector_data_openmv.c. */
        #ifndef VECTOR_DATA_H
        #define VECTOR_DATA_H
        #ifdef __cplusplus
        extern "C" {
        #endif
                /* Number of interrupts allocated */
        #ifndef VECTOR_DATA_IRQ_COUNT
        #define VECTOR_DATA_IRQ_COUNT    (95)
        #endif
        /* ISR prototypes */
        void layer3_switch_gwdi_isr(void);
        void layer3_switch_eaei_isr(void);
        void sci_b_uart_rxi_isr(void);
        void sci_b_uart_txi_isr(void);
        void sci_b_uart_tei_isr(void);
        void sci_b_uart_eri_isr(void);
        void iic_master_rxi_isr(void);
        void iic_master_txi_isr(void);
        void iic_master_tei_isr(void);
        void iic_master_eri_isr(void);
        void drw_int_isr(void);
        void sdhimmc_accs_isr(void);
        void sdhimmc_card_isr(void);
        void dmac_int_isr(void);
        void ssi_txi_isr(void);
        void ssi_int_isr(void);
        void usbfs_interrupt_handler(void);
        void usbfs_resume_handler(void);
        void usbfs_d0fifo_handler(void);
        void usbfs_d1fifo_handler(void);
        void usbhs_interrupt_handler(void);
        void usbhs_d0fifo_handler(void);
        void usbhs_d1fifo_handler(void);
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
        void canfd_error_isr(void);
        void canfd_channel_tx_isr(void);
        void canfd_common_fifo_rx_isr(void);
        void canfd_rx_fifo_isr(void);
        void gpt_counter_overflow_isr(void);
        void rtc_alarm_periodic_isr(void);
        void rtc_carry_isr(void);
        void r_icu_isr(void);
        void spi_b_rxi_isr(void);
        void spi_b_txi_isr(void);
        void spi_b_tei_isr(void);
        void spi_b_eri_isr(void);
        void pdm_sdet_isr(void);
        void pdm_dat_isr(void);
        void pdm_err_isr(void);
        void glcdc_line_detect_isr(void);
        void vin_status_isr(void);
        void vin_error_isr(void);
        void mipi_csi_rx_isr(void);
        void mipi_csi_dl_isr(void);
        void mipi_csi_vc_isr(void);
        void mipi_csi_pm_isr(void);
        void mipi_csi_gst_isr(void);
        void rm_ethosu_isr(void);
        void ipc_isr(void);

        /* Vector table allocations */
        #define VECTOR_NUMBER_SCI2_RXI ((IRQn_Type) 3) /* SCI2 RXI (Receive data full) */
        #define SCI2_RXI_IRQn          ((IRQn_Type) 3) /* SCI2 RXI (Receive data full) */
        #define VECTOR_NUMBER_SCI2_TXI ((IRQn_Type) 4) /* SCI2 TXI (Transmit data empty) */
        #define SCI2_TXI_IRQn          ((IRQn_Type) 4) /* SCI2 TXI (Transmit data empty) */
        #define VECTOR_NUMBER_SCI2_TEI ((IRQn_Type) 5) /* SCI2 TEI (Transmit end) */
        #define SCI2_TEI_IRQn          ((IRQn_Type) 5) /* SCI2 TEI (Transmit end) */
        #define VECTOR_NUMBER_SCI2_ERI ((IRQn_Type) 6) /* SCI2 ERI (Receive error) */
        #define SCI2_ERI_IRQn          ((IRQn_Type) 6) /* SCI2 ERI (Receive error) */
        #define VECTOR_NUMBER_IIC0_RXI ((IRQn_Type) 7) /* IIC0 RXI (Receive data full) */
        #define IIC0_RXI_IRQn          ((IRQn_Type) 7) /* IIC0 RXI (Receive data full) */
        #define VECTOR_NUMBER_IIC0_TXI ((IRQn_Type) 8) /* IIC0 TXI (Transmit data empty) */
        #define IIC0_TXI_IRQn          ((IRQn_Type) 8) /* IIC0 TXI (Transmit data empty) */
        #define VECTOR_NUMBER_IIC0_TEI ((IRQn_Type) 9) /* IIC0 TEI (Transmit end) */
        #define IIC0_TEI_IRQn          ((IRQn_Type) 9) /* IIC0 TEI (Transmit end) */
        #define VECTOR_NUMBER_IIC0_ERI ((IRQn_Type) 10) /* IIC0 ERI (Transfer error) */
        #define IIC0_ERI_IRQn          ((IRQn_Type) 10) /* IIC0 ERI (Transfer error) */
        #define VECTOR_NUMBER_DRW_INT ((IRQn_Type) 11) /* DRW INT (DRW interrupt) */
        #define DRW_INT_IRQn          ((IRQn_Type) 11) /* DRW INT (DRW interrupt) */
        #define VECTOR_NUMBER_SDHIMMC0_ACCS ((IRQn_Type) 12) /* SDHIMMC0 ACCS (Card access) */
        #define SDHIMMC0_ACCS_IRQn          ((IRQn_Type) 12) /* SDHIMMC0 ACCS (Card access) */
        #define VECTOR_NUMBER_SDHIMMC0_CARD ((IRQn_Type) 13) /* SDHIMMC0 CARD (Card detect) */
        #define SDHIMMC0_CARD_IRQn          ((IRQn_Type) 13) /* SDHIMMC0 CARD (Card detect) */
        #define VECTOR_NUMBER_DMAC0_INT ((IRQn_Type) 14) /* DMAC0 INT (DMAC0 transfer end) */
        #define DMAC0_INT_IRQn          ((IRQn_Type) 14) /* DMAC0 INT (DMAC0 transfer end) */
        #define VECTOR_NUMBER_SSI0_TXI ((IRQn_Type) 15) /* SSI0 TXI (Transmit data empty) */
        #define SSI0_TXI_IRQn          ((IRQn_Type) 15) /* SSI0 TXI (Transmit data empty) */
        #define VECTOR_NUMBER_SSI0_INT ((IRQn_Type) 16) /* SSI0 INT (Error interrupt) */
        #define SSI0_INT_IRQn          ((IRQn_Type) 16) /* SSI0 INT (Error interrupt) */
        #define VECTOR_NUMBER_USBFS_INT ((IRQn_Type) 17) /* USBFS INT (USBFS interrupt) */
        #define USBFS_INT_IRQn          ((IRQn_Type) 17) /* USBFS INT (USBFS interrupt) */
        #define VECTOR_NUMBER_USBFS_RESUME ((IRQn_Type) 18) /* USBFS RESUME (USBFS resume interrupt) */
        #define USBFS_RESUME_IRQn          ((IRQn_Type) 18) /* USBFS RESUME (USBFS resume interrupt) */
        #define VECTOR_NUMBER_USBFS_FIFO_0 ((IRQn_Type) 19) /* USBFS FIFO 0 (DMA/DTC transfer request 0) */
        #define USBFS_FIFO_0_IRQn          ((IRQn_Type) 19) /* USBFS FIFO 0 (DMA/DTC transfer request 0) */
        #define VECTOR_NUMBER_USBFS_FIFO_1 ((IRQn_Type) 20) /* USBFS FIFO 1 (DMA/DTC transfer request 1) */
        #define USBFS_FIFO_1_IRQn          ((IRQn_Type) 20) /* USBFS FIFO 1 (DMA/DTC transfer request 1) */
        #define VECTOR_NUMBER_USBHS_USB_INT_RESUME ((IRQn_Type) 21) /* USBHS USB INT RESUME (USBHS interrupt) */
        #define USBHS_USB_INT_RESUME_IRQn          ((IRQn_Type) 21) /* USBHS USB INT RESUME (USBHS interrupt) */
        #define VECTOR_NUMBER_USBHS_FIFO_0 ((IRQn_Type) 22) /* USBHS FIFO 0 (DMA transfer request 0) */
        #define USBHS_FIFO_0_IRQn          ((IRQn_Type) 22) /* USBHS FIFO 0 (DMA transfer request 0) */
        #define VECTOR_NUMBER_USBHS_FIFO_1 ((IRQn_Type) 23) /* USBHS FIFO 1 (DMA transfer request 1) */
        #define USBHS_FIFO_1_IRQn          ((IRQn_Type) 23) /* USBHS FIFO 1 (DMA transfer request 1) */
        #define VECTOR_NUMBER_ADC_LIMCLPI ((IRQn_Type) 24) /* ADC LIMCLPI (Limiter clip interrupt with the limit table 0 to 7) */
        #define ADC_LIMCLPI_IRQn          ((IRQn_Type) 24) /* ADC LIMCLPI (Limiter clip interrupt with the limit table 0 to 7) */
        #define VECTOR_NUMBER_ADC_ERR0 ((IRQn_Type) 25) /* ADC ERR0 (A/D converter unit 0 Error) */
        #define ADC_ERR0_IRQn          ((IRQn_Type) 25) /* ADC ERR0 (A/D converter unit 0 Error) */
        #define VECTOR_NUMBER_ADC_ERR1 ((IRQn_Type) 26) /* ADC ERR1 (A/D converter unit 1 Error) */
        #define ADC_ERR1_IRQn          ((IRQn_Type) 26) /* ADC ERR1 (A/D converter unit 1 Error) */
        #define VECTOR_NUMBER_ADC_RESOVF0 ((IRQn_Type) 27) /* ADC RESOVF0 (A/D conversion overflow on A/D converter unit 0) */
        #define ADC_RESOVF0_IRQn          ((IRQn_Type) 27) /* ADC RESOVF0 (A/D conversion overflow on A/D converter unit 0) */
        #define VECTOR_NUMBER_ADC_RESOVF1 ((IRQn_Type) 28) /* ADC RESOVF1 (A/D conversion overflow on A/D converter unit 1) */
        #define ADC_RESOVF1_IRQn          ((IRQn_Type) 28) /* ADC RESOVF1 (A/D conversion overflow on A/D converter unit 1) */
        #define VECTOR_NUMBER_ADC_CALEND0 ((IRQn_Type) 29) /* ADC CALEND0 (End of calibration of A/D converter unit 0) */
        #define ADC_CALEND0_IRQn          ((IRQn_Type) 29) /* ADC CALEND0 (End of calibration of A/D converter unit 0) */
        #define VECTOR_NUMBER_ADC_CALEND1 ((IRQn_Type) 30) /* ADC CALEND1 (End of calibration of A/D converter unit 1) */
        #define ADC_CALEND1_IRQn          ((IRQn_Type) 30) /* ADC CALEND1 (End of calibration of A/D converter unit 1) */
        #define VECTOR_NUMBER_ADC_ADI0 ((IRQn_Type) 31) /* ADC ADI0 (End of A/D scanning operation(Gr.0)) */
        #define ADC_ADI0_IRQn          ((IRQn_Type) 31) /* ADC ADI0 (End of A/D scanning operation(Gr.0)) */
        #define VECTOR_NUMBER_ADC_ADI1 ((IRQn_Type) 32) /* ADC ADI1 (End of A/D scanning operation(Gr.1)) */
        #define ADC_ADI1_IRQn          ((IRQn_Type) 32) /* ADC ADI1 (End of A/D scanning operation(Gr.1)) */
        #define VECTOR_NUMBER_ADC_ADI2 ((IRQn_Type) 33) /* ADC ADI2 (End of A/D scanning operation(Gr.2)) */
        #define ADC_ADI2_IRQn          ((IRQn_Type) 33) /* ADC ADI2 (End of A/D scanning operation(Gr.2)) */
        #define VECTOR_NUMBER_ADC_ADI3 ((IRQn_Type) 34) /* ADC ADI3 (End of A/D scanning operation(Gr.3)) */
        #define ADC_ADI3_IRQn          ((IRQn_Type) 34) /* ADC ADI3 (End of A/D scanning operation(Gr.3)) */
        #define VECTOR_NUMBER_ADC_ADI4 ((IRQn_Type) 35) /* ADC ADI4 (End of A/D scanning operation(Gr.4)) */
        #define ADC_ADI4_IRQn          ((IRQn_Type) 35) /* ADC ADI4 (End of A/D scanning operation(Gr.4)) */
        #define VECTOR_NUMBER_ADC_FIFOOVF ((IRQn_Type) 36) /* ADC FIFOOVF (FIFO data overflow) */
        #define ADC_FIFOOVF_IRQn          ((IRQn_Type) 36) /* ADC FIFOOVF (FIFO data overflow) */
        #define VECTOR_NUMBER_ADC_FIFOREQ0 ((IRQn_Type) 37) /* ADC FIFOREQ0 (FIFO data read request interrupt(Gr.0)) */
        #define ADC_FIFOREQ0_IRQn          ((IRQn_Type) 37) /* ADC FIFOREQ0 (FIFO data read request interrupt(Gr.0)) */
        #define VECTOR_NUMBER_ADC_FIFOREQ1 ((IRQn_Type) 38) /* ADC FIFOREQ1 (FIFO data read request interrupt(Gr.1)) */
        #define ADC_FIFOREQ1_IRQn          ((IRQn_Type) 38) /* ADC FIFOREQ1 (FIFO data read request interrupt(Gr.1)) */
        #define VECTOR_NUMBER_ADC_FIFOREQ2 ((IRQn_Type) 39) /* ADC FIFOREQ2 (FIFO data read request interrupt(Gr.2)) */
        #define ADC_FIFOREQ2_IRQn          ((IRQn_Type) 39) /* ADC FIFOREQ2 (FIFO data read request interrupt(Gr.2)) */
        #define VECTOR_NUMBER_ADC_FIFOREQ3 ((IRQn_Type) 40) /* ADC FIFOREQ3 (FIFO data read request interrupt(Gr.3)) */
        #define ADC_FIFOREQ3_IRQn          ((IRQn_Type) 40) /* ADC FIFOREQ3 (FIFO data read request interrupt(Gr.3)) */
        #define VECTOR_NUMBER_ADC_FIFOREQ4 ((IRQn_Type) 41) /* ADC FIFOREQ4 (FIFO data read request interrupt(Gr.4)) */
        #define ADC_FIFOREQ4_IRQn          ((IRQn_Type) 41) /* ADC FIFOREQ4 (FIFO data read request interrupt(Gr.4)) */
        #define VECTOR_NUMBER_CAN0_CHERR ((IRQn_Type) 42) /* CAN0 CHERR (Channel  error) */
        #define CAN0_CHERR_IRQn          ((IRQn_Type) 42) /* CAN0 CHERR (Channel  error) */
        #define VECTOR_NUMBER_CAN0_TX ((IRQn_Type) 43) /* CAN0 TX (Transmit interrupt) */
        #define CAN0_TX_IRQn          ((IRQn_Type) 43) /* CAN0 TX (Transmit interrupt) */
        #define VECTOR_NUMBER_CAN0_COMFRX ((IRQn_Type) 44) /* CAN0 COMFRX (Common FIFO receive interrupt) */
        #define CAN0_COMFRX_IRQn          ((IRQn_Type) 44) /* CAN0 COMFRX (Common FIFO receive interrupt) */
        #define VECTOR_NUMBER_CAN_GLERR ((IRQn_Type) 45) /* CAN GLERR (Global error) */
        #define CAN_GLERR_IRQn          ((IRQn_Type) 45) /* CAN GLERR (Global error) */
        #define VECTOR_NUMBER_CAN_RXF ((IRQn_Type) 46) /* CAN RXF (Global receive FIFO interrupt) */
        #define CAN_RXF_IRQn          ((IRQn_Type) 46) /* CAN RXF (Global receive FIFO interrupt) */
        #define VECTOR_NUMBER_GPT1_COUNTER_OVERFLOW ((IRQn_Type) 47) /* GPT1 COUNTER OVERFLOW (Overflow) */
        #define GPT1_COUNTER_OVERFLOW_IRQn          ((IRQn_Type) 47) /* GPT1 COUNTER OVERFLOW (Overflow) */
        #define VECTOR_NUMBER_RTC_ALARM ((IRQn_Type) 48) /* RTC ALARM (Alarm interrupt) */
        #define RTC_ALARM_IRQn          ((IRQn_Type) 48) /* RTC ALARM (Alarm interrupt) */
        #define VECTOR_NUMBER_RTC_PERIOD ((IRQn_Type) 49) /* RTC PERIOD (Periodic interrupt) */
        #define RTC_PERIOD_IRQn          ((IRQn_Type) 49) /* RTC PERIOD (Periodic interrupt) */
        #define VECTOR_NUMBER_RTC_CARRY ((IRQn_Type) 50) /* RTC CARRY (Carry interrupt) */
        #define RTC_CARRY_IRQn          ((IRQn_Type) 50) /* RTC CARRY (Carry interrupt) */
        #define VECTOR_NUMBER_ICU_IRQ4 ((IRQn_Type) 51) /* ICU IRQ4 (External pin interrupt 4) */
        #define ICU_IRQ4_IRQn          ((IRQn_Type) 51) /* ICU IRQ4 (External pin interrupt 4) */
        #define VECTOR_NUMBER_SPI1_RXI ((IRQn_Type) 52) /* SPI1 RXI (Receive buffer full) */
        #define SPI1_RXI_IRQn          ((IRQn_Type) 52) /* SPI1 RXI (Receive buffer full) */
        #define VECTOR_NUMBER_SPI1_TXI ((IRQn_Type) 53) /* SPI1 TXI (Transmit buffer empty) */
        #define SPI1_TXI_IRQn          ((IRQn_Type) 53) /* SPI1 TXI (Transmit buffer empty) */
        #define VECTOR_NUMBER_SPI1_TEI ((IRQn_Type) 54) /* SPI1 TEI (Transmission complete event) */
        #define SPI1_TEI_IRQn          ((IRQn_Type) 54) /* SPI1 TEI (Transmission complete event) */
        #define VECTOR_NUMBER_SPI1_ERI ((IRQn_Type) 55) /* SPI1 ERI (Error) */
        #define SPI1_ERI_IRQn          ((IRQn_Type) 55) /* SPI1 ERI (Error) */
        #define VECTOR_NUMBER_SCI8_RXI ((IRQn_Type) 56) /* SCI8 RXI (Receive data full) */
        #define SCI8_RXI_IRQn          ((IRQn_Type) 56) /* SCI8 RXI (Receive data full) */
        #define VECTOR_NUMBER_SCI8_TXI ((IRQn_Type) 57) /* SCI8 TXI (Transmit data empty) */
        #define SCI8_TXI_IRQn          ((IRQn_Type) 57) /* SCI8 TXI (Transmit data empty) */
        #define VECTOR_NUMBER_SCI8_TEI ((IRQn_Type) 58) /* SCI8 TEI (Transmit end) */
        #define SCI8_TEI_IRQn          ((IRQn_Type) 58) /* SCI8 TEI (Transmit end) */
        #define VECTOR_NUMBER_SCI8_ERI ((IRQn_Type) 59) /* SCI8 ERI (Receive error) */
        #define SCI8_ERI_IRQn          ((IRQn_Type) 59) /* SCI8 ERI (Receive error) */
        #define VECTOR_NUMBER_PDM_SDET ((IRQn_Type) 60) /* PDM SDET (Sound detection interrupt) */
        #define PDM_SDET_IRQn          ((IRQn_Type) 60) /* PDM SDET (Sound detection interrupt) */
        #define VECTOR_NUMBER_PDM_DAT2 ((IRQn_Type) 61) /* PDM DAT2 (Data reception interrupt channel 2) */
        #define PDM_DAT2_IRQn          ((IRQn_Type) 61) /* PDM DAT2 (Data reception interrupt channel 2) */
        #define VECTOR_NUMBER_PDM_ERR2 ((IRQn_Type) 62) /* PDM ERR2 (Error detection interrupt channel 2) */
        #define PDM_ERR2_IRQn          ((IRQn_Type) 62) /* PDM ERR2 (Error detection interrupt channel 2) */
        #define VECTOR_NUMBER_GPT0_COUNTER_OVERFLOW ((IRQn_Type) 63) /* GPT0 COUNTER OVERFLOW (Overflow) */
        #define GPT0_COUNTER_OVERFLOW_IRQn          ((IRQn_Type) 63) /* GPT0 COUNTER OVERFLOW (Overflow) */
        #define VECTOR_NUMBER_GLCDC_LINE_DETECT ((IRQn_Type) 64) /* GLCDC LINE DETECT (Specified line) */
        #define GLCDC_LINE_DETECT_IRQn          ((IRQn_Type) 64) /* GLCDC LINE DETECT (Specified line) */
        #define VECTOR_NUMBER_VIN_IRQ ((IRQn_Type) 65) /* VIN IRQ (Interrupt Request) */
        #define VIN_IRQ_IRQn          ((IRQn_Type) 65) /* VIN IRQ (Interrupt Request) */
        #define VECTOR_NUMBER_VIN_ERR ((IRQn_Type) 66) /* VIN ERR (Interrupt Request for SYNC Error) */
        #define VIN_ERR_IRQn          ((IRQn_Type) 66) /* VIN ERR (Interrupt Request for SYNC Error) */
        #define VECTOR_NUMBER_MIPICSI_RX ((IRQn_Type) 67) /* MIPICSI RX (Receive interrupt) */
        #define MIPICSI_RX_IRQn          ((IRQn_Type) 67) /* MIPICSI RX (Receive interrupt) */
        #define VECTOR_NUMBER_MIPICSI_DL ((IRQn_Type) 68) /* MIPICSI DL (Data Lane interrupt) */
        #define MIPICSI_DL_IRQn          ((IRQn_Type) 68) /* MIPICSI DL (Data Lane interrupt) */
        #define VECTOR_NUMBER_MIPICSI_VC ((IRQn_Type) 69) /* MIPICSI VC (Virtual Channel interrupt) */
        #define MIPICSI_VC_IRQn          ((IRQn_Type) 69) /* MIPICSI VC (Virtual Channel interrupt) */
        #define VECTOR_NUMBER_MIPICSI_PM ((IRQn_Type) 70) /* MIPICSI PM (Power Management interrupt) */
        #define MIPICSI_PM_IRQn          ((IRQn_Type) 70) /* MIPICSI PM (Power Management interrupt) */
        #define VECTOR_NUMBER_MIPICSI_GST ((IRQn_Type) 71) /* MIPICSI GST (Generic Short Packet interrupt) */
        #define MIPICSI_GST_IRQn          ((IRQn_Type) 71) /* MIPICSI GST (Generic Short Packet interrupt) */
        #define VECTOR_NUMBER_NPU_IRQ ((IRQn_Type) 72) /* NPU IRQ (NPU IRQ) */
        #define NPU_IRQ_IRQn          ((IRQn_Type) 72) /* NPU IRQ (NPU IRQ) */
        #define VECTOR_NUMBER_IPC_IRQ1 ((IRQn_Type) 73) /* IPC IRQ1 (CPU Mutual Interrupt 1) */
        #define IPC_IRQ1_IRQn          ((IRQn_Type) 73) /* IPC IRQ1 (CPU Mutual Interrupt 1) */
        /* The number of entries required for the ICU vector table. */
        #define BSP_ICU_VECTOR_NUM_ENTRIES (95)

        #ifdef __cplusplus
        }
        #endif
        #endif /* VECTOR_DATA_H */
