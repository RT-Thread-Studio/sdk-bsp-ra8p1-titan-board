/*
 * Copyright (c) 2006-2025, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author        Notes
 * 2025-07-04     kurisaW       first version
 */

#include "bus_servo.h"

static struct rt_device *servo_uart = RT_NULL;
static struct rt_mutex servo_mutex;

/* 计算校验和 */
static rt_uint8_t calculate_checksum(rt_uint8_t *data, rt_uint8_t len)
{
    rt_uint16_t sum = 0;

    for (rt_uint8_t i = 0; i < len; i++) {
        sum += data[i];
    }

    return (rt_uint8_t)(~(sum & 0xFF));
}

/* 发送指令 */
static int send_command(rt_uint8_t id, rt_uint8_t cmd, rt_uint8_t *params, rt_uint8_t param_len)
{
    rt_uint8_t buf[128];
    rt_uint8_t len = 0;
    rt_size_t sent;

    if (servo_uart == RT_NULL) {
        LOG_E("Servo UART not initialized");
        return -RT_ERROR;
    }

    /* 构建指令包 */
    buf[len++] = 0x55;
    buf[len++] = 0x55;
    buf[len++] = id;
    buf[len++] = param_len + 3;  // 数据长度 = 参数长度 + 3 (ID + Length + Cmd)
    buf[len++] = cmd;

    if (params != RT_NULL && param_len > 0) {
        rt_memcpy(&buf[len], params, param_len);
        len += param_len;
    }

    /* 计算校验和 */
    buf[len++] = calculate_checksum(&buf[2], len - 2);

    /* 发送数据 */
    rt_mutex_take(&servo_mutex, RT_WAITING_FOREVER);
    sent = rt_device_write(servo_uart, 0, buf, len);
    rt_mutex_release(&servo_mutex);

    if (sent != len) {
        LOG_E("Failed to send command, sent %d/%d bytes", sent, len);
        return -RT_ERROR;
    }

    return RT_EOK;
}

static int receive_response(rt_uint8_t *buf, rt_uint8_t buf_size, rt_uint32_t timeout)
{
    rt_size_t recv_len = 0;
    rt_tick_t start_tick = rt_tick_get();
    rt_uint8_t expected_len = 0;

    if (servo_uart == RT_NULL) {
        LOG_E("Servo UART not initialized");
        return -RT_ERROR;
    }

    while (1) {
        rt_size_t read_len = rt_device_read(servo_uart, 0, buf + recv_len, buf_size - recv_len);

        if (read_len > 0) {
            LOG_HEX_SERVO("Received raw chunk", read_len, buf + recv_len, read_len);
            recv_len += read_len;

            /* 动态检查帧完整性 */
            if (recv_len >= 4) {  // 至少收到帧头+ID+Length
                /* 检查帧头 */
                if (buf[0] != 0x55 || buf[1] != 0x55) {
                    LOG_E("Invalid frame header");
                    LOG_HEX_SERVO("Corrupted frame", recv_len, buf, recv_len);
                    return -RT_ERROR;
                }

                /* 从Length字段获取预期长度 */
                expected_len = buf[3] + 4;  // Length字段值+4(帧头+ID+Length+Cmd)

                /* 检查缓冲区是否足够 */
                if (expected_len > buf_size) {
                    LOG_E("Frame too long (%d > %d)", expected_len, buf_size);
                    return -RT_ERROR;
                }

                /* 检查是否收到完整帧 */
                if (recv_len >= expected_len) {
                    /* 校验和验证 */
                    rt_uint8_t checksum = calculate_checksum(&buf[2], expected_len - 3);
                    if (buf[expected_len - 1] != checksum) {
                        LOG_E("Checksum mismatch");
                        return -RT_ERROR;
                    }

                    LOG_HEX_SERVO("Valid frame", expected_len, buf, expected_len);
                    return expected_len;
                }
            }
        }

        /* 超时检查 */
        if (rt_tick_get() - start_tick > rt_tick_from_millisecond(timeout)) {
//            LOG_W("Timeout (received %d/%d bytes)", recv_len, expected_len);
            if (recv_len > 0) {
                LOG_HEX_SERVO("Partial frame", recv_len, buf, recv_len);
            }
            return -RT_ETIMEOUT;
        }

        rt_thread_mdelay(1);
    }
}

/* 初始化舵机总线 */
int bus_servo_init(const char *uart_name)
{
    rt_err_t result;

    if (servo_uart != RT_NULL) {
        LOG_W("Servo UART already initialized");
        return RT_EOK;
    }

    /* 查找串口设备 */
    servo_uart = rt_device_find(uart_name);
    if (servo_uart == RT_NULL) {
        LOG_E("Can't find UART device %s", uart_name);
        return -RT_ERROR;
    }

    /* 初始化互斥锁 */
    result = rt_mutex_init(&servo_mutex, "servo_mutex", RT_IPC_FLAG_FIFO);
    if (result != RT_EOK) {
        LOG_E("Failed to init mutex: %d", result);
        return result;
    }

    /* 打开串口设备 */
    result = rt_device_open(servo_uart, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX);
    if (result != RT_EOK) {
        LOG_E("Failed to open UART device: %d", result);
        return result;
    }

    LOG_I("Servo UART initialized successfully");
    return RT_EOK;
}

