/* generated HAL header file - do not edit */
#ifndef HAL_DATA_H_
#define HAL_DATA_H_
#include <stdint.h>
#include "bsp_api.h"
#include "common_data.h"
#include "r_ipc.h"
#include "r_gpt.h"
#include "r_timer_api.h"
#include "r_pdm_api.h"
            #include "r_pdm.h"
#include "r_sci_b_uart.h"
            #include "r_uart_api.h"
#include "r_dtc.h"
#include "r_transfer_api.h"
#include "r_spi_b.h"
#include "r_dmac.h"
#include "r_transfer_api.h"
#include "r_ospi_b.h"
#include "r_spi_flash_api.h"
#include "r_wdt.h"
#include "r_wdt_api.h"
#include "r_rtc.h"
#include "r_rtc_api.h"
#include "r_canfd.h"
#include "r_can_api.h"
#include "r_adc_b.h"
                      #include "r_adc_api.h"
#include "r_usb_basic.h"
#include "r_usb_basic_api.h"
#include "r_usb_pcdc_api.h"
#include "r_i2s_api.h"
            #include "r_ssi.h"
#include "r_sdhi.h"
#include "r_sdmmc_api.h"
#include "r_iic_master.h"
#include "r_i2c_master_api.h"
#include "r_ether_api.h"
#include "r_rmac.h"
FSP_HEADER
/** IPC Instance. */
            extern const ipc_instance_t g_ipc0;

            /** Access the IPC instance using these structures when calling API functions directly
            (::p_api is not used). */
            extern ipc_instance_ctrl_t g_ipc0_ctrl;
            extern const ipc_cfg_t g_ipc0_cfg;

#ifndef NULL
            void NULL(ipc_callback_args_t * p_args);
#endif
/** IPC Instance. */
            extern const ipc_instance_t g_ipc1;

            /** Access the IPC instance using these structures when calling API functions directly
            (::p_api is not used). */
            extern ipc_instance_ctrl_t g_ipc1_ctrl;
            extern const ipc_cfg_t g_ipc1_cfg;

#ifndef g_ipc1_callback
            void g_ipc1_callback(ipc_callback_args_t * p_args);
#endif
/** Timer on GPT Instance. */
extern const timer_instance_t g_timer0;

/** Access the GPT instance using these structures when calling API functions directly (::p_api is not used). */
extern gpt_instance_ctrl_t g_timer0_ctrl;
extern const timer_cfg_t g_timer0_cfg;

#ifndef timer0_callback
void timer0_callback(timer_callback_args_t * p_args);
#endif
/** Timer on GPT Instance. */
extern const timer_instance_t g_timer12;

/** Access the GPT instance using these structures when calling API functions directly (::p_api is not used). */
extern gpt_instance_ctrl_t g_timer12_ctrl;
extern const timer_cfg_t g_timer12_cfg;

#ifndef NULL
void NULL(timer_callback_args_t * p_args);
#endif
/** Timer on GPT Instance. */
extern const timer_instance_t g_timer8;

/** Access the GPT instance using these structures when calling API functions directly (::p_api is not used). */
extern gpt_instance_ctrl_t g_timer8_ctrl;
extern const timer_cfg_t g_timer8_cfg;

#ifndef NULL
void NULL(timer_callback_args_t * p_args);
#endif
/** Timer on GPT Instance. */
extern const timer_instance_t g_timer6;

/** Access the GPT instance using these structures when calling API functions directly (::p_api is not used). */
extern gpt_instance_ctrl_t g_timer6_ctrl;
extern const timer_cfg_t g_timer6_cfg;

#ifndef NULL
void NULL(timer_callback_args_t * p_args);
#endif
#define PDM2_CALCULATED_SINCRNG_VALUE (5)
#define PDM2_CALCULATED_SINCDEC_VALUE (124)
#define PDM2_FILTER_SETTLING_TIME_US  (1662)

            /** PDM Instance. */
            extern const pdm_instance_t      g_pdm0;

            /** Access the PDM instance using these structures when calling API functions directly (::p_api is not used). */
            extern pdm_instance_ctrl_t g_pdm0_ctrl;
            extern const pdm_cfg_t g_pdm0_cfg;

            #ifndef pdm_callback
            void pdm_callback(pdm_callback_args_t * p_args);
            #endif
/** UART on SCI Instance. */
            extern const uart_instance_t      g_uart8;

            /** Access the UART instance using these structures when calling API functions directly (::p_api is not used). */
            extern sci_b_uart_instance_ctrl_t     g_uart8_ctrl;
            extern const uart_cfg_t g_uart8_cfg;
            extern const sci_b_uart_extended_cfg_t g_uart8_cfg_extend;

            #ifndef user_uart8_callback
            void user_uart8_callback(uart_callback_args_t * p_args);
            #endif
/* Transfer on DTC Instance. */
extern const transfer_instance_t g_transfer4;

