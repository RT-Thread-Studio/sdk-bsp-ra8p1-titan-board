/*
 * factory_test.h
 *
 *  Created on: 2025年7月22日
 *      Author: RTT
 */

#ifndef FACTORY_TEST_H_
#define FACTORY_TEST_H_

#define LED_RED			    BSP_IO_PORT_00_PIN_13
#define LED_GREEN			BSP_IO_PORT_06_PIN_13
#define RTC_NAME       		"rtc"
#define ADC_DEV_NAME		"adc0"
#define REFER_VOLTAGE       330
#define CONVERT_BITS        (1 << 16)
#define CAN0_DEV_NAME       "canfd0"	/* CAN0 设备名称 */
#define CAN1_DEV_NAME       "canfd1"	/* CAN1 设备名称 */

#define ERROR(err)          {test_flag.flag_err += (1 << err);rt_pin_write(LED_RED, PIN_LOW);}
#define ERROR_COUNT         (sizeof(error_str)/sizeof(error_str[0]))

enum {
    BUZZER_ERROR,
    RTC_ERROR,
    ADC0_CH0_ERROR,
    ADC0_CH1_ERROR,
    ADC0_CH2_ERROR,
    ADC0_CH3_ERROR,
    RS485_ERROR,
    CANFD_ERROR,
    BMI088_ERROR,
    IST8310_ERROR,
    CAMERA_LCD_ERROR,
    SDCARD_ERROR,
    FLASH_ERROR,
    HYPERRAM_ERROR,
    ETH_ERROR,
    WIFI_ERROR
};

extern const char *error_str[];

typedef struct flag {
	rt_bool_t    flag_test;
	rt_bool_t    flag_adc;
	rt_bool_t    flag_can;
	rt_bool_t    flag_bmi088;
	rt_bool_t    flag_ist8310;
	rt_bool_t    flag_wifi;
	rt_uint16_t  flag_err;
} flag_t;

extern flag_t test_flag;

void pwm_output(void);
int	 rtc_sample(void);
int  can_receive(void);
int  can_send(void);
int  adc_init(void);
void adc_channel_test(rt_uint8_t channel);
void wifi_connect(const char *ssid, const char *passwd);
void test_board_entry(void);

#endif /* FACTORY_TEST_H_ */
