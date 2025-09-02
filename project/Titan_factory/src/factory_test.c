/*
 * factory_test.c
 *
 *  Created on: 2025年7月22日
 *      Author: RTT
 */

#include <rtthread.h>
#include "hal_data.h"
#include <rtdevice.h>
#include <board.h>
#include <dev_wlan_mgnt.h>
#include "factory_test.h"
#include <netdev_ipaddr.h>
#include <netdev.h>
#include "drv_rs485.h"

#define DBG_TAG "factory"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

flag_t test_flag;

static struct rt_semaphore rx_sem; /* 用于接收消息的信号量 */
static rt_device_t can0_dev; /* CAN 设备句柄 */
static rt_device_t can1_dev; /* CAN 设备句柄 */
static rt_adc_device_t adc_dev = RT_NULL;

rt_bool_t can_count = RT_TRUE;

const char *error_str[] = { "BUZZER_ERROR", "RTC_ERROR", "ADC0_CH0_ERROR", "ADC0_CH1_ERROR", "ADC0_CH2_ERROR",
        "ADC0_CH3_ERROR", "RS485_ERROR", "CANFD_ERROR", "BMI088_ERROR", "IST8310_ERROR", "CAMERA_LCD_ERROR",
        "SDCARD_ERROR", "FLASH_ERROR", "HYPERRAM_ERROR", "ETH_ERROR", "WIFI_ERROR" };

typedef struct
{
    uint16_t freq;   // 频率Hz
    uint16_t duration; // 持续时间ms
} note_t;

note_t song[] = { { 262, 400 }, { 294, 400 }, { 330, 400 }, { 262, 400 },  // 1 2 3 1
        { 262, 400 }, { 294, 400 }, { 330, 400 }, { 262, 400 },  // 1 2 3 1
        { 330, 400 }, { 349, 400 }, { 392, 800 },             // 3 4 5
        { 330, 400 }, { 349, 400 }, { 392, 800 },             // 3 4 5
        { 392, 200 }, { 440, 200 }, { 392, 200 }, { 349, 200 }, { 330, 400 }, { 262, 400 }, // 5 6 5 4 3 1
        { 392, 200 }, { 440, 200 }, { 392, 200 }, { 349, 200 }, { 330, 400 }, { 262, 400 }, // 5 6 5 4 3 1
        { 262, 400 }, { 196, 400 }, { 262, 400 }, { 0, 400 },    // 1(低) 7(低) 1 高 休止
        { 262, 400 }, { 196, 400 }, { 262, 400 }, { 0, 400 },    // 1 7 1 休止
        };

int buzzer_test(void)
{
    struct rt_device_pwm *pwm_dev = (struct rt_device_pwm *) rt_device_find("pwm7");

    for (size_t i = 0; i < sizeof(song) / sizeof(song[0]); i++)
    {
        if (song[i].freq == 0)
        {
            rt_pwm_disable(pwm_dev, 0);
        }
        else
        {
            uint32_t period_ns = 1000000000 / song[i].freq;  // ns
            uint32_t pulse_ns = period_ns / 2;              // 50%

            rt_pwm_set(pwm_dev, 0, period_ns, pulse_ns);
            rt_pwm_enable(pwm_dev, 0);
        }

        rt_thread_mdelay(song[i].duration);
    }

    rt_pwm_disable(pwm_dev, 0);
    return 0;
}

/**
 * @brief PMW2、PWM10、PWM12 实现呼吸灯效果
 */
