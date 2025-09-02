//
// Created by lucas on 2022-08-29.
//
#ifndef __SERIAL_SERVO_H
#define __SERIAL_SERVO_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>


#define SERIAL_SERVO_FRAME_HEADER         0x55
#define SERIAL_SERVO_MOVE_TIME_WRITE      1
#define SERIAL_SERVO_MOVE_TIME_READ       2
#define SERIAL_SERVO_MOVE_TIME_WAIT_WRITE 7
#define SERIAL_SERVO_MOVE_TIME_WAIT_READ  8
#define SERIAL_SERVO_MOVE_START           11
#define SERIAL_SERVO_MOVE_STOP            12
#define SERIAL_SERVO_ID_WRITE             13
#define SERIAL_SERVO_ID_READ              14
#define SERIAL_SERVO_ANGLE_OFFSET_ADJUST  17
#define SERIAL_SERVO_ANGLE_OFFSET_WRITE   18
#define SERIAL_SERVO_ANGLE_OFFSET_READ    19
#define SERIAL_SERVO_ANGLE_LIMIT_WRITE    20
#define SERIAL_SERVO_ANGLE_LIMIT_READ     21
#define SERIAL_SERVO_VIN_LIMIT_WRITE      22
#define SERIAL_SERVO_VIN_LIMIT_READ       23
#define SERIAL_SERVO_TEMP_MAX_LIMIT_WRITE 24
#define SERIAL_SERVO_TEMP_MAX_LIMIT_READ  25
#define SERIAL_SERVO_TEMP_READ            26
#define SERIAL_SERVO_VIN_READ             27
#define SERIAL_SERVO_POS_READ             28
#define SERIAL_SERVO_OR_MOTOR_MODE_WRITE  29
#define SERIAL_SERVO_OR_MOTOR_MODE_READ   30
#define SERIAL_SERVO_LOAD_OR_UNLOAD_WRITE 31
#define SERIAL_SERVO_LOAD_OR_UNLOAD_READ  32
#define SERIAL_SERVO_LED_CTRL_WRITE       33
#define SERIAL_SERVO_LED_CTRL_READ        34
#define SERIAL_SERVO_LED_ERROR_WRITE      35
#define SERIAL_SERVO_LED_ERROR_READ       36

#define CMD_SERVO_MOVE 0x03

#define SERIAL_DEBUG 1


#pragma pack(1)
typedef struct {
    uint8_t header_1;
    uint8_t header_2;
    union {
        struct {
            uint8_t servo_id;
            uint8_t length;
            uint8_t command;
            uint8_t args[8];
        } elements;
        uint8_t data_raw[11];
    };
} SerialServoCmdTypeDef;
#pragma pack()


typedef enum {
    SERIAL_SERVO_RECV_STARTBYTE_1 = 0,
    SERIAL_SERVO_RECV_STARTBYTE_2,
    SERIAL_SERVO_RECV_SERVO_ID,
    SERIAL_SERVO_RECV_LENGTH,
    SERIAL_SERVO_RECV_COMMAND,
    SERIAL_SERVO_RECV_ARGUMENTS,
    SERIAL_SERVO_RECV_CHECKSUM,
} SerialServoRecvState;

typedef struct SerialServoControllerTypeDef SerialServoControllerTypeDef;
struct SerialServoControllerTypeDef {
    SerialServoRecvState rx_state;
    SerialServoCmdTypeDef rx_frame;
    uint32_t rx_args_index;

    SerialServoCmdTypeDef tx_frame;
    uint32_t tx_byte_index;
    bool tx_only;

    uint32_t proc_timeout;
    int (*serial_write_and_read)(SerialServoControllerTypeDef *self, SerialServoCmdTypeDef *frame, bool tx_only);
};