/* 反初始化舵机总线 */
int bus_servo_deinit(void)
{
    if (servo_uart == RT_NULL) {
        return RT_EOK;
    }

    rt_device_close(servo_uart);
    rt_mutex_detach(&servo_mutex);
    servo_uart = RT_NULL;

    LOG_I("Servo UART deinitialized");
    return RT_EOK;
}

/* 控制舵机转动到指定角度 */
int bus_servo_move(rt_uint8_t id, rt_uint16_t angle, rt_uint16_t time)
{
    rt_uint8_t params[4];

    /* 限制角度范围 */
    if (angle > SERVO_MAX_ANGLE) {
        angle = SERVO_MAX_ANGLE;
    }

    params[0] = angle & 0xFF;         // 角度低8位
    params[1] = (angle >> 8) & 0xFF;  // 角度高8位
    params[2] = time & 0xFF;          // 时间低8位
    params[3] = (time >> 8) & 0xFF;   // 时间高8位

    return send_command(id, SERVO_CMD_MOVE_TIME_WRITE, params, sizeof(params));
}

/* 预设舵机转动角度(需要START指令触发) */
int bus_servo_move_wait(rt_uint8_t id, rt_uint16_t angle, rt_uint16_t time)
{
    rt_uint8_t params[4];

    /* 限制角度范围 */
    if (angle > SERVO_MAX_ANGLE) {
        angle = SERVO_MAX_ANGLE;
    }

    params[0] = angle & 0xFF;         // 角度低8位
    params[1] = (angle >> 8) & 0xFF;  // 角度高8位
    params[2] = time & 0xFF;          // 时间低8位
    params[3] = (time >> 8) & 0xFF;   // 时间高8位

    return send_command(id, SERVO_CMD_MOVE_TIME_WAIT_WRITE, params, sizeof(params));
}

/* 触发预设的舵机动作 */
int bus_servo_move_start(rt_uint8_t id)
{
    return send_command(id, SERVO_CMD_MOVE_START, RT_NULL, 0);
}

/* 停止舵机转动 */
int bus_servo_move_stop(rt_uint8_t id)
{
    return send_command(id, SERVO_CMD_MOVE_STOP, RT_NULL, 0);
}

/* 设置舵机ID */
int bus_servo_set_id(rt_uint8_t old_id, rt_uint8_t new_id)
{
    rt_uint8_t params[1] = {new_id};
    return send_command(old_id, SERVO_CMD_ID_WRITE, params, sizeof(params));
}

/* 获取舵机ID */
int bus_servo_get_id(rt_uint8_t *id)
{
    rt_uint8_t buf[16];
    int ret;

    /* 发送读取指令 */
    ret = send_command(SERVO_BROADCAST_ID, SERVO_CMD_ID_READ, RT_NULL, 0);
    if (ret != RT_EOK) {
        return ret;
    }

    /* 接收响应 */
    ret = receive_response(buf, sizeof(buf), SERVO_RESPONSE_TIMEOUT);

    *id = buf[5];  // 参数1为ID

    return RT_EOK;
}

/* 设置舵机角度偏移(临时) */
int bus_servo_set_angle_offset(rt_uint8_t id, rt_int8_t offset)
{
    rt_uint8_t params[1] = {(rt_uint8_t)offset};
    return send_command(id, SERVO_CMD_ANGLE_OFFSET_ADJUST, params, sizeof(params));
}

/* 保存舵机角度偏移 */
int bus_servo_save_angle_offset(rt_uint8_t id)
{
    return send_command(id, SERVO_CMD_ANGLE_OFFSET_WRITE, RT_NULL, 0);
}

/* 获取舵机角度偏移 */
int bus_servo_get_angle_offset(rt_uint8_t id, rt_int8_t *offset)
{
    rt_uint8_t buf[16];
    int ret;

    ret = send_command(id, SERVO_CMD_ANGLE_OFFSET_READ, RT_NULL, 0);
    if (ret != RT_EOK) {
        return ret;
    }

    ret = receive_response(buf, sizeof(buf), SERVO_RESPONSE_TIMEOUT);

    *offset = (rt_int8_t)buf[5];

    return RT_EOK;
}

