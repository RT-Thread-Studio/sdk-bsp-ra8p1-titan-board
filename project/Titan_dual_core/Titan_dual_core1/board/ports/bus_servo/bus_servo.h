/*
 * Copyright (c) 2006-2025, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author        Notes
 * 2025-07-04     kurisaW       first version
 */

#ifndef __BUS_SERVO_H__
#define __BUS_SERVO_H__

#include <rtthread.h>
#include <rtdevice.h>

#define DBG_TAG "bus.servo"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#define LOG_HEX_SERVO_ENABLE 0

#if LOG_HEX_SERVO_ENABLE
#define LOG_HEX_SERVO(desc, expect_len, buf, actual_len) \
    do { \
            rt_kprintf("[D/%s] %s (expect:%d actual:%d)\n", DBG_TAG, desc, expect_len, actual_len); \
            rt_kprintf("HEX: "); \
            for (int _i = 0; _i < actual_len; _i++) { \
                rt_kprintf("%02X ", (buf)[_i]); \
                if ((_i + 1) % 8 == 0) rt_kprintf("\n      "); /* 每8字节换行 */ \
            } \
            rt_kprintf("\nASCII: "); \
            for (int _i = 0; _i < actual_len; _i++) { \
                rt_kprintf("%c", ((buf)[_i] >= 0x20 && (buf)[_i] <= 0x7E) ? (buf)[_i] : '.'); \
                if ((_i + 1) % 8 == 0) rt_kprintf("\n       "); \
            } \
            if (expect_len != actual_len) { \
                rt_kprintf("\n>> LENGTH ERROR! <<"); \
            } \
            rt_kprintf("\n"); \
    } while (0)
#else
#define LOG_HEX_SERVO
#endif


/* 舵机指令定义 */
#define SERVO_CMD_MOVE_TIME_WRITE          0x01
#define SERVO_CMD_MOVE_TIME_READ           0x02
#define SERVO_CMD_MOVE_TIME_WAIT_WRITE     0x07
#define SERVO_CMD_MOVE_TIME_WAIT_READ      0x08
#define SERVO_CMD_MOVE_START               0x0B
#define SERVO_CMD_MOVE_STOP                0x0C
#define SERVO_CMD_ID_WRITE                 0x0D
#define SERVO_CMD_ID_READ                  0x0E
#define SERVO_CMD_ANGLE_OFFSET_ADJUST      0x11
#define SERVO_CMD_ANGLE_OFFSET_WRITE       0x12
#define SERVO_CMD_ANGLE_OFFSET_READ        0x13
#define SERVO_CMD_ANGLE_LIMIT_WRITE        0x14
#define SERVO_CMD_ANGLE_LIMIT_READ         0x15
#define SERVO_CMD_VIN_LIMIT_WRITE          0x16
#define SERVO_CMD_VIN_LIMIT_READ           0x17
#define SERVO_CMD_TEMP_MAX_LIMIT_WRITE     0x18
#define SERVO_CMD_TEMP_MAX_LIMIT_READ      0x19
#define SERVO_CMD_TEMP_READ                0x1A
#define SERVO_CMD_VIN_READ                 0x1B
#define SERVO_CMD_POS_READ                 0x1C
#define SERVO_CMD_OR_MOTOR_MODE_WRITE      0x1D
#define SERVO_CMD_OR_MOTOR_MODE_READ       0x1E
#define SERVO_CMD_LOAD_OR_UNLOAD_WRITE     0x1F
#define SERVO_CMD_LOAD_OR_UNLOAD_READ      0x20
#define SERVO_CMD_LED_CTRL_WRITE           0x21
#define SERVO_CMD_LED_CTRL_READ            0x22
#define SERVO_CMD_LED_ERROR_WRITE          0x23
#define SERVO_CMD_LED_ERROR_READ           0x24
#define SERVO_CMD_DIS_READ                 0x30

/* 广播ID */
#define SERVO_BROADCAST_ID                 0xFE

/* 舵机模式 */
#define SERVO_MODE_POSITION                0x00
#define SERVO_MODE_MOTOR                   0x01

/* 电机模式 */
#define MOTOR_MODE_DUTY                    0x00
#define MOTOR_MODE_SPEED                   0x01