/** Access the DTC instance using these structures when calling API functions directly (::p_api is not used). */
extern dtc_instance_ctrl_t g_transfer4_ctrl;
extern const transfer_cfg_t g_transfer4_cfg;
/* Transfer on DTC Instance. */
extern const transfer_instance_t g_transfer3;

/** Access the DTC instance using these structures when calling API functions directly (::p_api is not used). */
extern dtc_instance_ctrl_t g_transfer3_ctrl;
extern const transfer_cfg_t g_transfer3_cfg;
/** SPI on SPI Instance. */
extern const spi_instance_t g_spi1;

/** Access the SPI instance using these structures when calling API functions directly (::p_api is not used). */
extern spi_b_instance_ctrl_t g_spi1_ctrl;
extern const spi_cfg_t g_spi1_cfg;

/** Callback used by SPI Instance. */
#ifndef spi1_callback
void spi1_callback(spi_callback_args_t * p_args);
#endif


#define RA_NOT_DEFINED (1)
#if (RA_NOT_DEFINED == g_transfer3)
    #define g_spi1_P_TRANSFER_TX (NULL)
#else
    #define g_spi1_P_TRANSFER_TX (&g_transfer3)
#endif
#if (RA_NOT_DEFINED == g_transfer4)
    #define g_spi1_P_TRANSFER_RX (NULL)
#else
    #define g_spi1_P_TRANSFER_RX (&g_transfer4)
#endif
#undef RA_NOT_DEFINED
/* Transfer on DMAC Instance. */
extern const transfer_instance_t g_transfer0;

/** Access the DMAC instance using these structures when calling API functions directly (::p_api is not used). */
extern dmac_instance_ctrl_t g_transfer0_ctrl;
extern const transfer_cfg_t g_transfer0_cfg;

#ifndef NULL
void NULL(transfer_callback_args_t * p_args);
#endif
#if OSPI_B_CFG_DMAC_SUPPORT_ENABLE
    #include "r_dmac.h"
#endif
#if OSPI_CFG_DOTF_SUPPORT_ENABLE
    #include "r_sce_if.h"
#endif

extern const spi_flash_instance_t g_ospi_b;
extern ospi_b_instance_ctrl_t g_ospi_b_ctrl;
extern const spi_flash_cfg_t g_ospi_b_cfg;
/** WDT on WDT Instance. */
extern const wdt_instance_t g_wdt;

/** Access the WDT instance using these structures when calling API functions directly (::p_api is not used). */
extern wdt_instance_ctrl_t g_wdt_ctrl;
extern const wdt_cfg_t g_wdt_cfg;

#ifndef NULL
void NULL(wdt_callback_args_t * p_args);
#endif
/* RTC Instance. */
extern const rtc_instance_t g_rtc;

/** Access the RTC instance using these structures when calling API functions directly (::p_api is not used). */
extern rtc_instance_ctrl_t g_rtc_ctrl;
extern const rtc_cfg_t g_rtc_cfg;

#ifndef rtc_callback
void rtc_callback(rtc_callback_args_t * p_args);
#endif
/** Timer on GPT Instance. */
extern const timer_instance_t g_timer9;

/** Access the GPT instance using these structures when calling API functions directly (::p_api is not used). */
extern gpt_instance_ctrl_t g_timer9_ctrl;
extern const timer_cfg_t g_timer9_cfg;

#ifndef NULL
void NULL(timer_callback_args_t * p_args);
#endif
/** Timer on GPT Instance. */
extern const timer_instance_t g_timer10;

/** Access the GPT instance using these structures when calling API functions directly (::p_api is not used). */
extern gpt_instance_ctrl_t g_timer10_ctrl;
extern const timer_cfg_t g_timer10_cfg;

#ifndef NULL
void NULL(timer_callback_args_t * p_args);
#endif
/** Timer on GPT Instance. */
extern const timer_instance_t g_timer1;

/** Access the GPT instance using these structures when calling API functions directly (::p_api is not used). */
extern gpt_instance_ctrl_t g_timer1_ctrl;
extern const timer_cfg_t g_timer1_cfg;

#ifndef timer1_callback
void timer1_callback(timer_callback_args_t * p_args);
#endif
/** CANFD on CANFD Instance. */
extern const can_instance_t g_canfd0;
/** Access the CANFD instance using these structures when calling API functions directly (::p_api is not used). */
extern canfd_instance_ctrl_t g_canfd0_ctrl;
extern const can_cfg_t g_canfd0_cfg;
extern const canfd_extended_cfg_t g_canfd0_cfg_extend;

#ifndef canfd0_callback
void canfd0_callback(can_callback_args_t * p_args);
#endif

/* Global configuration (referenced by all instances) */
extern canfd_global_cfg_t g_canfd_global_cfg;
/** ADC on ADC_B instance. */
                    extern const adc_instance_t g_adc0;

                    /** Access the ADC_B instance using these structures when calling API functions directly (::p_api is not used). */
                    extern adc_b_instance_ctrl_t g_adc0_ctrl;
                    extern const adc_cfg_t g_adc0_cfg;
                    extern const adc_b_scan_cfg_t g_adc0_scan_cfg;

                    #ifndef NULL
                    void NULL(adc_callback_args_t * p_args);
                    #endif