/* 设置舵机角度限制 */
int bus_servo_set_angle_limit(rt_uint8_t id, rt_uint16_t min, rt_uint16_t max)
{
    rt_uint8_t params[4];

    if (min > max) {
        LOG_E("Invalid angle limit: min(%d) > max(%d)", min, max);
        return -RT_EINVAL;
    }

    params[0] = min & 0xFF;
    params[1] = (min >> 8) & 0xFF;
    params[2] = max & 0xFF;
    params[3] = (max >> 8) & 0xFF;

    return send_command(id, SERVO_CMD_ANGLE_LIMIT_WRITE, params, sizeof(params));
}

/* 获取舵机角度限制 */
int bus_servo_get_angle_limit(rt_uint8_t id, rt_uint16_t *min, rt_uint16_t *max)
{
    rt_uint8_t buf[16];
    int ret;

    ret = send_command(id, SERVO_CMD_ANGLE_LIMIT_READ, RT_NULL, 0);
    if (ret != RT_EOK) {
        return ret;
    }

    ret = receive_response(buf, sizeof(buf), SERVO_RESPONSE_TIMEOUT);

    *min = buf[5] | (buf[6] << 8);
    *max = buf[7] | (buf[8] << 8);

    return RT_EOK;
}

/* 设置舵机电压限制 */
int bus_servo_set_vin_limit(rt_uint8_t id, rt_uint16_t min, rt_uint16_t max)
{
    rt_uint8_t params[4];

    if (min > max) {
        LOG_E("Invalid voltage limit: min(%d) > max(%d)", min, max);
        return -RT_EINVAL;
    }

    params[0] = min & 0xFF;
    params[1] = (min >> 8) & 0xFF;
    params[2] = max & 0xFF;
    params[3] = (max >> 8) & 0xFF;

    return send_command(id, SERVO_CMD_VIN_LIMIT_WRITE, params, sizeof(params));
}

/* 获取舵机电压限制 */
int bus_servo_get_vin_limit(rt_uint8_t id, rt_uint16_t *min, rt_uint16_t *max)
{
    rt_uint8_t buf[16];
    int ret;

    ret = send_command(id, SERVO_CMD_VIN_LIMIT_READ, RT_NULL, 0);
    if (ret != RT_EOK) {
        return ret;
    }

    ret = receive_response(buf, sizeof(buf), SERVO_RESPONSE_TIMEOUT);

    *min = buf[5] | (buf[6] << 8);
    *max = buf[7] | (buf[8] << 8);

    return RT_EOK;
}

/* 设置舵机最高温度限制 */
int bus_servo_set_temp_limit(rt_uint8_t id, rt_uint8_t temp)
{
    rt_uint8_t params[1] = {temp};
    return send_command(id, SERVO_CMD_TEMP_MAX_LIMIT_WRITE, params, sizeof(params));
}

/* 获取舵机最高温度限制 */
int bus_servo_get_temp_limit(rt_uint8_t id, rt_uint8_t *temp)
{
    rt_uint8_t buf[16];
    int ret;

    ret = send_command(id, SERVO_CMD_TEMP_MAX_LIMIT_READ, RT_NULL, 0);
    if (ret != RT_EOK) {
        return ret;
    }

    ret = receive_response(buf, sizeof(buf), SERVO_RESPONSE_TIMEOUT);

    *temp = buf[5];

    return RT_EOK;
}

/* 获取舵机当前温度 */
int bus_servo_get_temp(rt_uint8_t id, rt_uint8_t *temp)
{
    rt_uint8_t buf[16];
    int ret;

    ret = send_command(id, SERVO_CMD_TEMP_READ, RT_NULL, 0);
    if (ret != RT_EOK) {
        return ret;
    }

    ret = receive_response(buf, sizeof(buf), SERVO_RESPONSE_TIMEOUT);

    if (buf[4] != SERVO_CMD_TEMP_READ) {
        LOG_E("Unexpected response command: 0x%02X", buf[4]);
        return -RT_ERROR;
    }

    *temp = buf[5];

    return RT_EOK;
}

/* 获取舵机当前电压 */
int bus_servo_get_vin(rt_uint8_t id, rt_uint16_t *vin)
{
    rt_uint8_t buf[16];
    int ret;

    ret = send_command(id, SERVO_CMD_VIN_READ, RT_NULL, 0);
    if (ret != RT_EOK) {
        return ret;
    }

    ret = receive_response(buf, sizeof(buf), SERVO_RESPONSE_TIMEOUT);

    *vin = buf[5] | (buf[6] << 8);

    return RT_EOK;
}

/* 获取舵机当前位置 */
int bus_servo_get_pos(rt_uint8_t id, rt_int16_t *pos)
{
    rt_uint8_t buf[16];
    int ret;

    ret = send_command(id, SERVO_CMD_POS_READ, RT_NULL, 0);
    if (ret != RT_EOK) {
        return ret;
    }

    ret = receive_response(buf, sizeof(buf), SERVO_RESPONSE_TIMEOUT);

    *pos = (rt_int16_t)(buf[5] | (buf[6] << 8));

    positionToAngle((int)pos);

    return RT_EOK;
}

