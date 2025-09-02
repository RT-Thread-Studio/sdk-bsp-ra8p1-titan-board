//#include <stdio.h>
//#include <string.h>
//#include "rpmsg_lite.h"
//#include "rpmsg_queue.h"
//#include "rpmsg_ns.h"
//#include "hal_data.h"
//#include <rtthread.h>
//
//#define DRV_DEBUG
//#define LOG_TAG             "rpmsg_master"
//#include <drv_log.h>
//
//#define LOCAL_EPT_ADDR             0x00
//#define MAX_MESSAGE_COUNT          100
//#define RPMSG_LITE_SHMEM_BASE      0x22080000
//#define SH_MEM_TOTAL_SIZE          (64 * 1024)
//
//// M33 -> M85 消息结构
//typedef struct m33_to_m85_msg {
//    uint32_t index;               // 消息索引
//    float left_rear_rpm;          // 左后轮转速 (RPM)
//    float right_rear_rpm;         // 右后轮转速 (RPM)
//    float front_steering_angle;   // 前轮转向角 (度)
//    uint32_t checksum;            // 校验和
//} M33_TO_M85_MSG;
//
//// M85 -> M33 消息结构
//typedef struct m85_to_m33_msg {
//    uint32_t index;               // 消息索引
//    float vehicle_speed;          // 车速 (km/h)
//    float steering_angle;         // 转向角 (度)
//    uint32_t checksum;            // 校验和
//} M85_TO_M33_MSG;
//
//// 计算校验和
//static uint32_t calculate_checksum(const void *msg, size_t size) {
//    uint32_t sum = 0;
//    const uint8_t *bytes = (const uint8_t *)msg;
//    // 除去 checksum 字段
//    for (size_t i = 0; i < size - sizeof(uint32_t); i++) {
//        sum += bytes[i];
//    }
//    return sum;
//}
//
//// 验证校验和
//static int verify_checksum(const void *msg, size_t size, uint32_t received_checksum) {
//    uint32_t expected = calculate_checksum(msg, size);
//    return (expected == received_checksum) ? 0 : -1;
//}
//
//static void print_m33_to_m85_msg(const M33_TO_M85_MSG *msg, const char *direction)
//{
//    LOG_I("[%s][%u][%u] M33 → M85 | LEFT:%d.%02d RPM | RIGHT:%d.%02d RPM | STEER:%d.%02d°",
//          direction,
//          msg->index,
//          msg->checksum,
//          (int)msg->left_rear_rpm, abs((int)(msg->left_rear_rpm * 100) % 100),
//          (int)msg->right_rear_rpm, abs((int)(msg->right_rear_rpm * 100) % 100),
//          (int)msg->front_steering_angle, abs((int)(msg->front_steering_angle * 100) % 100));
//}
//
//// 打印 M85 -> M33
//static void print_m85_to_m33_msg(const M85_TO_M33_MSG *msg, const char *direction)
//{
//    LOG_I("[%s][%u][%u] M85 → M33 | SEPPD:%d.%02d KM/H | STEER:%d.%02d°",
//          direction,
//          msg->index,
//          msg->checksum,
//          (int)msg->vehicle_speed, abs((int)(msg->vehicle_speed * 100) % 100),
//          (int)msg->steering_angle, abs((int)(msg->steering_angle * 100) % 100));
//}
//
//static M33_TO_M85_MSG rx_msg = {0};
//static M85_TO_M33_MSG tx_msg = {0};
//rt_mailbox_t my_rpmsg_mb;
//
//static void app_nameservice_isr_cb(uint32_t new_ept, const char *new_ept_name, uint32_t flags, void *user_data) {
//    uint32_t *data = (uint32_t *)user_data;
//    *data = new_ept;
//}
//
//void rpmsg_entry(void *param) {
//    volatile uint32_t remote_addr = 0U;
//    struct rpmsg_lite_endpoint *my_ept;
//    rpmsg_queue_handle my_queue;
//    struct rpmsg_lite_instance *my_rpmsg;
//    uint32_t len;
//
//    // 初始化 rpmsg-lite
//    my_rpmsg = rpmsg_lite_master_init((void *)RPMSG_LITE_SHMEM_BASE, SH_MEM_TOTAL_SIZE, RL_PLATFORM_RA8P1_M85_LINK_ID, RL_NO_FLAGS);
//    if (!my_rpmsg) {
//        rt_kprintf("rpmsg_lite_master_init failed!\r\n");
//        return;
//    }
//
//    my_queue = rpmsg_queue_create(my_rpmsg);
//    my_ept = rpmsg_lite_create_ept(my_rpmsg, LOCAL_EPT_ADDR, rpmsg_queue_rx_cb, my_queue);
//    if (!my_ept || !my_queue) {
//        rt_kprintf("Failed to create endpoint or queue!\r\n");
//        return;
//    }
//
//    rt_thread_mdelay(100);
//
//    // 初始化发送消息
//    tx_msg.index = 0;
//    tx_msg.vehicle_speed = 10.0f; // 模拟初始车速
//    tx_msg.steering_angle = 0.0f; // 模拟初始转向角
//    tx_msg.checksum = calculate_checksum(&tx_msg, sizeof(M85_TO_M33_MSG));
//    print_m85_to_m33_msg(&tx_msg, "TX");
//
//    // 发送第一条消息
//    if (rpmsg_lite_send(my_rpmsg, my_ept, remote_addr, (char *)&tx_msg, sizeof(M85_TO_M33_MSG), RL_DONT_BLOCK) != RL_SUCCESS) {
//        rt_kprintf("Initial message send failed!\r\n");
//    }
//
//    while (rx_msg.index < MAX_MESSAGE_COUNT) {
//        rt_kprintf("Waiting for message...\r\n");
//        // 接收 M33 的消息
//        if (rpmsg_queue_recv(my_rpmsg, my_queue, (uint32_t *)&remote_addr, (char *)&rx_msg, sizeof(M33_TO_M85_MSG), &len, RL_BLOCK) != RL_SUCCESS) {
//            rt_kprintf("Failed to receive message!\r\n");
//            continue;
//        }
//
//        // 验证校验和
//        if (verify_checksum(&rx_msg, sizeof(M33_TO_M85_MSG), rx_msg.checksum) != 0) {
//            rt_kprintf("[ERROR] Checksum verification failed for index %u!\r\n", rx_msg.index);
//            continue;
//        }
//
//        // 打印接收到的消息
//        print_m33_to_m85_msg(&rx_msg, "RX");
//
//        // 准备回复消息
//        tx_msg.index = rx_msg.index + 1;
//        tx_msg.vehicle_speed += 0.5f; // 模拟车速递增
//        tx_msg.steering_angle = rx_msg.front_steering_angle * 0.8f; // 模拟转向角处理
//        tx_msg.checksum = calculate_checksum(&tx_msg, sizeof(M85_TO_M33_MSG));
//        print_m85_to_m33_msg(&tx_msg, "TX");
//
//        // 发送回复消息
//        if (rpmsg_lite_send(my_rpmsg, my_ept, remote_addr, (char *)&tx_msg, sizeof(M85_TO_M33_MSG), RL_BLOCK) != RL_SUCCESS) {
//            rt_kprintf("Failed to send message for index %u!\r\n", tx_msg.index);
//        }
//    }
//
//    // 清理资源
//    rpmsg_lite_destroy_ept(my_rpmsg, my_ept);
//    rpmsg_queue_destroy(my_rpmsg, my_queue);
//    rpmsg_lite_deinit(my_rpmsg);
//
//    rt_kprintf("\r\nRPMsg demo ends\r\n");
//}
//
//int rp_ping_pong(void) {
//    rt_thread_t tid = rt_thread_create("rpmsg", rpmsg_entry, RT_NULL, 1024, 25, 5);
//    if (tid != RT_NULL) {
//        rt_thread_startup(tid);
//    } else {
//        rt_kprintf("Failed to create rpmsg thread!\r\n");
//    }
//    return 0;
//}
//MSH_CMD_EXPORT(rp_ping_pong, rp_ping_pong);
