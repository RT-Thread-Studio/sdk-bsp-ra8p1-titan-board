/* generated vector header file - do not edit */
        #ifndef VECTOR_DATA_H
        #define VECTOR_DATA_H
        #ifdef __cplusplus
        extern "C" {
        #endif
                /* Number of interrupts allocated */
        #ifndef VECTOR_DATA_IRQ_COUNT
        #define VECTOR_DATA_IRQ_COUNT    (7)
        #endif
        /* ISR prototypes */
        void layer3_switch_gwdi_isr(void);
        void layer3_switch_eaei_isr(void);
        void sci_b_uart_rxi_isr(void);
        void sci_b_uart_txi_isr(void);
        void sci_b_uart_tei_isr(void);
        void sci_b_uart_eri_isr(void);

        /* Vector table allocations */
        #define VECTOR_NUMBER_ETHER_GWDI0 ((IRQn_Type) 0) /* ETHER GWDI0 (GWCA Data Interrupt 0) */
        #define ETHER_GWDI0_IRQn          ((IRQn_Type) 0) /* ETHER GWDI0 (GWCA Data Interrupt 0) */
        #define VECTOR_NUMBER_ETHER_EAEI0 ((IRQn_Type) 1) /* ETHER EAEI0 (ETHA0 Error Interrupt) */
        #define ETHER_EAEI0_IRQn          ((IRQn_Type) 1) /* ETHER EAEI0 (ETHA0 Error Interrupt) */
        #define VECTOR_NUMBER_ETHER_EAEI1 ((IRQn_Type) 2) /* ETHER EAEI1 (ETHA1 Error Interrupt) */
        #define ETHER_EAEI1_IRQn          ((IRQn_Type) 2) /* ETHER EAEI1 (ETHA1 Error Interrupt) */
        #define VECTOR_NUMBER_SCI8_RXI ((IRQn_Type) 3) /* SCI8 RXI (Receive data full) */
        #define SCI8_RXI_IRQn          ((IRQn_Type) 3) /* SCI8 RXI (Receive data full) */
        #define VECTOR_NUMBER_SCI8_TXI ((IRQn_Type) 4) /* SCI8 TXI (Transmit data empty) */
        #define SCI8_TXI_IRQn          ((IRQn_Type) 4) /* SCI8 TXI (Transmit data empty) */
        #define VECTOR_NUMBER_SCI8_TEI ((IRQn_Type) 5) /* SCI8 TEI (Transmit end) */
        #define SCI8_TEI_IRQn          ((IRQn_Type) 5) /* SCI8 TEI (Transmit end) */
        #define VECTOR_NUMBER_SCI8_ERI ((IRQn_Type) 6) /* SCI8 ERI (Receive error) */
        #define SCI8_ERI_IRQn          ((IRQn_Type) 6) /* SCI8 ERI (Receive error) */
        /* The number of entries required for the ICU vector table. */
        #define BSP_ICU_VECTOR_NUM_ENTRIES (7)

        #ifdef __cplusplus
        }
        #endif
        #endif /* VECTOR_DATA_H */