void breath_led_entry(void *parameter)
{
    struct rt_device_pwm *pwm2_dev; /* PWM设备句柄 */
    struct rt_device_pwm *pwm10_dev; /* PWM设备句柄 */
    struct rt_device_pwm *pwm12_dev; /* PWM设备句柄 */

    rt_uint32_t pulse = 0;
    rt_uint32_t period = 1000;
    rt_int8_t dir = 1;

    pwm2_dev = (struct rt_device_pwm *) rt_device_find("pwm2");
    pwm10_dev = (struct rt_device_pwm *) rt_device_find("pwm10");
    pwm12_dev = (struct rt_device_pwm *) rt_device_find("pwm12");

    while (1)
    {
        /* 设置 PWM 占空比 */
        rt_pwm_set(pwm2_dev, 0, period, pulse);
        rt_pwm_set(pwm10_dev, 0, period, pulse);
        rt_pwm_set(pwm12_dev, 0, period, pulse);
        rt_pwm_enable(pwm2_dev, 0);
        rt_pwm_enable(pwm10_dev, 0);
        rt_pwm_enable(pwm12_dev, 0);

        /* 更新占空比 */
        pulse += dir * 10;  // 每次调整 10us，占空比步进
        if (pulse >= period) /* 到顶点，开始减小 */
        {
            pulse = period;
            dir = -1;
        }
        else if (pulse <= 0) /* 到最低点，开始增加 */
        {
            pulse = 0;
            dir = 1;
        }

        rt_thread_mdelay(50); /* 调整呼吸灯速度 */
    }
}

static int breath_led(void)
{
    rt_thread_t tid = rt_thread_create("breath_led", breath_led_entry, RT_NULL, 1024, 20, 10);
    if (tid != RT_NULL)
        rt_thread_startup(tid);

    return RT_EOK;
}
INIT_APP_EXPORT(breath_led);

/* 接收数据回调函数 */
static rt_err_t can_rx_call(rt_device_t dev, rt_size_t size)
{
    /* CAN 接收到数据后产生中断，调用此回调函数，然后发送接收信号量 */
    rt_sem_release(&rx_sem);
    test_flag.flag_can = 1;
    return RT_EOK;
}

static void can_rx_thread(void *parameter)
{
    struct rt_can_msg rxmsg = { 0 };

    /* 设置接收回调函数 */
    rt_device_set_rx_indicate(can1_dev, can_rx_call);

    while (1)
    {
        /* hdr 值为 - 1，表示直接从 uselist 链表读取数据 */
        rxmsg.hdr_index = -1;
        /* 阻塞等待接收信号量 */
        rt_sem_take(&rx_sem, RT_WAITING_FOREVER);
        /* 从 CAN 读取一帧数据 */
        rt_device_read(can1_dev, 0, &rxmsg, sizeof(rxmsg));
        /* 打印数据 ID 及内容 */
        rt_kprintf("ID:%x\tmessege:", rxmsg.id);
        for (int i = 0; i < 8; i++)
        {
            rt_kprintf("%d", rxmsg.data[i]);
        }
        rt_kprintf("\n");
    }
}

static void can_tx_thread(void *parameter)
{
    rt_uint8_t count = 5;
    struct rt_can_msg msg = { 0 };
    rt_ssize_t size;
    msg.id = 0x78; /* ID 为 0x78 */
    msg.ide = RT_CAN_STDID; /* 标准格式 */
    msg.rtr = RT_CAN_DTR; /* 数据帧 */
    msg.len = 8; /* 数据长度为 8 */
    /* 待发送的 8 字节数据 */
    rt_memset(msg.data, 0, sizeof(msg.data));
    while (can_count)
    {
        /* 发送一帧 CAN 数据 */
        for (int i = 0; i < 8; i++)
        {
            msg.data[i] = i;
        }
        size = rt_device_write(can0_dev, 0, &msg, sizeof(msg));
        if (size == 0)
        {
            LOG_E("can dev write data failed!\n");
        }
        rt_thread_delay(1000);
    }
}

