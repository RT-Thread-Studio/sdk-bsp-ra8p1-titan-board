// /**
//  * @file app.c
//  * @author Wu TongXing (Hiwonder)
//  * @version 0.1
//  * @date 2023-09-21
//  * @copyright Copyright (c) 2023
//  * @brief 读取总线舵机的 ID、位置、偏差、限位、电压阈值、电压、过温阈值、温度、是否上电
//  */

// //#include "global.h"
// #include <rtthread.h>
// #include "serial_servo.h"


// // 声明外部变量 总线舵机控制器
// extern SerialServoControllerTypeDef serial_servo_controller;

// // 总线舵机初始化
// void serial_servo_init(void);

// /* 用户入口函数 */
// void app_task_entry(void *argument)
// {
// 	uint32_t broadcast_id = 0xFE; //这里为广播ID，不可更改
// 	int ret = 0;
// 	uint8_t ret_servo_id = 0;
// 	uint32_t servo_id_new = 0;
	
// 	/* 串口舵机控制初始化 */
// 	serial_servo_init();

// 	while(1)
// 	{
// 		//读取舵机ID，只能连接一个舵机，否则会有总线冲突
// 		ret = serial_servo_read_id(&serial_servo_controller , broadcast_id , &ret_servo_id);
// 		if(ret == 0)
// 		{
// 			//控制舵机运动到 0 位置
// 			serial_servo_set_position(&serial_servo_controller , ret_servo_id , 0 , 1000);
// 			rt_thread_mdelay(1000); //等待运动完成
// 			break;
// 		}else{
	
// 		}
// 		rt_thread_mdelay(100);
// 	}

// 	servo_id_new = 0x01;
// 	//设置舵机ID
// 	serial_servo_set_id(&serial_servo_controller , ret_servo_id , servo_id_new);


// 	ret_servo_id = 0;
// 	while(1)
// 	{
// 		//读取舵机ID，只能连接一个舵机，否则会有总线冲突
// 		ret = serial_servo_read_id(&serial_servo_controller , broadcast_id , &ret_servo_id);
// 		if(ret == 0)
// 		{
// 			printf("servo new id is %d\r\n",ret_servo_id);
// 			break;
// 		}else{
// 			printf("read new id fail, again\r\b");
// 		}
// 		rt_thread_mdelay(100);
// 	}

// 	//判断是否设置成功
// 	if(ret_servo_id == servo_id_new)
// 	{
// 		printf("set servo id is success\r\n");
// 		//若设置成功，则可以用新的ID号控制舵机运动
// 		serial_servo_set_position(&serial_servo_controller , servo_id_new , 750 , 1000);
// 		rt_thread_mdelay(1000); //等待运动完成
// 	}else{
// 		printf("set servo id is fail\r\n");
// 	}

// 	serial_servo_stop(&serial_servo_controller , ret_servo_id); //使舵机停止运动
	
// 	// 循环  : RTOS任务中的循环，必须要有rt_thread_mdelay或者其他系统阻塞函数，否则会导致系统异常
//     for(;;) 
// 	{
// 		printf("serial_servo_change_id_testing...\r\n");
// 		rt_thread_mdelay(300);
// 	}
	
// }
// MSH_CMD_EXPORT(app_task_entry, app_task_entry);

// /*
//  * 程序清单：这是一个串口设备 开启 DMA 模式后使用例程
//  * 例程导出了 uart_dma_sample 命令到控制终端
//  * 命令调用格式：uart_dma_sample uart1
//  * 命令解释：命令第二个参数是要使用的串口设备名称，为空则使用默认的串口设备
//  * 程序功能：通过串口输出字符串 "hello RT-Thread!"，并通过串口输出接收到的数据，然后打印接收到的数据。
// */

// #include <rtthread.h>
// #include <rtdevice.h>

// #define SAMPLE_UART_NAME       "uart5"      /* 串口设备名称 */

// /* 串口接收消息结构 */
// struct rx_msg
// {
//     rt_device_t dev;
//     rt_size_t size;
// };
// /* 串口设备句柄 */
// static rt_device_t serial;
// /* 消息队列控制块 */
// static struct rt_messagequeue rx_mq;