void serial_servo_controller_object_init(SerialServoControllerTypeDef *self);
void serial_servo_set_id(SerialServoControllerTypeDef *self, uint32_t old_id, uint32_t new_id);
int serial_servo_read_id(SerialServoControllerTypeDef *self, uint32_t servo_id, uint8_t *ret_servo_id);
void serial_servo_set_position(SerialServoControllerTypeDef *self, uint32_t servo_id, int position, uint32_t duration);
int serial_servo_read_position(SerialServoControllerTypeDef *self, uint32_t servo_id, int16_t *position);
void serial_servo_stop(SerialServoControllerTypeDef *self, uint32_t servo_id);
void serial_servo_set_deviation(SerialServoControllerTypeDef *self, uint32_t servo_id, int new_deviation);
int serial_servo_read_deviation(SerialServoControllerTypeDef *self, uint32_t servo_id, int8_t *deviation);
void serial_servo_save_deviation(SerialServoControllerTypeDef *self, uint32_t servo_id);
void serial_servo_load_unload(SerialServoControllerTypeDef *self, uint32_t servo_id, uint32_t load);
void serial_servo_set_angle_limit(SerialServoControllerTypeDef *self, uint32_t servo_id, uint32_t limit_l, uint32_t limit_h);
int serial_servo_read_angle_limit(SerialServoControllerTypeDef *self, uint32_t servo_id, uint16_t limit[2]);
void serial_servo_set_temp_limit(SerialServoControllerTypeDef *self, uint32_t servo_id, uint32_t limit);
int serial_servo_read_temp_limit(SerialServoControllerTypeDef *self, uint32_t servo_id, uint8_t *limit);
int serial_servo_read_temp(SerialServoControllerTypeDef *self, uint32_t servo_id, uint8_t *temp);
void serial_servo_set_vin_limit(SerialServoControllerTypeDef *self, uint32_t servo_id, uint32_t limit_l, uint32_t limit_h);
int serial_servo_read_vin_limit(SerialServoControllerTypeDef *self, uint32_t servo_id, uint16_t limit[2]);
int serial_servo_read_vin(SerialServoControllerTypeDef *self, uint32_t servo_id, uint16_t *vin);
int serial_servo_read_load_unload(SerialServoControllerTypeDef *self, uint32_t servo_id, uint8_t* load_unload);

static inline uint8_t serial_servo_checksum(const uint8_t buf[])
{
    uint16_t temp = 0;
    for (int i = 2; i < buf[3] + 2; ++i) {
        temp += buf[i];
    }
    return (uint8_t)(~temp);
}

static inline int serial_servo_rx_handler(SerialServoControllerTypeDef *self, uint8_t rx_byte)
{
    switch (self->rx_state) {
        case SERIAL_SERVO_RECV_STARTBYTE_1: {
            self->rx_state = SERIAL_SERVO_FRAME_HEADER == rx_byte ? SERIAL_SERVO_RECV_STARTBYTE_2 : SERIAL_SERVO_RECV_STARTBYTE_1;
            self->rx_frame.header_1 = SERIAL_SERVO_FRAME_HEADER;
            return -1;
        }
        case SERIAL_SERVO_RECV_STARTBYTE_2: {
            self->rx_state = 0x55 == rx_byte ? SERIAL_SERVO_RECV_SERVO_ID : SERIAL_SERVO_RECV_STARTBYTE_1;
            self->rx_frame.header_2 = SERIAL_SERVO_FRAME_HEADER;
            return -2;
        }
        case SERIAL_SERVO_RECV_SERVO_ID: {
            self->rx_frame.elements.servo_id = rx_byte;
			self->rx_state = SERIAL_SERVO_RECV_LENGTH;
            return 1;
        }
        case SERIAL_SERVO_RECV_LENGTH: {
            if(rx_byte > 7) {
                self->rx_state = SERIAL_SERVO_RECV_STARTBYTE_1; /* 包长度超过允许长度 */
                return -3;
            }
            self->rx_frame.elements.length = rx_byte;
            self->rx_state = SERIAL_SERVO_RECV_COMMAND;
            return 2;
        }
        case SERIAL_SERVO_RECV_COMMAND: {
            self->rx_frame.elements.command = rx_byte;
            self->rx_args_index = 0;
            self->rx_state = self->rx_frame.elements.length == 6 ? SERIAL_SERVO_RECV_CHECKSUM : SERIAL_SERVO_RECV_ARGUMENTS; /* 没有参数的话直接进入校验字段 */
            return 3;
        }
        case SERIAL_SERVO_RECV_ARGUMENTS: {
            self->rx_frame.elements.args[self->rx_args_index++] = rx_byte;
            if (self->rx_args_index + 3 == self->rx_frame.elements.length) {
                self->rx_state = SERIAL_SERVO_RECV_CHECKSUM;
            }
            return 4;
        }
        case SERIAL_SERVO_RECV_CHECKSUM: {
            if(serial_servo_checksum((uint8_t*)&self->rx_frame) != rx_byte) {
                self->rx_state = SERIAL_SERVO_RECV_STARTBYTE_1;
                return -99;
            } else {
                self->rx_state = SERIAL_SERVO_RECV_STARTBYTE_1;
                return 0;
            }
        }

        default: {
            self->rx_state = SERIAL_SERVO_RECV_STARTBYTE_1;
            return -100;
        }
    }
}

#endif