/* 舵机状态 */
#define SERVO_LOAD                         0x01
#define SERVO_UNLOAD                       0x00

/* LED状态 */
#define LED_ON                             0x00
#define LED_OFF                            0x01

/* 错误标志 */
#define ERROR_OVER_TEMP                    0x01
#define ERROR_OVER_VOLTAGE                 0x02
#define ERROR_STALL                        0x04

/* 最大角度 */
#define SERVO_MAX_ANGLE                    1000
#define SERVO_MIN_ANGLE                    0

/* 角度范围 */
#define SERVO_ANGLE_RANGE                  240.0f
#define SERVO_ANGLE_OFFSET_RANGE          30.0f

/* 响应超时时间 */
#define SERVO_RESPONSE_TIMEOUT             100  // ms

/* 舵机控制结构体 */
struct servo_cmd {
    rt_uint8_t id;
    rt_uint16_t angle;
    rt_uint16_t time;
};

/* 舵机信息结构体 */
struct servo_info {
    rt_uint8_t id;
    rt_int16_t angle_offset;
    rt_uint16_t angle_min;
    rt_uint16_t angle_max;
    rt_uint16_t vin_min;
    rt_uint16_t vin_max;
    rt_uint8_t temp_max;
    rt_uint8_t mode;
    rt_uint8_t motor_mode;
    rt_int16_t speed;
    rt_uint8_t load_state;
    rt_uint8_t led_state;
    rt_uint8_t error_mask;
};

/* 函数声明 */
int bus_servo_init(const char *uart_name);
int bus_servo_deinit(void);

int bus_servo_move(rt_uint8_t id, rt_uint16_t angle, rt_uint16_t time);
int bus_servo_move_wait(rt_uint8_t id, rt_uint16_t angle, rt_uint16_t time);
int bus_servo_move_start(rt_uint8_t id);
int bus_servo_move_stop(rt_uint8_t id);

int bus_servo_set_id(rt_uint8_t old_id, rt_uint8_t new_id);
int bus_servo_get_id(rt_uint8_t *id);

int bus_servo_set_angle_offset(rt_uint8_t id, rt_int8_t offset);
int bus_servo_save_angle_offset(rt_uint8_t id);
int bus_servo_get_angle_offset(rt_uint8_t id, rt_int8_t *offset);

int bus_servo_set_angle_limit(rt_uint8_t id, rt_uint16_t min, rt_uint16_t max);
int bus_servo_get_angle_limit(rt_uint8_t id, rt_uint16_t *min, rt_uint16_t *max);

int bus_servo_set_vin_limit(rt_uint8_t id, rt_uint16_t min, rt_uint16_t max);
int bus_servo_get_vin_limit(rt_uint8_t id, rt_uint16_t *min, rt_uint16_t *max);

int bus_servo_set_temp_limit(rt_uint8_t id, rt_uint8_t temp);
int bus_servo_get_temp_limit(rt_uint8_t id, rt_uint8_t *temp);

int bus_servo_get_temp(rt_uint8_t id, rt_uint8_t *temp);
int bus_servo_get_vin(rt_uint8_t id, rt_uint16_t *vin);
int bus_servo_get_pos(rt_uint8_t id, rt_int16_t *pos);

int bus_servo_set_mode(rt_uint8_t id, rt_uint8_t mode, rt_uint8_t motor_mode, rt_int16_t speed);
int bus_servo_get_mode(rt_uint8_t id, rt_uint8_t *mode, rt_uint8_t *motor_mode, rt_int16_t *speed);

int bus_servo_set_load(rt_uint8_t id, rt_uint8_t state);
int bus_servo_get_load(rt_uint8_t id, rt_uint8_t *state);

int bus_servo_set_led(rt_uint8_t id, rt_uint8_t state);
int bus_servo_get_led(rt_uint8_t id, rt_uint8_t *state);

int bus_servo_set_error_mask(rt_uint8_t id, rt_uint8_t mask);
int bus_servo_get_error_mask(rt_uint8_t id, rt_uint8_t *mask);

int bus_servo_get_distance(rt_uint8_t id, rt_uint32_t *distance);

/* 命令行函数 */
int bus_servo_cmd(int argc, char **argv);
float positionToAngle(int position);

#endif /* __BUS_SERVO_H__ */