int can_receive(void)
{
    rt_err_t res;
    rt_thread_t thread;

    /* 查找 CAN 设备 */
    can1_dev = rt_device_find(CAN1_DEV_NAME);
    if (!can1_dev)
    {
        LOG_E("find %s failed!\n", CAN1_DEV_NAME);
        return RT_ERROR;
    }

    /* 初始化 CAN 接收信号量 */
    rt_sem_init(&rx_sem, "rx_sem", 0, RT_IPC_FLAG_FIFO);

    /* 以中断接收及发送方式打开 CAN 设备 */
    res = rt_device_open(can1_dev, RT_DEVICE_FLAG_INT_TX | RT_DEVICE_FLAG_INT_RX);
    RT_ASSERT(res == RT_EOK);
    /* 创建数据接收线程 */
    thread = rt_thread_create("can1_rx", can_rx_thread, RT_NULL, 1024, 25, 10);
    if (thread != RT_NULL)
    {
        rt_thread_startup(thread);
    }
    else
    {
        LOG_E("create can_rx thread failed!\n");
    }
    return res;
}

int can_send(void)
{
    rt_err_t res;
    rt_thread_t thread;

    test_flag.flag_can = 0;

    /* 查找 CAN 设备 */
    can0_dev = rt_device_find(CAN0_DEV_NAME);
    if (!can0_dev)
    {
        LOG_E("find %s failed!\n", CAN0_DEV_NAME);
        return RT_ERROR;
    }

    /* 以中断接收及发送方式打开 CAN 设备 */
    res = rt_device_open(can0_dev, RT_DEVICE_FLAG_INT_TX | RT_DEVICE_FLAG_INT_RX);
    RT_ASSERT(res == RT_EOK);
    /* 创建数据接收线程 */
    thread = rt_thread_create("can0_tx", can_tx_thread, RT_NULL, 1024, 25, 10);
    if (thread != RT_NULL)
    {
        rt_thread_startup(thread);
    }
    else
    {
        LOG_E("create can_rx thread failed!\n");
    }

    return res;
}

int adc_init(void)
{
    rt_err_t ret = RT_EOK;

    adc_dev = (rt_adc_device_t) rt_device_find(ADC_DEV_NAME);
    if (adc_dev == RT_NULL)
    {
        LOG_E("adc sample run failed! can't find %s device!", ADC_DEV_NAME);
        return RT_ERROR;
    }
    return RT_EOK;
}

void adc_channel_test(rt_uint8_t channel)
{
    rt_uint32_t vol, value = 0;
    test_flag.flag_adc = 0;
    int count = 0;

    rt_err_t res = rt_adc_enable(adc_dev, channel);
    if (RT_EOK != res)
    {
        LOG_E("ADC0 Channel%d Enable Failed!", channel);

        switch (channel)
        {
        case 0:
            ERROR(ADC0_CH0_ERROR)
            ;
            break;
        case 1:
            ERROR(ADC0_CH1_ERROR)
            ;
            break;
        case 2:
            ERROR(ADC0_CH2_ERROR)
            ;
            break;
        case 3:
            ERROR(ADC0_CH3_ERROR)
            ;
            break;
        }
    }

    for (int i = 0; i < 8; i++)
    {
        value = rt_adc_read(adc_dev, channel);
        if (value >= 0 && value < 65536)
        {
            count++;
        }
        vol = value * REFER_VOLTAGE / CONVERT_BITS;
        rt_kprintf("vol: %d\r\n", value);
        rt_kprintf("the adc voltage is: %d.%02d \n", vol / 100, vol % 100);
        rt_thread_mdelay(500);
    }

    if (count > 6)
        test_flag.flag_adc = 1;

    rt_adc_disable(adc_dev, channel);
}