// /* 接收数据回调函数 */
// static rt_err_t uart_input(rt_device_t dev, rt_size_t size)
// {
//     struct rx_msg msg;
//     rt_err_t result;
//     msg.dev = dev;
//     msg.size = size;

//     result = rt_mq_send(&rx_mq, &msg, sizeof(msg));
//     if (result == -RT_EFULL)
//     {
//         /* 消息队列满 */
//         rt_kprintf("message queue full！\n");
//     }
//     return result;
// }

// static void serial_thread_entry(void *parameter)
// {
//     struct rx_msg msg;
//     rt_err_t result;
//     rt_uint32_t rx_length;
//     static char rx_buffer[BSP_UART5_RX_BUFSIZE + 1];

//     while (1)
//     {
//         rt_memset(&msg, 0, sizeof(msg));
//         /* 从消息队列中读取消息 */
//         result = rt_mq_recv(&rx_mq, &msg, sizeof(msg), RT_WAITING_FOREVER);
//         if (result > 0)
//         {
//             /* 从串口读取数据 */
//             rx_length = rt_device_read(msg.dev, 0, rx_buffer, msg.size);
//             rx_buffer[rx_length] = '\0';
//             /* 通过串口设备 serial 输出读取到的消息 */
//             rt_device_write(serial, 0, rx_buffer, rx_length);
//             /* 打印数据 */
//             rt_kprintf("%s\n",rx_buffer);
//         }
//     }
// }

// static int uart_dma_sample(int argc, char *argv[])
// {
//     rt_err_t ret = RT_EOK;
//     char uart_name[RT_NAME_MAX];
//     static char msg_pool[256];
//     char str[] = {0x55, 0x55, 0xFE, 0x03, 0x0E, 0xF0};
// //    char str[] = {0x55, 0x55, 0x01, 0x07, 0x01, 0xF4, 0x01, 0xE8, 0x03, 0xF6};
// //    char str[] = {0x55, 0x55, 0x01, 0x03, 0x30, 0xCB};

//     if (argc == 2)
//     {
//         rt_strncpy(uart_name, argv[1], RT_NAME_MAX);
//     }
//     else
//     {
//         rt_strncpy(uart_name, SAMPLE_UART_NAME, RT_NAME_MAX);
//     }

//     /* 查找串口设备 */
//     serial = rt_device_find(uart_name);
//     if (!serial)
//     {
//         rt_kprintf("find %s failed!\n", uart_name);
//         return RT_ERROR;
//     }

//     /* 初始化消息队列 */
//     rt_mq_init(&rx_mq, "rx_mq",
//                msg_pool,                 /* 存放消息的缓冲区 */
//                sizeof(struct rx_msg),    /* 一条消息的最大长度 */
//                sizeof(msg_pool),         /* 存放消息的缓冲区大小 */
//                RT_IPC_FLAG_FIFO);        /* 如果有多个线程等待，按照先来先得到的方法分配消息 */

//     /* 以 DMA 接收及轮询发送方式打开串口设备 */
//     rt_device_open(serial, RT_DEVICE_FLAG_RX_NON_BLOCKING | RT_DEVICE_FLAG_TX_BLOCKING);
//     /* 设置接收回调函数 */
//     rt_device_set_rx_indicate(serial, uart_input);
//     /* 发送字符串 */
// //    rt_device_write(serial, 0, str, (sizeof(str)) - 1);

//     while(1)
//     {
//         rt_device_write(serial, 0, str, (sizeof(str)));
//         rt_thread_mdelay(1000);
//     }

//     /* 创建 serial 线程 */
//     rt_thread_t thread = rt_thread_create("serial", serial_thread_entry, RT_NULL, 1024, 25, 10);
//     /* 创建成功则启动线程 */
//     if (thread != RT_NULL)
//     {
//         rt_thread_startup(thread);
//     }
//     else
//     {
//         ret = RT_ERROR;
//     }

//     return ret;
// }
// /* 导出到 msh 命令列表中 */
// MSH_CMD_EXPORT(uart_dma_sample, uart device dma sample);
