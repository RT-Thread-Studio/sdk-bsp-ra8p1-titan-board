#include <string.h>
#include <inttypes.h>
#include "hal_data.h"

#include "hyper_ram_test.h"
#include <rtthread.h>

#define ram_cfg     g_ospi1_cfg
#define ram_ctrl    g_ospi1_ctrl

#define OSPI_OM_RESET               BSP_IO_PORT_12_PIN_07
#define HYPER_RAM_RESET_DELAY()     R_BSP_SoftwareDelay(10UL, BSP_DELAY_UNITS_MICROSECONDS)

#define RAM_WORD_SIZE               (2UL)
#define RAM_WORDS_PER_BLOCK         (8UL)
#define RAM_BLOCK_SIZE              (RAM_WORD_SIZE * RAM_WORDS_PER_BLOCK)
#define RAM_BLOCKS_PER_PAGE         (64UL)
#define RAM_PAGE_SIZE               (RAM_BLOCK_SIZE * RAM_BLOCKS_PER_PAGE)
#define RAM_PAGE_COUNT              (32768UL)
#define RAM_DENSITY                 (RAM_PAGE_SIZE * RAM_PAGE_COUNT)

#define CHUNK_SIZE                  (64UL)

#define HYPER_RAM_CFG_REG_0_ADDRESS (0x01000000)
#define HYPER_RAM_CFG_REG_1_ADDRESS (0x01000001)

#if !defined(LENGTH_OF)
 #define LENGTH_OF(arr)             (sizeof((arr)) / sizeof((arr)[0]))
#endif

#define TRY(fn)                     _TRY_BREAK(fn, false)
#define TRY_BREAK(fn)               _TRY_BREAK(fn, true)
#define _TRY_BREAK(fn,break_on_err)         \
    do                                      \
    {                                       \
        volatile fsp_err_t _err;            \
        if (FSP_SUCCESS != (_err = (fn)))   \
        {                                   \
            if (break_on_err) __BKPT(0); \
            return _err;                    \
        }                                   \
    } while (0)

#define CATCH_BREAK(fn)                     \
    do                                      \
    {                                       \
        volatile fsp_err_t _err;            \
        if (FSP_SUCCESS != (_err = (fn)))   \
        {                                   \
            __BKPT(0);                   \
        }                                   \
    } while (0)

typedef enum
{
    HYPER_RAM_CMD_READ           = 0x80,
    HYPER_RAM_CMD_WRITE          = 0x00,
    HYPER_RAM_CMD_MEMORY_SPACE   = 0x00,
    HYPER_RAM_CMD_REGISTER_SPACE = 0x40,
    HYPER_RAM_CMD_WRAP           = 0x00,
    HYPER_RAM_CMD_LINEAR         = 0x20,
} hyper_ram_cmd_t;

ospi_b_xspi_command_set_t g_hyper_ram_commands[] =
{
 {
  .protocol = SPI_FLASH_PROTOCOL_8D_8D_8D,
  .frame_format = OSPI_B_FRAME_FORMAT_XSPI_PROFILE_2_EXTENDED,
  .latency_mode = OSPI_B_LATENCY_MODE_FIXED,
  .command_bytes = OSPI_B_COMMAND_BYTES_1,
  .address_bytes = SPI_FLASH_ADDRESS_BYTES_4,

  .read_command = 0xA0,
  .read_dummy_cycles = 11,
  .program_command = 0x20,
  .program_dummy_cycles = 11,

  .address_msb_mask = 0xF0,
  .status_needs_address = false,

  .p_erase_commands = NULL,
 }
};

uint16_t swap16(uint16_t value)
{
    uint16_t ret;
    ret  = value << 8;
    ret |= value >> 8;
    return ret;
}

static fsp_err_t hyper_ram_config_get(uint32_t address, uint16_t * const p_value_out)
{
    spi_flash_direct_transfer_t xfer = {
            .address = address,
            .address_length = 4,
            .command_length = 2,
            .command = 0xE000,
            .data_length = 2,
            .dummy_cycles = 11,
    };

    TRY_BREAK(R_OSPI_B_DirectTransfer(&ram_ctrl, &xfer, SPI_FLASH_DIRECT_TRANSFER_DIR_READ));

    *p_value_out = (uint16_t) xfer.data;

    return FSP_SUCCESS;
}

static fsp_err_t hyper_ram_config_set(uint32_t address, uint16_t value)
{
    spi_flash_direct_transfer_t xfer = {
            .address = address,
            .address_length = 4,
            .command = 0x6000,
            .command_length = 2,
            .data = (uint16_t) value,
            .data_length = 2,
            .dummy_cycles = 0,
    };

    TRY_BREAK(R_OSPI_B_DirectTransfer(&ram_ctrl, &xfer, SPI_FLASH_DIRECT_TRANSFER_DIR_WRITE));

    return FSP_SUCCESS;
}