/* 设置舵机模式 */
int bus_servo_set_mode(rt_uint8_t id, rt_uint8_t mode, rt_uint8_t motor_mode, rt_int16_t speed)
{
    rt_uint8_t params[4];

    params[0] = mode;
    params[1] = motor_mode;
    params[2] = speed & 0xFF;
    params[3] = (speed >> 8) & 0xFF;

    return send_command(id, SERVO_CMD_OR_MOTOR_MODE_WRITE, params, sizeof(params));
}

/* 获取舵机模式 */
int bus_servo_get_mode(rt_uint8_t id, rt_uint8_t *mode, rt_uint8_t *motor_mode, rt_int16_t *speed)
{
    rt_uint8_t buf[16];
    int ret;

    ret = send_command(id, SERVO_CMD_OR_MOTOR_MODE_READ, RT_NULL, 0);
    if (ret != RT_EOK) {
        return ret;
    }

    ret = receive_response(buf, sizeof(buf), SERVO_RESPONSE_TIMEOUT);

    *mode = buf[5];
    *motor_mode = buf[6];
    *speed = (rt_int16_t)(buf[7] | (buf[8] << 8));

    return RT_EOK;
}

/* 设置舵机负载状态 */
int bus_servo_set_load(rt_uint8_t id, rt_uint8_t state)
{
    rt_uint8_t params[1] = {state};
    return send_command(id, SERVO_CMD_LOAD_OR_UNLOAD_WRITE, params, sizeof(params));
}

/* 获取舵机负载状态 */
int bus_servo_get_load(rt_uint8_t id, rt_uint8_t *state)
{
    rt_uint8_t buf[16];
    int ret;

    ret = send_command(id, SERVO_CMD_LOAD_OR_UNLOAD_READ, RT_NULL, 0);
    if (ret != RT_EOK) {
        return ret;
    }

    ret = receive_response(buf, sizeof(buf), SERVO_RESPONSE_TIMEOUT);

    *state = buf[5];

    return RT_EOK;
}

/* 设置舵机LED状态 */
int bus_servo_set_led(rt_uint8_t id, rt_uint8_t state)
{
    rt_uint8_t params[1] = {state};
    return send_command(id, SERVO_CMD_LED_CTRL_WRITE, params, sizeof(params));
}

/* 获取舵机LED状态 */
int bus_servo_get_led(rt_uint8_t id, rt_uint8_t *state)
{
    rt_uint8_t buf[16];
    int ret;

    ret = send_command(id, SERVO_CMD_LED_CTRL_READ, RT_NULL, 0);
    if (ret != RT_EOK) {
        return ret;
    }

    ret = receive_response(buf, sizeof(buf), SERVO_RESPONSE_TIMEOUT);

    *state = buf[5];

    return RT_EOK;
}

/* 设置舵机错误报警掩码 */
int bus_servo_set_error_mask(rt_uint8_t id, rt_uint8_t mask)
{
    rt_uint8_t params[1] = {mask};
    return send_command(id, SERVO_CMD_LED_ERROR_WRITE, params, sizeof(params));
}

/* 获取舵机错误报警掩码 */
int bus_servo_get_error_mask(rt_uint8_t id, rt_uint8_t *mask)
{
    rt_uint8_t buf[16];
    int ret;

    ret = send_command(id, SERVO_CMD_LED_ERROR_READ, RT_NULL, 0);
    if (ret != RT_EOK) {
        return ret;
    }

    ret = receive_response(buf, sizeof(buf), SERVO_RESPONSE_TIMEOUT);

    *mask = buf[5];

    return RT_EOK;
}

/* 获取舵机转动距离 */
int bus_servo_get_distance(rt_uint8_t id, rt_uint32_t *distance)
{
    rt_uint8_t buf[16];
    int ret;

    ret = send_command(id, SERVO_CMD_DIS_READ, RT_NULL, 0);
    if (ret != RT_EOK) {
        return ret;
    }

    ret = receive_response(buf, sizeof(buf), SERVO_RESPONSE_TIMEOUT);

    *distance = buf[5] | (buf[6] << 8) | (buf[7] << 16) | (buf[8] << 24);

    return RT_EOK;
}

/**
 * @brief 将位置值转换为角度
 * @param position 输入位置值，范围0~1000
 * @return 对应的角度值，范围0~240度
 */
float positionToAngle(int position)
{
    if (position < 0) {
        position = 0;
    } else if (position > 1000) {
        position = 1000;
    }

    // 线性转换公式：角度 = (位置值 / 最大值) * 角度范围
    float angle = ((float)position / 1000.0f) * 240.0f;

    return angle;
}