int rtc_sample(void)
{
    rt_err_t ret = RT_EOK;
    time_t now;
    rt_device_t device = RT_NULL;

    device = rt_device_find(RTC_NAME);
    if (!device)
    {
        LOG_E("find %s failed!\n", RTC_NAME);
        return RT_ERROR;
    }

    if (rt_device_open(device, 0) != RT_EOK)
    {
        LOG_E("open %s failed!\n", RTC_NAME);
        return RT_ERROR;
    }

    ret = set_date(2025, 8, 1);
    rt_kprintf("set RTC date to 2025-8-1\n");
    if (ret != RT_EOK)
    {
        LOG_E("set RTC date failed\n");
        return ret;
    }
    ret = set_time(15, 35, 55);
    if (ret != RT_EOK)
    {
        LOG_E("set RTC time failed\n");
        return ret;
    }
    rt_thread_mdelay(1000);
    rt_uint8_t err = get_timestamp(&now);
    rt_kprintf("now: %.*s", 25, ctime(&now));

    return ret;
}

void wifi_connect(const char *ssid, const char *passwd)
{
    rt_uint8_t count = 0;
    while (rt_wlan_is_ready() != RT_TRUE)
    {
        if (count > 10)
        {
            LOG_E("WiFi Connect Failed!\n");
            ERROR(WIFI_ERROR);
            break;
        }
        rt_wlan_connect(ssid, passwd);
        rt_thread_mdelay(1000);
        count++;
    }
}

rt_err_t wifi_test()
{
    test_flag.flag_wifi = 0;
    rt_kprintf("[Wi-Fi]Start ping!\r\n");

    struct netdev *netdev = netdev_get_by_name("w0");
    if (netdev == RT_NULL)
    {
        LOG_E("error\r\n");
    }
    char * target_name = (char *) (ip4addr_ntoa(&(netdev->gw)));
    if (netdev_cmd_ping(target_name, "w0", 4, 0) != RT_EOK)
    {
        return -RT_ERROR;
    }
    return RT_EOK;
}

rt_err_t eth_test()
{
    rt_kprintf("[ETH]Start ping!\r\n");

    struct netdev *netdev = netdev_get_by_name("e0");
    if (netdev == RT_NULL)
    {
        LOG_E("error\r\n");
    }
    char * target_name = (char *) (ip4addr_ntoa(&(netdev->gw)));
    netdev = netdev_get_first_by_flags(NETDEV_FLAG_LINK_UP);
    if (netdev_cmd_ping(target_name, "e0", 4, 0) != RT_EOK)
    {
        return -RT_ERROR;
    }
    return RT_EOK;
}

void print_error_info(void)
{
    if (test_flag.flag_err == 0)
    {
        rt_pin_write(LED_GREEN, PIN_LOW);
        LOG_I("=====================ALL Test Run Successful!=====================");
        return;
    }

    LOG_E("ERROR Flag is %d", test_flag.flag_err);
    LOG_E("Detected errors:");

    for (int i = 0; i < ERROR_COUNT; i++)
    {
        if (test_flag.flag_err & (1 << i))
        {
            rt_kprintf(" - %s\n", error_str[i]);
        }
    }
}