/* Define the static array at address 0x70000000 using section attribute */
//#define TEST_SIZE 8388608 // 32MB (8M x 4 bytes)
//static uint32_t test_array[TEST_SIZE] __attribute__((section(".ospi1_cs0_noinit")));
//
//void hyper_ram_test(void)
//{
////    uint16_t cfg_reg0 = 0;
////    hyper_ram_config_get(HYPER_RAM_CFG_REG_0_ADDRESS, &cfg_reg0);
////    rt_kprintf("Read CR0 value: 0x%x\n", swap16(cfg_reg0));
////
////    // CR0 default: 0x8F2F
////    uint16_t value = 0xbf1d;
////    hyper_ram_config_set(HYPER_RAM_CFG_REG_0_ADDRESS, swap16(value));
////    rt_kprintf("Set CR0 to 0x%x\n", value);
////
////    cfg_reg0 = 0;
////    hyper_ram_config_get(HYPER_RAM_CFG_REG_0_ADDRESS, &cfg_reg0);
////    rt_kprintf("Read CR0 value: 0x%x\n", swap16(cfg_reg0));
//
//    const uint32_t test_bytes = TEST_SIZE * 4; // Total bytes: 32MB
//    uint32_t errors = 0;
//    rt_tick_t start_time, end_time;
//    uint32_t write_speed_kbs, read_speed_kbs;
//    uint32_t write_time_ms, read_time_ms;
//
//    /* Write pattern to RAM and measure time */
//    start_time = rt_tick_get();
//    for (uint32_t i = 0; i < TEST_SIZE; i++)
//    {
//        test_array[i] = i ^ 0xA5A5A5A5; // Use XOR pattern for better error detection
//    }
//    end_time = rt_tick_get();
//
//    /* Calculate write speed in KB/s and time in ms */
//    write_time_ms = (end_time - start_time) * 1000 / RT_TICK_PER_SECOND;
//    if (write_time_ms > 0)
//    {
//        write_speed_kbs = (test_bytes / 1024) / write_time_ms * 1000; // KB/s
//    }
//    else
//    {
//        write_speed_kbs = 0; // Avoid division by zero
//    }
//
//    /* Verify written pattern while measuring read time */
//    start_time = rt_tick_get();
//    for (uint32_t i = 0; i < TEST_SIZE; i++)
//    {
//        uint32_t expected = i ^ 0xA5A5A5A5;
//        uint32_t actual = test_array[i];
//        if (actual != expected)
//        {
//            errors++;
//            rt_kprintf("errors:%d actual:0x%x expected:0x%x\n", errors, actual, expected);
//        }
//    }
//    end_time = rt_tick_get();
//
//    /* Calculate read speed in KB/s and time in ms */
//    read_time_ms = (end_time - start_time) * 1000 / RT_TICK_PER_SECOND;
//    if (read_time_ms > 0)
//    {
//        read_speed_kbs = (test_bytes / 1024) / read_time_ms * 1000; // KB/s
//    }
//    else
//    {
//        read_speed_kbs = 0; // Avoid division by zero
//    }
//
//    /* Print test results */
//    if (errors == 0)
//    {
//        rt_kprintf("Hyper RAM test passed successfully!\n");
//    }
//    else
//    {
//        rt_kprintf("Hyper RAM test failed with %u errors\n", errors);
//    }
//
//    /* Print read and write speeds as integers */
//    rt_kprintf("Write speed: %u KB/s (%u ms)\n", write_speed_kbs, write_time_ms);
//    rt_kprintf("Read speed: %u KB/s (%u ms)\n", read_speed_kbs, read_time_ms);
//}
//MSH_CMD_EXPORT(hyper_ram_test, hyper_ram_test);

int hyper_ram_init(void)
{
    /* Change OM_RESET back to normal IO mode. */
    R_IOPORT_PinCfg(&g_ioport_ctrl,
                    OSPI_OM_RESET,
                    IOPORT_CFG_PORT_DIRECTION_OUTPUT
                        | IOPORT_CFG_DRIVE_HIGH
                        | IOPORT_CFG_PORT_DIRECTION_OUTPUT
                        | IOPORT_CFG_PORT_OUTPUT_HIGH);

    /* Pin reset the OctaFlash */
    R_BSP_PinWrite(OSPI_OM_RESET, BSP_IO_LEVEL_LOW);
    HYPER_RAM_RESET_DELAY();
    R_BSP_PinWrite(OSPI_OM_RESET, BSP_IO_LEVEL_HIGH);
    HYPER_RAM_RESET_DELAY();

    /* Open the interface and immediately transition to 8D-8D-8D mode */
    R_OSPI_B_Open((spi_flash_ctrl_t *)&ram_ctrl, &ram_cfg);
    R_OSPI_B_SpiProtocolSet(&ram_ctrl, SPI_FLASH_PROTOCOL_8D_8D_8D);

    uint16_t cfg_reg0 = 0;
    hyper_ram_config_get(HYPER_RAM_CFG_REG_0_ADDRESS, &cfg_reg0);
    rt_kprintf("Read CR0 value: 0x%x\n", swap16(cfg_reg0));

    // CR0 default: 0x8F2F
    uint16_t value = 0x8f1d;
    hyper_ram_config_set(HYPER_RAM_CFG_REG_0_ADDRESS, swap16(value));
    rt_kprintf("Set CR0 to 0x%x\n", value);

    cfg_reg0 = 0;
    hyper_ram_config_get(HYPER_RAM_CFG_REG_0_ADDRESS, &cfg_reg0);
    rt_kprintf("Read CR0 value: 0x%x\n", swap16(cfg_reg0));

    return RT_EOK;
}
INIT_BOARD_EXPORT(hyper_ram_init);

