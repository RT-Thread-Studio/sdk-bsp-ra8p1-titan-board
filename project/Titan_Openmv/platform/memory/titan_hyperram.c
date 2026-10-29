/* SPDX-License-Identifier: Apache-2.0 */
#include "titan_hyperram.h"
#include "hal_data.h"
#include "r_ospi_b.h"

/* Titan Board OSPI1 CS0, 32 MiB HyperRAM. The command set, CR0 and latch
 * delay follow Titan_driver_hyperram. No test pattern overwrites VM memory. */
static ospi_b_instance_ctrl_t hyperram_ctrl;
volatile fsp_err_t titan_hyperram_status;

static const ospi_b_xspi_command_set_t hyperram_commands[] = {
    {
        .protocol = SPI_FLASH_PROTOCOL_8D_8D_8D,
        .frame_format = OSPI_B_FRAME_FORMAT_XSPI_PROFILE_2_EXTENDED,
        .latency_mode = OSPI_B_LATENCY_MODE_VARIABLE,
        .command_bytes = OSPI_B_COMMAND_BYTES_2,
        .address_bytes = SPI_FLASH_ADDRESS_BYTES_4,
        .read_command = 0xA000,
        .read_dummy_cycles = 5,
        .program_command = 0x2000,
        .program_dummy_cycles = 5,
        .address_msb_mask = 0xF0,
        .status_needs_address = false,
        .p_erase_commands = NULL,
    },
};
static const ospi_b_table_t hyperram_table = {
    .p_table = (void *)hyperram_commands,
    .length = 1,
};
static const ospi_b_timing_setting_t hyperram_timing = {
    .command_to_command_interval = OSPI_B_COMMAND_INTERVAL_CLOCKS_8,
    .cs_pullup_lag = OSPI_B_COMMAND_CS_PULLUP_CLOCKS_NO_EXTENSION,
    .cs_pulldown_lead = OSPI_B_COMMAND_CS_PULLDOWN_CLOCKS_NO_EXTENSION,
    .sdr_drive_timing = OSPI_B_SDR_DRIVE_TIMING_BEFORE_CK,
    .sdr_sampling_edge = OSPI_B_CK_EDGE_FALLING,
    .sdr_sampling_delay = OSPI_B_SDR_SAMPLING_DELAY_NONE,
    .ddr_sampling_extension = OSPI_B_DDR_SAMPLING_EXTENSION_1,
};
static const ospi_b_extended_cfg_t hyperram_extended = {
    .ospi_b_unit = 1,
    .channel = (ospi_b_device_number_t)0,
    .p_timing_settings = &hyperram_timing,
    .p_xspi_command_set = &hyperram_table,
    .data_latch_delay_clocks = OSPI_B_DS_TIMING_DELAY_8,
    .p_autocalibration_preamble_pattern_addr = NULL,
};
static const spi_flash_cfg_t hyperram_config = {
    .spi_protocol = SPI_FLASH_PROTOCOL_8D_8D_8D,
    .read_mode = SPI_FLASH_READ_MODE_STANDARD,
    .address_bytes = SPI_FLASH_ADDRESS_BYTES_4,
    .dummy_clocks = SPI_FLASH_DUMMY_CLOCKS_DEFAULT,
    .page_size_bytes = 64,
    .write_status_bit = 1,
    .write_enable_bit = 1,
    .p_extend = &hyperram_extended,
};

fsp_err_t titan_hyperram_init(void)
{
#if BSP_CFG_OCTACLK_SOURCE == BSP_CLOCKS_SOURCE_CLOCK_PLL1Q
    bsp_octaclk_settings_t clock_settings = {
        .source_clock = BSP_CLOCKS_SOURCE_CLOCK_PLL1Q,
        .divider = BSP_CLOCKS_OCTA_CLOCK_DIV_2,
    };
    R_BSP_OctaclkUpdate(&clock_settings);
#endif
    fsp_err_t err = R_IOPORT_PinCfg(&g_ioport_ctrl, BSP_IO_PORT_12_PIN_07,
        IOPORT_CFG_DRIVE_HIGH | IOPORT_CFG_PORT_DIRECTION_OUTPUT | IOPORT_CFG_PORT_OUTPUT_HIGH);
    if (err != FSP_SUCCESS) { return titan_hyperram_status = err; }
    R_BSP_PinWrite(BSP_IO_PORT_12_PIN_07, BSP_IO_LEVEL_LOW);
    R_BSP_SoftwareDelay(10, BSP_DELAY_UNITS_MICROSECONDS);
    R_BSP_PinWrite(BSP_IO_PORT_12_PIN_07, BSP_IO_LEVEL_HIGH);
    R_BSP_SoftwareDelay(10, BSP_DELAY_UNITS_MICROSECONDS);

    err = R_OSPI_B_Open(&hyperram_ctrl, &hyperram_config);
    if (err != FSP_SUCCESS) { return titan_hyperram_status = err; }
    err = R_OSPI_B_SpiProtocolSet(&hyperram_ctrl, SPI_FLASH_PROTOCOL_8D_8D_8D);
    if (err != FSP_SUCCESS) { return titan_hyperram_status = err; }

    /* RA8P1 User's Manual 45.5.4: CPU narrow writes are not supported with
     * OSPI write combination enabled. The VM, queues and vision decoder all
     * perform byte/word stores. Configure both bridge channels before the
     * external C runtime/heap is initialized; keep read prefetch enabled. */
    for (unsigned channel = 0; channel < 2; ++channel)
    {
        R_XSPI1->BMCFGCH[channel] =
            (R_XSPI1->BMCFGCH[channel] &
             ~(R_XSPI0_BMCFGCH_MWRCOMB_Msk | R_XSPI0_BMCFGCH_MWRSIZE_Msk)) |
            R_XSPI0_BMCFGCH_WRMD_Msk;
    }
    __DSB();
    R_XSPI1->LIOCFGCS_b[0].WRMSKMD = 1;

    /* CR0=0x8f0d: the register transfer uses byte-swapped data. */
    spi_flash_direct_transfer_t transfer = {
        .address = 0x01000000,
        .address_length = 4,
        .command = 0x6000,
        .command_length = 2,
        .data = 0x0d8f,
        .data_length = 2,
        .dummy_cycles = 0,
    };
    err = R_OSPI_B_DirectTransfer(&hyperram_ctrl, &transfer, SPI_FLASH_DIRECT_TRANSFER_DIR_WRITE);
    if (err != FSP_SUCCESS) { return titan_hyperram_status = err; }
    transfer.command = 0xE000;
    transfer.dummy_cycles = 5;
    transfer.data = 0;
    err = R_OSPI_B_DirectTransfer(&hyperram_ctrl, &transfer, SPI_FLASH_DIRECT_TRANSFER_DIR_READ);
    if (err == FSP_SUCCESS && (uint16_t)transfer.data != 0x0d8f) { err = FSP_ERR_INVALID_DATA; }
    __DSB();
    __ISB();
    return titan_hyperram_status = err;
}