void test_board_entry(void)
{
   rt_kprintf("\n=====================BUZZER TEST=============================\n");
   if (buzzer_test() != RT_EOK)
   {
       LOG_E("BUZZER Test Failed!\n");
       ERROR(BUZZER_ERROR);
   }
   rt_kprintf("=====================BUZZER TEST END=============================\n\n");

    rt_thread_mdelay(10);

    rt_kprintf("=====================HyperRAM TEST=============================\n");
    extern rt_err_t hyper_ram_test();
    if (RT_EOK != hyper_ram_test())
    {
        LOG_E("HyperRAM Test Failed!\n");
        ERROR(HYPERRAM_ERROR);
    }
    rt_kprintf("=====================HyperRAM TEST END=============================\n\n");

    rt_thread_mdelay(10);

    rt_kprintf("=====================WIFI TEST=============================\n");
    if (wifi_test() != RT_EOK)
    {
        LOG_E("Wi-Fi Test Failed!\n");
        ERROR(WIFI_ERROR);
    }
    rt_kprintf("=====================WIFI TEST END=============================\n\n");

    rt_thread_mdelay(10);

    rt_kprintf("=====================ETH TEST=============================\n");
    if (eth_test() != RT_EOK)
    {
        LOG_E("ETH Test Failed!\n");
        ERROR(ETH_ERROR);
    }
    rt_kprintf("=====================ETH TEST END=============================\n\n");

    rt_thread_mdelay(10);

    rt_kprintf("=====================RTC TEST=============================\n");
    if (rtc_sample() != RT_EOK)
    {
        LOG_E("RTC Test Failed\r\n");
        ERROR(RTC_ERROR);
    }
    rt_kprintf("=====================RTC TEST END=============================\n\n");

    rt_thread_mdelay(10);

    rt_kprintf("=====================ADC TEST=============================\n");
    rt_kprintf("=====================ADC0-CHANNEL0 Test Start=============================\n");
    adc_channel_test(0);
    if (test_flag.flag_adc == 0)
    {
        LOG_E("ADC0-CHANNEL0 Test Failed!\n");
        ERROR(ADC0_CH0_ERROR);
    }

    rt_kprintf("=====================ADC0-CHANNEL0 END Test=============================\n");

    rt_thread_mdelay(10);

    rt_kprintf("=====================ADC0-CHANNEL1 Test Start=============================\n");
    adc_channel_test(1);
    if (test_flag.flag_adc == 0)
    {
        LOG_E("ADC0-CHANNEL1 Test Failed!\n");
        ERROR(ADC0_CH1_ERROR);
    }
    rt_kprintf("=====================ADC0-CHANNEL1 Test END=============================\n");

    rt_thread_mdelay(10);

    rt_kprintf("=====================ADC0-CHANNEL2 Test Start=============================\n");
    adc_channel_test(2);
    if (test_flag.flag_adc == 0)
    {
        LOG_E("ADC0-CHANNEL2 Test Failed!\n");
        ERROR(ADC0_CH2_ERROR);
    }
    rt_kprintf("=====================ADC0-CHANNEL2 END Test=============================\n");

    rt_thread_mdelay(10);

    rt_kprintf("=====================ADC0-CHANNEL3 Start Test=============================\n");
    adc_channel_test(3);
    if (test_flag.flag_adc == 0)
    {
        LOG_E("ADC0-CHANNEL3 Test Failed!\n");
        ERROR(ADC0_CH3_ERROR);
    }
    rt_kprintf("=====================ADC0-CHANNEL3 Test END=============================\n");
    rt_kprintf("=====================ADC TEST END=============================\n\n");

    rt_thread_mdelay(10);

    rt_kprintf("=====================CANFD TEST=============================\n");
    can_receive();
    rt_thread_mdelay(500);
    can_send();
    rt_thread_mdelay(2000);
    extern rt_bool_t can_count;
    can_count = RT_FALSE;
    if (test_flag.flag_can != 1)
    {
        LOG_E("CAN Test Failed!\n");
        ERROR(CANFD_ERROR);
    };
    rt_kprintf("=====================CANFD TEST END=============================\n\n");

    rt_thread_mdelay(100);

    rt_kprintf("=====================BMI088 TEST=============================\n");
    msh_exec("bmi088_app", strlen("bmi088_app"));
    rt_thread_mdelay(1500);
    if (test_flag.flag_bmi088 != RT_TRUE)
    {
        LOG_E("BMI088 Test Failed!\n");
        ERROR(BMI088_ERROR);
    }
    rt_kprintf("=====================BMI088 TEST END=============================\n\n");

    rt_kprintf("=====================IST8310 TEST=============================\n");
    msh_exec("ist8310_app", strlen("ist8310_app"));
    rt_thread_mdelay(1500);
    if (test_flag.flag_ist8310 != RT_TRUE)
    {
        LOG_E("IST8310 Test Failed!\n");
        ERROR(IST8310_ERROR);
    }
    rt_kprintf("=====================IST8310 TEST END=============================\n\n");

    print_error_info();
}