/** Timer on GPT Instance. */
extern const timer_instance_t g_timer7;

/** Access the GPT instance using these structures when calling API functions directly (::p_api is not used). */
extern gpt_instance_ctrl_t g_timer7_ctrl;
extern const timer_cfg_t g_timer7_cfg;

#ifndef NULL
void NULL(timer_callback_args_t * p_args);
#endif
/* Basic on USB Instance. */
extern const usb_instance_t g_basic0;

/** Access the USB instance using these structures when calling API functions directly (::p_api is not used). */
extern usb_instance_ctrl_t g_basic0_ctrl;
extern const usb_cfg_t g_basic0_cfg;

#ifndef NULL
void NULL(void *);
#endif

#if 0 == BSP_CFG_RTOS
#ifndef NULL
void NULL(usb_callback_args_t *);
#endif
#endif

#if 2 == BSP_CFG_RTOS
#ifndef NULL
void NULL(usb_event_info_t *, usb_hdl_t, usb_onoff_t);
#endif
#endif
/** CDC Driver on USB Instance. */
/** Timer on GPT Instance. */
extern const timer_instance_t g_timer2;

/** Access the GPT instance using these structures when calling API functions directly (::p_api is not used). */
extern gpt_instance_ctrl_t g_timer2_ctrl;
extern const timer_cfg_t g_timer2_cfg;

#ifndef NULL
void NULL(timer_callback_args_t * p_args);
#endif
/* Transfer on DTC Instance. */
extern const transfer_instance_t g_transfer2;

/** Access the DTC instance using these structures when calling API functions directly (::p_api is not used). */
extern dtc_instance_ctrl_t g_transfer2_ctrl;
extern const transfer_cfg_t g_transfer2_cfg;
/** SSI Instance. */
            extern const i2s_instance_t      g_i2s0;

            /** Access the SSI instance using these structures when calling API functions directly (::p_api is not used). */
            extern ssi_instance_ctrl_t g_i2s0_ctrl;
            extern const i2s_cfg_t g_i2s0_cfg;

            #ifndef i2s0_callback
            void i2s0_callback(i2s_callback_args_t * p_args);
            #endif
/* Transfer on DMAC Instance. */
extern const transfer_instance_t g_transfer1;

/** Access the DMAC instance using these structures when calling API functions directly (::p_api is not used). */
extern dmac_instance_ctrl_t g_transfer1_ctrl;
extern const transfer_cfg_t g_transfer1_cfg;

#ifndef g_sdmmc0_dmac_callback
void g_sdmmc0_dmac_callback(transfer_callback_args_t * p_args);
#endif
/** SDMMC on SDMMC Instance. */
extern const sdmmc_instance_t g_sdmmc0;


/** Access the SDMMC instance using these structures when calling API functions directly (::p_api is not used). */
extern sdhi_instance_ctrl_t g_sdmmc0_ctrl;
extern sdmmc_cfg_t g_sdmmc0_cfg;

#ifndef sdhi0_callback
void sdhi0_callback(sdmmc_callback_args_t * p_args);
#endif
/** Timer on GPT Instance. */
extern const timer_instance_t g_timer11;

/** Access the GPT instance using these structures when calling API functions directly (::p_api is not used). */
extern gpt_instance_ctrl_t g_timer11_ctrl;
extern const timer_cfg_t g_timer11_cfg;

#ifndef NULL
void NULL(timer_callback_args_t * p_args);
#endif
/* I2C Master on IIC Instance. */
extern const i2c_master_instance_t g_i2c_master0;

/** Access the I2C Master instance using these structures when calling API functions directly (::p_api is not used). */
extern iic_master_instance_ctrl_t g_i2c_master0_ctrl;
extern const i2c_master_cfg_t g_i2c_master0_cfg;

#ifndef i2c_master_callback
void i2c_master_callback(i2c_master_callback_args_t * p_args);
#endif
/** UART on SCI Instance. */
            extern const uart_instance_t      g_uart2;

            /** Access the UART instance using these structures when calling API functions directly (::p_api is not used). */
            extern sci_b_uart_instance_ctrl_t     g_uart2_ctrl;
            extern const uart_cfg_t g_uart2_cfg;
            extern const sci_b_uart_extended_cfg_t g_uart2_cfg_extend;

            #ifndef user_uart2_callback
            void user_uart2_callback(uart_callback_args_t * p_args);
            #endif
/** Ether on RMAC Instance. */
extern const ether_instance_t g_ether0;

/** Access the RMAC instance using these structures when calling API functions directly (::p_api is not used). */
extern rmac_instance_ctrl_t g_ether0_ctrl;
extern const ether_cfg_t g_ether0_cfg;

#ifndef user_ether0_callback
void user_ether0_callback(ether_callback_args_t * p_args);
#endif
void hal_entry(void);
void g_hal_init(void);
FSP_FOOTER
#endif /* HAL_DATA_H_ */
