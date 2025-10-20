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
#include <finsh.h>

static void servo_help(void)
{
    rt_kprintf("Usage: servo <command> [args...]\n");
    rt_kprintf("Commands:\n");
    rt_kprintf("  init <uart>          - Initialize servo bus with UART device name\n");
    rt_kprintf("  deinit               - Deinitialize servo bus\n");
    rt_kprintf("  move <id> <angle> <time> - Move servo to angle (0-1000) in time (ms)\n");
    rt_kprintf("  move_wait <id> <angle> <time> - Preset move (need start command)\n");
    rt_kprintf("  move_start <id>      - Start preset move\n");
    rt_kprintf("  move_stop <id>       - Stop servo movement\n");
    rt_kprintf("  set_id <old> <new>   - Change servo ID\n");
    rt_kprintf("  get_id               - Get servo ID (broadcast)\n");
    rt_kprintf("  set_offset <id> <offset> - Set angle offset (-125~125)\n");
    rt_kprintf("  save_offset <id>     - Save angle offset\n");
    rt_kprintf("  get_offset <id>      - Get angle offset\n");
    rt_kprintf("  set_angle_limit <id> <min> <max> - Set angle limits (0-1000)\n");
    rt_kprintf("  get_angle_limit <id> - Get angle limits\n");
    rt_kprintf("  set_vin_limit <id> <min> <max> - Set voltage limits (4500-14000 mV)\n");
    rt_kprintf("  get_vin_limit <id>   - Get voltage limits\n");
    rt_kprintf("  set_temp_limit <id> <temp> - Set max temperature (50-100 C)\n");
    rt_kprintf("  get_temp_limit <id>  - Get max temperature\n");
    rt_kprintf("  get_temp <id>        - Get current temperature\n");
    rt_kprintf("  get_vin <id>         - Get current voltage\n");
    rt_kprintf("  get_pos <id>         - Get current position\n");
    rt_kprintf("  set_mode <id> <mode> <motor_mode> <speed> - Set servo mode\n");
    rt_kprintf("    mode: 0-position, 1-motor\n");
    rt_kprintf("    motor_mode: 0-duty, 1-speed\n");
    rt_kprintf("    speed: -1000~1000 (duty) or -50~50 (speed)\n");
    rt_kprintf("  get_mode <id>        - Get servo mode\n");
    rt_kprintf("  set_load <id> <state> - Set load state (0-unload, 1-load)\n");
    rt_kprintf("  get_load <id>        - Get load state\n");
    rt_kprintf("  set_led <id> <state> - Set LED state (0-on, 1-off)\n");
    rt_kprintf("  get_led <id>         - Get LED state\n");
    rt_kprintf("  set_error_mask <id> <mask> - Set error mask (0-7)\n");
    rt_kprintf("  get_error_mask <id>   - Get error mask\n");
    rt_kprintf("  get_distance <id>     - Get rotation distance\n");
}

