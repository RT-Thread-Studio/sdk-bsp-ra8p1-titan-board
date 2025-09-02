/* generated configuration header file - do not edit */
#ifndef BSP_PIN_CFG_H_
#define BSP_PIN_CFG_H_
#include "r_ioport.h"

/* Common macro for FSP header files. There is also a corresponding FSP_FOOTER macro at the end of this file. */
FSP_HEADER

#define S4_INT (BSP_IO_PORT_00_PIN_00)
#define ARDUINO_A0 (BSP_IO_PORT_00_PIN_01) /* Enable when connected */
#define GROVE2_2_AN (BSP_IO_PORT_00_PIN_02) /* Reference E42 / E40 */
#define ARDUINO_A2 (BSP_IO_PORT_00_PIN_03) /* Enable when connected */
#define ARDUINO_A3_MIKROBUS_AN (BSP_IO_PORT_00_PIN_04) /* Enable when connected */
#define GROVE2_1_AN (BSP_IO_PORT_00_PIN_05) /* Reference E41 / E39 */
#define PMOD1_7_INT (BSP_IO_PORT_00_PIN_06)
#define ARDUINO_A1 (BSP_IO_PORT_00_PIN_07) /* Enable when connected */
#define USER_S2 (BSP_IO_PORT_00_PIN_08)
#define USER_S1 (BSP_IO_PORT_00_PIN_09)
#define CAM_INT (BSP_IO_PORT_00_PIN_10) /* Enable when connected */
#define ARDUINO_D2 (BSP_IO_PORT_00_PIN_11) /* Enable when connected */
#define PMOD2_7_IRQ15 (BSP_IO_PORT_00_PIN_12)
#define I2C_SCL0_PULLUP (BSP_IO_PORT_00_PIN_13) /* Enable I3C_SEL_L to use */
#define ARDUINO_A4 (BSP_IO_PORT_00_PIN_14) /* Enable when connected */
#define ARDUINO_A5 (BSP_IO_PORT_00_PIN_15) /* Enable when connected */
#define OSPI_DQ0_ARDUINO_D12_MISOB_MIKROBUS_MISOB (BSP_IO_PORT_01_PIN_00) /* See SW4 in manual */
#define OSPI_DQ3_ARDUINO_D11_MOSIB_MIKROBUS_MOSIB (BSP_IO_PORT_01_PIN_01) /* See SW4 in manual */
#define OSPI_DQ4_ARDUINO_D13_RSPCKB_MIKROBUS_RSPCKB (BSP_IO_PORT_01_PIN_02) /* See SW4 in manual */
#define OSPI_DQ2_ARDUINO_D10_SSLB_MIKROBUS_SSLB (BSP_IO_PORT_01_PIN_03) /* See SW4 in manual */
#define OSPI_CS_ARDUINO_D5 (BSP_IO_PORT_01_PIN_04) /* See SW4 in manual */
#define OSPI_INT_ARDUINO_D6 (BSP_IO_PORT_01_PIN_05) /* See SW4 in manual */
#define OSPI_RES (BSP_IO_PORT_01_PIN_06) /* See SW4 in manual */
#define ETH_MDINT (BSP_IO_PORT_01_PIN_07)
#define J2_30 (BSP_IO_PORT_01_PIN_08) /* Enable when connected */
#define I2C_SDA0_PULLUP (BSP_IO_PORT_01_PIN_09) /* Enable I3C_SEL_L to use */
#define ARDUINO_D9 (BSP_IO_PORT_01_PIN_10) /* Enable when connected */
#define LCD_INT_MIPI_IRQ19 (BSP_IO_PORT_01_PIN_11) /* See SW4 in manual */
#define SDRAM_DQ3 (BSP_IO_PORT_01_PIN_12)
#define SDRAM_DQ4 (BSP_IO_PORT_01_PIN_13)
#define SDRAM_DQ5 (BSP_IO_PORT_01_PIN_14)
#define SDRAM_DQ6 (BSP_IO_PORT_01_PIN_15)
#define NMI (BSP_IO_PORT_02_PIN_00)
#define MD_MIKROBUS_RES (BSP_IO_PORT_02_PIN_01) /* See Manual */
#define ENET_RX_CTL (BSP_IO_PORT_02_PIN_06) /* Reference E38 in manual */
#define LCD_D9 (BSP_IO_PORT_02_PIN_07)
#define DEBUG_TDI (BSP_IO_PORT_02_PIN_08)
#define DEBUG_TDO_SWO (BSP_IO_PORT_02_PIN_09)
#define DEBUG_TMS_SWDIO (BSP_IO_PORT_02_PIN_10)
#define DEBUG_TCLK_SWCLK (BSP_IO_PORT_02_PIN_11)
#define EXTAL (BSP_IO_PORT_02_PIN_12) /* Main clock */
#define XTAL (BSP_IO_PORT_02_PIN_13) /* Main clock */
#define XCOUT (BSP_IO_PORT_02_PIN_14) /* Subclock */
#define XCIN (BSP_IO_PORT_02_PIN_15) /* Subclock */
#define SDRAM_DQ2 (BSP_IO_PORT_03_PIN_00)
#define SDRAM_DQ1 (BSP_IO_PORT_03_PIN_01)
#define SDRAM_DQ0 (BSP_IO_PORT_03_PIN_02)
#define USER_LED2_GREEN (BSP_IO_PORT_03_PIN_03)
#define ETH_TXD3_TRACE_TDATA3 (BSP_IO_PORT_03_PIN_04) /* Reference SW4 /  E44 / E18 in manual */
#define ETH_TXD2_TRACE_TDATA2 (BSP_IO_PORT_03_PIN_05) /* Reference SW4 /  E45 / E19 in manual */
#define ETH_TXD1_TRACE_TDATA1 (BSP_IO_PORT_03_PIN_06) /* Reference SW4 /  E46 / E20 in manual */
#define ETH_TXD0_TRACE_TDATA0 (BSP_IO_PORT_03_PIN_07) /* Reference SW4 /  E47 / E21 in manual */
#define TRACE_TCLK (BSP_IO_PORT_03_PIN_08) /* Reference SW4 /  E48 in manual */
#define ETH_TX_CLK (BSP_IO_PORT_03_PIN_09) /* Reference SW4 /  E23 in manual */
#define ETH_TX_CTL (BSP_IO_PORT_03_PIN_10) /* Reference SW4 /  E22 in manual */
#define CAM_D8 (BSP_IO_PORT_03_PIN_11)
#define ARDUINO_D7 (BSP_IO_PORT_03_PIN_12) /* Enable when connected */
#define CAM_D0_ARDUINO_GROVE1_SCL0 (BSP_IO_PORT_04_PIN_00)
#define ARDUINO_GROVE1_SDA0 (BSP_IO_PORT_04_PIN_01)
#define PMOD1_8_GPIO_RST (BSP_IO_PORT_04_PIN_02) /* Enable when connected */
#define AUDIO_BCLK (BSP_IO_PORT_04_PIN_03)
#define AUDIO_WCLK (BSP_IO_PORT_04_PIN_04)
#define CAM_D2_AUDIO_DATIN (BSP_IO_PORT_04_PIN_05) /* See J41 in manual */
#define CAM_D3_AUDIO_DOUT (BSP_IO_PORT_04_PIN_06) /* See J41 in manual */
#define USBFS_VBUS (BSP_IO_PORT_04_PIN_07)
#define USBHS_VBUS (BSP_IO_PORT_04_PIN_08)
#define PMOD2_9_GPIO (BSP_IO_PORT_04_PIN_09) /* Enable when connected */
#define PMOD2_8_GPIO_RST (BSP_IO_PORT_04_PIN_10) /* Enable when connected */
#define DSI_TE (BSP_IO_PORT_04_PIN_11)
#define PMOD1_9_GPIO (BSP_IO_PORT_04_PIN_12) /* Enable when connected */
#define PMOD1_10_GPIO (BSP_IO_PORT_04_PIN_13) /* Enable when connected */
#define ETH_MDIO (BSP_IO_PORT_04_PIN_14)
#define ETH_MDC (BSP_IO_PORT_04_PIN_15)
#define USBFS_VBUSEN (BSP_IO_PORT_05_PIN_00)
#define CAM_XCLK (BSP_IO_PORT_05_PIN_01) /* Required for communication / operation */
#define MEMS_MIC_DATA (BSP_IO_PORT_05_PIN_02)
#define SDRAM_A4 (BSP_IO_PORT_05_PIN_03)
#define SDRAM_A5 (BSP_IO_PORT_05_PIN_04)
#define SDRAM_A6 (BSP_IO_PORT_05_PIN_05)
#define SDRAM_A7 (BSP_IO_PORT_05_PIN_06)
#define SDRAM_A8 (BSP_IO_PORT_05_PIN_07)
#define SDRAM_A9 (BSP_IO_PORT_05_PIN_08)
#define SDRAM_A10 (BSP_IO_PORT_05_PIN_09)
#define SDRAM_A11 (BSP_IO_PORT_05_PIN_10)
#define IIC_SYSTEM_SDA1 (BSP_IO_PORT_05_PIN_11)
#define IIC_SYSTEM_SCL1 (BSP_IO_PORT_05_PIN_12)
#define LCD_TCON3 (BSP_IO_PORT_05_PIN_13)
#define ETH_RST (BSP_IO_PORT_05_PIN_14)
#define LCD_CLK (BSP_IO_PORT_05_PIN_15) /* Enable when connected */
#define USER_LED1_BLUE (BSP_IO_PORT_06_PIN_00)
#define PMOD2_4_RSPCKB (BSP_IO_PORT_06_PIN_01) /* See E16 / E14 in manual */
#define PMOD2_3_MISOB_RXD0 (BSP_IO_PORT_06_PIN_02)
#define PMOD2_2_MOSIB_TXD0 (BSP_IO_PORT_06_PIN_03)
#define PMOD2_1_SSLB0_CTS_RTS0 (BSP_IO_PORT_06_PIN_04) /* See E14 / E15 in manual */
#define PMOD2_1_CTS0 (BSP_IO_PORT_06_PIN_05) /* See E10 in manual */
#define LCD_RST (BSP_IO_PORT_06_PIN_06)
#define SDRAM_DQ31 (BSP_IO_PORT_06_PIN_07)
#define SDRAM_A12 (BSP_IO_PORT_06_PIN_08)
#define SDRAM_DQ7 (BSP_IO_PORT_06_PIN_09)
#define SDRAM_DQ12 (BSP_IO_PORT_06_PIN_10)
#define SDRAM_DQ13 (BSP_IO_PORT_06_PIN_11)
#define SDRAM_DQ14 (BSP_IO_PORT_06_PIN_12)
#define SDRAM_DQ15 (BSP_IO_PORT_06_PIN_13)
#define SDRAM_DQM0 (BSP_IO_PORT_06_PIN_14)
#define SDRAM_DQM2 (BSP_IO_PORT_06_PIN_15)
#define CAM_D4 (BSP_IO_PORT_07_PIN_00) /* See SW4 in manual */
#define CAM_D5 (BSP_IO_PORT_07_PIN_01) /* See SW4 in manual */
#define CAM_D6 (BSP_IO_PORT_07_PIN_02) /* See SW4 in manual */
#define CAM_D7 (BSP_IO_PORT_07_PIN_03) /* See SW4 in manual */
#define PMOD2_10_GPIO (BSP_IO_PORT_07_PIN_04) /* Enable when connected */
#define CAM_D9 (BSP_IO_PORT_07_PIN_05)
#define CAM_D10 (BSP_IO_PORT_07_PIN_06)
#define LCD_D18_CAM_D11 (BSP_IO_PORT_07_PIN_07)
#define LCD_BLEN (BSP_IO_PORT_07_PIN_08)
#define CAM_RST (BSP_IO_PORT_07_PIN_09)
#define LCD_EXTCLK (BSP_IO_PORT_07_PIN_10)
#define LCD_D19 (BSP_IO_PORT_07_PIN_11)
#define LCD_D20 (BSP_IO_PORT_07_PIN_12)
#define LCD_D21 (BSP_IO_PORT_07_PIN_13)
#define LCD_D22 (BSP_IO_PORT_07_PIN_14)
#define LCD_D23 (BSP_IO_PORT_07_PIN_15)
#define OSPI_DQ5_PMOD1_1_CTS2 (BSP_IO_PORT_08_PIN_00) /* See SW4 in manual */
#define OSPI_DS (BSP_IO_PORT_08_PIN_01) /* See SW4 in manual */
#define OSPI_DQ6 (BSP_IO_PORT_08_PIN_02) /* See SW4 in manual */
#define OSPI_DQ1 (BSP_IO_PORT_08_PIN_03) /* See SW4 in manual */
#define OSPI_DQ7 (BSP_IO_PORT_08_PIN_04) /* See SW4 in manual */
#define LCD_HSYNC_TCON1 (BSP_IO_PORT_08_PIN_05) /* See SW4 in manual */
#define LCD_VSYNC_TCON0 (BSP_IO_PORT_08_PIN_06) /* See SW4 in manual */
#define LCD_DE_TCON2 (BSP_IO_PORT_08_PIN_07) /* See SW4 in manual */
#define OSPI_SCLK_ARDUINO_D0_RXD_MIKROBUS_RXD (BSP_IO_PORT_08_PIN_08) /* See SW4 in manual */
#define ARDUINO_D1_TXD_MIKROBUS_TXD (BSP_IO_PORT_08_PIN_09) /* Enable when connected */
#define ARDUINO_D4_MIKROBUS_1_PWM (BSP_IO_PORT_08_PIN_10) /* Enable when connected */
#define ARDUINO_D3 (BSP_IO_PORT_08_PIN_11) /* Enable when connected */
#define MEMS_MIC_CLK (BSP_IO_PORT_08_PIN_12)
#define SDRAM_CS (BSP_IO_PORT_08_PIN_13) /* See E50 in manual */
#define USBFS_DP (BSP_IO_PORT_08_PIN_14)
#define USBFS_DN (BSP_IO_PORT_08_PIN_15)
#define CAM_D1_LCD_D3 (BSP_IO_PORT_09_PIN_02) /* See SW4 in manual */
#define LCD_D2 (BSP_IO_PORT_09_PIN_03)
#define LCD_D8 (BSP_IO_PORT_09_PIN_04)
#define ETH_RX_CLK (BSP_IO_PORT_09_PIN_05) /* See E37 in manual */
#define ETH_RXD0 (BSP_IO_PORT_09_PIN_06) /* See E36 in manual */
#define ETH_RXD1 (BSP_IO_PORT_09_PIN_07) /* See E34 in manual */
#define ETH_RXD2 (BSP_IO_PORT_09_PIN_08) /* See E33 in manual */
#define ETH_RXD3 (BSP_IO_PORT_09_PIN_09) /* See E24 in manual */
#define LCD_D4 (BSP_IO_PORT_09_PIN_10) /* See SW4 in manual */
#define LCD_D5 (BSP_IO_PORT_09_PIN_11) /* See SW4 in manual */
#define LCD_D6 (BSP_IO_PORT_09_PIN_12) /* See SW4 in manual */
#define LCD_D7 (BSP_IO_PORT_09_PIN_13) /* See SW4 in manual */
#define LCD_D0 (BSP_IO_PORT_09_PIN_14) /* See SW4 in manual */
#define LCD_D1 (BSP_IO_PORT_09_PIN_15) /* See SW4 in manual */
#define SDRAM_A3 (BSP_IO_PORT_10_PIN_00)
#define SDRAM_A2 (BSP_IO_PORT_10_PIN_01)
#define SDRAM_A1 (BSP_IO_PORT_10_PIN_02)
#define SDRAM_A0 (BSP_IO_PORT_10_PIN_03)
#define SDRAM_DQM3 (BSP_IO_PORT_10_PIN_04)
#define SDRAM_DQM1 (BSP_IO_PORT_10_PIN_05)
#define SDRAM_CKE (BSP_IO_PORT_10_PIN_06)
#define USER_LED3_RED (BSP_IO_PORT_10_PIN_07)
#define SDRAM_WE (BSP_IO_PORT_10_PIN_08)
#define SDRAM_CAS (BSP_IO_PORT_10_PIN_09)
#define SDRAM_RAS (BSP_IO_PORT_10_PIN_10)
#define SDRAM_DQ8 (BSP_IO_PORT_10_PIN_11)
#define SDRAM_DQ9 (BSP_IO_PORT_10_PIN_12)
#define SDRAM_DQ10 (BSP_IO_PORT_10_PIN_13)
#define SDRAM_DQ11 (BSP_IO_PORT_10_PIN_14)
#define SDRAM_CLK (BSP_IO_PORT_10_PIN_15)
#define LCD_D17 (BSP_IO_PORT_11_PIN_00)
#define LCD_D13 (BSP_IO_PORT_11_PIN_01)
#define CAM_VSYNC_LCD_D16 (BSP_IO_PORT_11_PIN_02) /* See SW4 in manual */
#define CAM_HSYNC_LCD_D15 (BSP_IO_PORT_11_PIN_03) /* See SW4 in manual */
#define CAM_PCLK_LCD_D14 (BSP_IO_PORT_11_PIN_04) /* See SW4 in manual */
#define LCD_D12 (BSP_IO_PORT_11_PIN_05)
#define LCD_D11 (BSP_IO_PORT_11_PIN_06)
#define LCD_D10 (BSP_IO_PORT_11_PIN_07)
#define SDRAM_DQ30 (BSP_IO_PORT_12_PIN_00)
#define SDRAM_DQ29 (BSP_IO_PORT_12_PIN_01)
#define SDRAM_DQ28 (BSP_IO_PORT_12_PIN_02)
#define SDRAM_DQ27 (BSP_IO_PORT_12_PIN_03)
#define SDRAM_DQ26 (BSP_IO_PORT_12_PIN_04)
#define SDRAM_DQ25 (BSP_IO_PORT_12_PIN_05)
#define SDRAM_DQ24 (BSP_IO_PORT_12_PIN_06)
#define SDRAM_DQ23 (BSP_IO_PORT_12_PIN_07)
#define SDRAM_DQ22 (BSP_IO_PORT_12_PIN_08)
#define SDRAM_DQ21 (BSP_IO_PORT_12_PIN_09)
#define SDRAM_DQ20 (BSP_IO_PORT_12_PIN_10)
#define SDRAM_DQ19 (BSP_IO_PORT_12_PIN_11)
#define SDRAM_DQ18 (BSP_IO_PORT_12_PIN_12)
#define SDRAM_DQ17 (BSP_IO_PORT_12_PIN_13)
#define SDRAM_DQ16 (BSP_IO_PORT_12_PIN_14)
#define SDRAM_BA1 (BSP_IO_PORT_12_PIN_15)
#define SDRAM_BA0 (BSP_IO_PORT_13_PIN_00)
#define ARDUINO_D8_MIKROBUS_INT (BSP_IO_PORT_13_PIN_01) /* Enable when connected */
#define JLINK_VCOM_TX (BSP_IO_PORT_13_PIN_02)
#define JLINK_VCOM_RXD (BSP_IO_PORT_13_PIN_03)
#define JLINK_VCOM_CTS_RTS (BSP_IO_PORT_13_PIN_04) /* See E17 in manual */
#define JLINK_VCOM_CTS (BSP_IO_PORT_13_PIN_05) /* See E9 in manual */
#define AUDIO_MCLK (BSP_IO_PORT_13_PIN_06) /* Enable when connected */
#define USB_HS_VBUS_SEL (BSP_IO_PORT_13_PIN_07)

extern const ioport_cfg_t g_bsp_pin_cfg; /* Titan_Board */

void BSP_PinConfigSecurityInit();

/* Common macro for FSP header files. There is also a corresponding FSP_HEADER macro at the top of this file. */
FSP_FOOTER
#endif /* BSP_PIN_CFG_H_ */