int bus_servo_cmd(int argc, char **argv)
{
    if (argc < 2) {
        servo_help();
        return -1;
    }

    if (rt_strcmp(argv[1], "init") == 0) {
        if (argc != 3) {
            rt_kprintf("Usage: servo init <uart>\n");
            return -1;
        }
        return bus_servo_init(argv[2]);
    }
    else if (rt_strcmp(argv[1], "deinit") == 0) {
        return bus_servo_deinit();
    }
    else if (rt_strcmp(argv[1], "move") == 0) {
        if (argc != 5) {
            rt_kprintf("Usage: servo move <id> <angle> <time>\n");
            return -1;
        }
        else if((atoi(argv[3]) > 638) || (atoi(argv[3]) < 365))
        {
        	rt_kprintf("Note: The control range of the steering gear is 365 to 638 (corresponding to 87.6° to 153.12°)\n");
        	rt_kprintf("out of the angle control range of the vehicle, please try again...\n");
        	return -1;
        }
        return bus_servo_move(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
    }
    else if (rt_strcmp(argv[1], "move_wait") == 0) {
        if (argc != 5) {
            rt_kprintf("Usage: servo move_wait <id> <angle> <time>\n");
            return -1;
        }
        return bus_servo_move_wait(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
    }
    else if (rt_strcmp(argv[1], "move_start") == 0) {
        if (argc != 3) {
            rt_kprintf("Usage: servo move_start <id>\n");
            return -1;
        }
        return bus_servo_move_start(atoi(argv[2]));
    }
    else if (rt_strcmp(argv[1], "move_stop") == 0) {
        if (argc != 3) {
            rt_kprintf("Usage: servo move_stop <id>\n");
            return -1;
        }
        return bus_servo_move_stop(atoi(argv[2]));
    }
    else if (rt_strcmp(argv[1], "set_id") == 0) {
        if (argc != 4) {
            rt_kprintf("Usage: servo set_id <old> <new>\n");
            return -1;
        }
        return bus_servo_set_id(atoi(argv[2]), atoi(argv[3]));
    }
    else if (rt_strcmp(argv[1], "get_id") == 0) {
        rt_uint8_t id;
        int ret = bus_servo_get_id(&id);
        if (ret == RT_EOK) {
            rt_kprintf("Servo ID: %d\n", id);
        }
        return ret;
    }
    else if (rt_strcmp(argv[1], "set_offset") == 0) {
        if (argc != 4) {
            rt_kprintf("Usage: servo set_offset <id> <offset>\n");
            return -1;
        }
        return bus_servo_set_angle_offset(atoi(argv[2]), atoi(argv[3]));
    }
    else if (rt_strcmp(argv[1], "save_offset") == 0) {
        if (argc != 3) {
            rt_kprintf("Usage: servo save_offset <id>\n");
            return -1;
        }
        return bus_servo_save_angle_offset(atoi(argv[2]));
    }
    else if (rt_strcmp(argv[1], "get_offset") == 0) {
        if (argc != 3) {
            rt_kprintf("Usage: servo get_offset <id>\n");
            return -1;
        }
        rt_int8_t offset;
        int ret = bus_servo_get_angle_offset(atoi(argv[2]), &offset);
        if (ret == RT_EOK) {
            rt_kprintf("Angle offset: %d\n", offset);
        }
        return ret;
    }
    else if (rt_strcmp(argv[1], "set_angle_limit") == 0) {
        if (argc != 5) {
            rt_kprintf("Usage: servo set_angle_limit <id> <min> <max>\n");
            return -1;
        }
        return bus_servo_set_angle_limit(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
    }
    else if (rt_strcmp(argv[1], "get_angle_limit") == 0) {
        if (argc != 3) {
            rt_kprintf("Usage: servo get_angle_limit <id>\n");
            return -1;
        }
        rt_uint16_t min, max;
        int ret = bus_servo_get_angle_limit(atoi(argv[2]), &min, &max);
        if (ret == RT_EOK) {
            rt_kprintf("Angle limit: min=%d, max=%d\n", min, max);
        }
        return ret;
    }
    else if (rt_strcmp(argv[1], "set_vin_limit") == 0) {
        if (argc != 5) {
            rt_kprintf("Usage: servo set_vin_limit <id> <min> <max>\n");
            return -1;
        }
        return bus_servo_set_vin_limit(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
    }
    else if (rt_strcmp(argv[1], "get_vin_limit") == 0) {
        if (argc != 3) {
            rt_kprintf("Usage: servo get_vin_limit <id>\n");
            return -1;
        }
        rt_uint16_t min, max;
        int ret = bus_servo_get_vin_limit(atoi(argv[2]), &min, &max);
        if (ret == RT_EOK) {
            rt_kprintf("Voltage limit: min=%d mV, max=%d mV\n", min, max);
        }
        return ret;
    }
    else if (rt_strcmp(argv[1], "set_temp_limit") == 0) {
        if (argc != 4) {
            rt_kprintf("Usage: servo set_temp_limit <id> <temp>\n");
            return -1;
        }
        return bus_servo_set_temp_limit(atoi(argv[2]), atoi(argv[3]));
    }
    else if (rt_strcmp(argv[1], "get_temp_limit") == 0) {
        if (argc != 3) {
            rt_kprintf("Usage: servo get_temp_limit <id>\n");
            return -1;
        }
        rt_uint8_t temp;
        int ret = bus_servo_get_temp_limit(atoi(argv[2]), &temp);
        if (ret == RT_EOK) {
            rt_kprintf("Max temperature: %d C\n", temp);
        }
        return ret;
    }
    else if (rt_strcmp(argv[1], "get_temp") == 0) {
        if (argc != 3) {
            rt_kprintf("Usage: servo get_temp <id>\n");
            return -1;
        }
        rt_uint8_t temp;
        int ret = bus_servo_get_temp(atoi(argv[2]), &temp);
        if (ret == RT_EOK) {
            rt_kprintf("Current temperature: %d °C\n", temp);
        }
        return ret;
    }
    else if (rt_strcmp(argv[1], "get_vin") == 0) {
        if (argc != 3) {
            rt_kprintf("Usage: servo get_vin <id>\n");
            return -1;
        }
        rt_uint16_t vin;
        int ret = bus_servo_get_vin(atoi(argv[2]), &vin);
        if (ret == RT_EOK) {
            rt_kprintf("Current voltage: %d mV\n", vin);
        }
        return ret;
    }
    else if (rt_strcmp(argv[1], "get_pos") == 0) {
        if (argc != 3) {
            rt_kprintf("Usage: servo get_pos <id>\n");
            return -1;
        }
        rt_int16_t pos;
        int ret = bus_servo_get_pos(atoi(argv[2]), &pos);
        if (ret == RT_EOK) {
            rt_kprintf("Current position: %d\n", pos);
        }
        return ret;
    }
    else if (rt_strcmp(argv[1], "set_mode") == 0) {
        if (argc != 6) {
            rt_kprintf("Usage: servo set_mode <id> <mode> <motor_mode> <speed>\n");
            return -1;
        }
        return bus_servo_set_mode(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), atoi(argv[5]));
    }
    else if (rt_strcmp(argv[1], "get_mode") == 0) {
        if (argc != 3) {
            rt_kprintf("Usage: servo get_mode <id>\n");
            return -1;
        }
        rt_uint8_t mode, motor_mode;
        rt_int16_t speed;
        int ret = bus_servo_get_mode(atoi(argv[2]), &mode, &motor_mode, &speed);
        if (ret == RT_EOK) {
            rt_kprintf("Mode: %s\n", mode == SERVO_MODE_POSITION ? "Position" : "Motor");
            rt_kprintf("Motor mode: %s\n", motor_mode == MOTOR_MODE_DUTY ? "Duty" : "Speed");
            rt_kprintf("Speed: %d\n", speed);
        }
        return ret;
    }
    else if (rt_strcmp(argv[1], "set_load") == 0) {
        if (argc != 4) {
            rt_kprintf("Usage: servo set_load <id> <state>\n");
            return -1;
        }
        return bus_servo_set_load(atoi(argv[2]), atoi(argv[3]));
    }
    else if (rt_strcmp(argv[1], "get_load") == 0) {
        if (argc != 3) {
            rt_kprintf("Usage: servo get_load <id>\n");
            return -1;
        }
        rt_uint8_t state;
        int ret = bus_servo_get_load(atoi(argv[2]), &state);
        if (ret == RT_EOK) {
            rt_kprintf("Load state: %s\n", state == SERVO_LOAD ? "Loaded" : "Unloaded");
        }
        return ret;
    }
    else if (rt_strcmp(argv[1], "set_led") == 0) {
        if (argc != 4) {
            rt_kprintf("Usage: servo set_led <id> <state>\n");
            return -1;
        }
        return bus_servo_set_led(atoi(argv[2]), atoi(argv[3]));
    }
    else if (rt_strcmp(argv[1], "get_led") == 0) {
        if (argc != 3) {
            rt_kprintf("Usage: servo get_led <id>\n");
            return -1;
        }
        rt_uint8_t state;
        int ret = bus_servo_get_led(atoi(argv[2]), &state);
        if (ret == RT_EOK) {
            rt_kprintf("LED state: %s\n", state == LED_ON ? "On" : "Off");
        }
        return ret;
    }
    else if (rt_strcmp(argv[1], "set_error_mask") == 0) {
        if (argc != 4) {
            rt_kprintf("Usage: servo set_error_mask <id> <mask>\n");
            return -1;
        }
        return bus_servo_set_error_mask(atoi(argv[2]), atoi(argv[3]));
    }
    else if (rt_strcmp(argv[1], "get_error_mask") == 0) {
        if (argc != 3) {
            rt_kprintf("Usage: servo get_error_mask <id>\n");
            return -1;
        }
        rt_uint8_t mask;
        int ret = bus_servo_get_error_mask(atoi(argv[2]), &mask);
        if (ret == RT_EOK) {
            rt_kprintf("Error mask: 0x%02X\n", mask);
            if (mask & ERROR_OVER_TEMP) rt_kprintf("  Over temperature\n");
            if (mask & ERROR_OVER_VOLTAGE) rt_kprintf("  Over voltage\n");
            if (mask & ERROR_STALL) rt_kprintf("  Stall\n");
        }
        return ret;
    }
    else if (rt_strcmp(argv[1], "get_distance") == 0) {
        if (argc != 3) {
            rt_kprintf("Usage: servo get_distance <id>\n");
            return -1;
        }
        rt_uint32_t distance;
        int ret = bus_servo_get_distance(atoi(argv[2]), &distance);
        if (ret == RT_EOK) {
            rt_kprintf("Rotation distance: %d pulses\n", distance);
        }
        return ret;
    }
    else {
        servo_help();
        return -1;
    }

    return 0;
}

/* 导出到MSH命令 */
MSH_CMD_EXPORT(bus_servo_cmd, bus servo control command);
