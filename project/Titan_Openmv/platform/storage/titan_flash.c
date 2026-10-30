/* SPDX-License-Identifier: Apache-2.0 */
#include <errno.h>
#include <string.h>
#include <rtthread.h>
#include "hal_data.h"
#include "titan_flash.h"

_Static_assert(OSPI_B_CFG_DMAC_SUPPORT_ENABLE == 0, "Flash uses bounded manual commands");
_Static_assert(OSPI_B_CFG_PREFETCH_FUNCTION == 0, "Flash manual access must not use prefetch");
_Static_assert(OSPI_B_CFG_COMBINATION_FUNCTION == OSPI_B_COMBINATION_FUNCTION_DISABLE,
               "Flash does not use mapped write combination");
_Static_assert(BSP_CFG_XTAL_HZ == 24000000 &&
               BSP_CFG_OCTACLK_SOURCE == BSP_CLOCKS_SOURCE_CLOCK_MAIN_OSC &&
               BSP_CFG_OCTACLK_DIV == BSP_CLOCKS_OCTA_CLOCK_DIV_1,
               "W25Q64 standard SPI requires the project 24 MHz clock view");

#define FLASH_PAGE_SIZE 256U
#define FLASH_COMMAND_TIMEOUT_US 1000U
#define FLASH_PROGRAM_TIMEOUT_MS 10U
#define FLASH_ERASE_TIMEOUT_MS 1000U
#define FLASH_SR_BUSY 1U
#define FLASH_SR_WEL 2U
_Static_assert(TITAN_FLASH_PROGRAM_SIZE == 8U, "Manual command data is limited to 8 bytes");
/* RA8P1 UM 45.2.4.1: CS1 command anomalies are separate from AXI mapping
 * errors. Generic CMSIS BRGOF/BRGUF bits are reserved on this device. */
#define FLASH_COMMAND_ERRORS (R_XSPI0_INTS_PERTO_Msk | \
    ((R_XSPI0_INTS_DSTOCS_Msk | R_XSPI0_INTS_ECSCS_Msk | R_XSPI0_INTS_CAFAILCS_Msk) << 1))
#define FLASH_BRIDGE_ERRORS (R_XSPI0_INTS_BUSERRCH_Msk | (R_XSPI0_INTS_BUSERRCH_Msk << 1))
#define FLASH_STATUS_HANDLED (R_XSPI0_INTS_CMDCMP_Msk | FLASH_COMMAND_ERRORS | FLASH_BRIDGE_ERRORS)
#define FLASH_MAPPING_ACCESS (R_XSPI0_BMCTL0_CH0CS0ACC_Msk | R_XSPI0_BMCTL0_CH0CS1ACC_Msk | \
    R_XSPI0_BMCTL0_CH1CS0ACC_Msk | R_XSPI0_BMCTL0_CH1CS1ACC_Msk)

static volatile bool flash_ready;
static bool flash_attempted;
static int flash_fault;
static bool flash_bridge_probe_pending;
static uint8_t flash_jedec_id[3];

static uint32_t capture_status(R_XSPI0_Type *reg, uint32_t status)
{
    uint32_t bmctl0 = reg->BMCTL0;
    uint32_t bridge = status & FLASH_BRIDGE_ERRORS;
    if (bridge) {
        rt_base_t level = rt_hw_interrupt_disable();
        flash_bridge_probe_pending = true;
        rt_hw_interrupt_enable(level);
    }
    return bmctl0;
}

static int fail(int error)
{
    /* Latch the first error so uncertain media rejects all later I/O. */
    rt_base_t level = rt_hw_interrupt_disable();
    if (!flash_fault) { flash_fault = error; }
    flash_ready = false;
    rt_hw_interrupt_enable(level);
    return flash_fault;
}

static int wait_command(R_XSPI0_Type *reg)
{
    for (unsigned us = 0; us < FLASH_COMMAND_TIMEOUT_US; ++us) {
        if (!(reg->CDCTL0 & R_XSPI0_CDCTL0_TRREQ_Msk)) { return 0; }
        rt_hw_us_delay(1);
    }
    /* RA8P1 UM 45.2.2.4: clearing TRREQ cancels the manual transaction.
     * Even when cancellation takes effect, a program/erase may have reached
     * the chip. Latch failure and never format/reuse uncertain media. */
    capture_status(reg, reg->INTS);
    reg->CDCTL0 &= ~R_XSPI0_CDCTL0_TRREQ_Msk;
    __DSB();
    return fail(-ETIMEDOUT);
}

static int command(uint8_t opcode, uint32_t address, uint8_t address_length,
                   void *data, size_t length, bool writing)
{
    if (flash_fault) { return flash_fault; }
    if (length > 8 || (length && !data) || (!writing && !length)) { return -EINVAL; }
    R_XSPI0_Type *reg = g_ospi_b_ctrl.p_reg;
    int result = wait_command(reg);
    if (result) { return result; }
    uint32_t pending = reg->INTS;
    uint32_t bmctl0 = capture_status(reg, pending);
    if (bmctl0 & FLASH_MAPPING_ACCESS) {
        return fail(-EIO);
    }
    if (pending & FLASH_COMMAND_ERRORS) { return fail(-EIO); }
    reg->INTC = pending & FLASH_STATUS_HANDLED;
    const uint32_t control = 1U << R_XSPI0_CDCTL0_CSSEL_Pos;
    reg->CDCTL0 = control;
    reg->CDBUF[0].CDT = ((uint32_t)opcode << 24) |
        (1U << R_XSPI0_CDBUF_CDT_CMDSIZE_Pos) |
        ((uint32_t)address_length << R_XSPI0_CDBUF_CDT_ADDSIZE_Pos) |
        ((uint32_t)length << R_XSPI0_CDBUF_CDT_DATASIZE_Pos) |
        ((uint32_t)(writing ? SPI_FLASH_DIRECT_TRANSFER_DIR_WRITE : SPI_FLASH_DIRECT_TRANSFER_DIR_READ)
         << R_XSPI0_CDBUF_CDT_TRTYPE_Pos);
    reg->CDBUF[0].CDA = address;
    if (writing && length) {
        uint64_t value = 0;
        memcpy(&value, data, length);
        reg->CDBUF[0].CDD0 = (uint32_t)value;
        reg->CDBUF[0].CDD1 = (uint32_t)(value >> 32);
    }
    __DSB();
    reg->CDCTL0 = control | R_XSPI0_CDCTL0_TRREQ_Msk;
    result = wait_command(reg);
    if (result) { return result; }
    uint32_t status = reg->INTS;
    bmctl0 = capture_status(reg, status);
    reg->INTC = status & FLASH_STATUS_HANDLED;
    if (bmctl0 & FLASH_MAPPING_ACCESS) {
        return fail(-EIO);
    }
    if (!(status & R_XSPI0_INTS_CMDCMP_Msk) || (status & FLASH_COMMAND_ERRORS)) { return fail(-EIO); }
    if (!writing) {
        uint64_t value = reg->CDBUF[0].CDD0;
        if (length > 4) { value |= (uint64_t)reg->CDBUF[0].CDD1 << 32; }
        memcpy(data, &value, length);
    }
    return 0;
}

static int probe_manual_path(void)
{
    if (!flash_ready || !flash_bridge_probe_pending) { return 0; }
    /* The caller has just observed WIP=0. Probe via manual registers, never
     * by reading the disabled map. command() may record a new bridge event
     * during the probe; leave it pending for a later bounded check. */
    flash_bridge_probe_pending = false;
    uint8_t id[3];
    int result = command(0x9f, 0, 0, id, sizeof(id), false);
    if (result) { return result; }
    if (memcmp(id, flash_jedec_id, sizeof(id))) {
        return fail(-EIO);
    }
    return 0;
}

static int wait_ready(uint32_t timeout_ms)
{
    rt_tick_t started = rt_tick_get();
    rt_tick_t timeout = rt_tick_from_millisecond(timeout_ms);
    if (!timeout) { timeout = 1; }
    bool fast = (DWT->CTRL & DWT_CTRL_CYCCNTENA_Msk) != 0;
    uint32_t cycles = DWT->CYCCNT;
    /* Short programs should finish before paying for a scheduler tick on
     * every 8-byte write. Poll with IRQs enabled for at most 1 ms; erases
     * still yield after the initial 100 us probe. A separate iteration cap
     * guarantees a yield if CYCCNT stops after making initial progress. */
    const bool programming = timeout_ms == FLASH_PROGRAM_TIMEOUT_MS;
    const uint32_t fast_limit = SystemCoreClock / (programming ? 1000U : 10000U);
    const unsigned poll_limit = programming ? 1024U : 32U;
    unsigned fast_polls = 0;
    for (;;) {
        uint8_t status;
        int result = command(0x05, 0, 0, &status, 1, false);
        if (result) { return result; }
        if (!(status & FLASH_SR_BUSY)) { return probe_manual_path(); }
        if ((rt_tick_t)(rt_tick_get() - started) >= timeout) {
            return fail(-ETIMEDOUT);
        }
        uint32_t elapsed_cycles = DWT->CYCCNT - cycles;
        if (fast && fast_polls++ < poll_limit && elapsed_cycles && elapsed_cycles < fast_limit) {
            rt_hw_us_delay(1);
        } else {
            fast = false;
            rt_thread_mdelay(1);
        }
    }
}

static int write_enable(void)
{
    int result = command(0x06, 0, 0, NULL, 0, true);
    uint8_t status;
    if (!result) { result = command(0x05, 0, 0, &status, 1, false); }
    if (result) { return result; }
    if (status & FLASH_SR_WEL) {
        /* A bridge event can also arrive while issuing WREN. Validate the
         * manual path before the following program/erase, without changing WEL. */
        return flash_bridge_probe_pending ? wait_ready(FLASH_PROGRAM_TIMEOUT_MS) : 0;
    }
    return fail(-EACCES);
}

bool titan_flash_is_ready(void) { return flash_ready; }

int titan_flash_init(void)
{
    if (flash_ready) { return 0; }
    if (flash_attempted) { return flash_fault ? flash_fault : -ENODEV; }
    flash_attempted = true;
    const ospi_b_extended_cfg_t *cfg = g_ospi_b_cfg.p_extend;
    if (!cfg || cfg->ospi_b_unit != 0 || cfg->channel != 1 ||
        g_ospi_b_cfg.spi_protocol != SPI_FLASH_PROTOCOL_1S_1S_1S) { return fail(-EINVAL); }
    if (R_OSPI_B_Open(&g_ospi_b_ctrl, &g_ospi_b_cfg) != FSP_SUCCESS) { return fail(-EIO); }
    /* Neither CPU nor DMA accesses the Flash mapping. All chip I/O uses the
     * manual command registers; no DMA channel is claimed by this driver. */
    R_XSPI0_Type *reg = g_ospi_b_ctrl.p_reg;
    reg->BMCTL0 = 0;
    __DSB();
    int result = wait_ready(FLASH_ERASE_TIMEOUT_MS);
    if (result) { return result; }
    uint8_t id[3];
    result = command(0x9f, 0, 0, id, sizeof(id), false);
    if (result) { return result; }
    /* Winbond serial NOR family, 64 Mbit. Reject floating/all-zero buses and
     * other capacities before any command carrying a storage address. */
    if (id[0] != 0xef || (id[1] != 0x40 && id[1] != 0x60 && id[1] != 0x70) || id[2] != 0x17) {
        return fail(-ENODEV);
    }
    memcpy(flash_jedec_id, id, sizeof(id));
    flash_ready = true;
    return 0;
}

static int valid_range(uint32_t offset, const void *buffer, size_t length)
{
    if (offset > TITAN_FLASH_STORAGE_SIZE || length > TITAN_FLASH_STORAGE_SIZE - offset ||
        (length && !buffer)) { return -EINVAL; }
    return flash_ready ? 0 : (flash_fault ? flash_fault : -ENODEV);
}

int titan_flash_read(uint32_t offset, void *buffer, size_t length)
{
    int result = valid_range(offset, buffer, length);
    if (result || !length) { return result; }
    result = wait_ready(FLASH_ERASE_TIMEOUT_MS);
    uint8_t *destination = buffer;
    while (!result && length) {
        size_t count = length < 8 ? length : 8;
        result = command(0x03, TITAN_FLASH_STORAGE_OFFSET + offset, 3, destination, count, false);
        destination += count; offset += (uint32_t)count; length -= count;
    }
    if (!result && flash_bridge_probe_pending) { result = wait_ready(FLASH_ERASE_TIMEOUT_MS); }
    return result;
}

int titan_flash_program(uint32_t offset, const void *buffer, size_t length)
{
    int result = valid_range(offset, buffer, length);
    if (result || !length) { return result; }
    const uint8_t *source = buffer;
    while (length) {
        size_t count = FLASH_PAGE_SIZE - (offset & (FLASH_PAGE_SIZE - 1));
        if (count > TITAN_FLASH_PROGRAM_SIZE) { count = TITAN_FLASH_PROGRAM_SIZE; }
        if (count > length) { count = length; }
        result = wait_ready(FLASH_PROGRAM_TIMEOUT_MS);
        if (!result) { result = write_enable(); }
        if (!result) { result = command(0x02, TITAN_FLASH_STORAGE_OFFSET + offset, 3, (void *)source, count, true); }
        if (!result) { result = wait_ready(FLASH_PROGRAM_TIMEOUT_MS); }
        if (result) { return result; }
        /* A protected sector can ignore programming without setting WIP.
         * Manual reads bypass both CPU cache and the OSPI last-line buffer. */
        for (size_t i = 0; i < count; i += 8) {
            uint8_t verify[8];
            size_t bytes = count - i < 8 ? count - i : 8;
            result = command(0x03, TITAN_FLASH_STORAGE_OFFSET + offset + (uint32_t)i, 3,
                             verify, bytes, false);
            if (result) { return result; }
            if (memcmp(verify, source + i, bytes)) {
                return fail(-EIO);
            }
        }
        source += count; offset += (uint32_t)count; length -= count;
    }
    return flash_bridge_probe_pending ? wait_ready(FLASH_PROGRAM_TIMEOUT_MS) : 0;
}

int titan_flash_erase_sector(uint32_t offset)
{
    if ((offset & (TITAN_FLASH_ERASE_SIZE - 1)) ||
        offset > TITAN_FLASH_STORAGE_SIZE - TITAN_FLASH_ERASE_SIZE) { return -EINVAL; }
    if (!flash_ready) { return flash_fault ? flash_fault : -ENODEV; }
    int result = wait_ready(FLASH_ERASE_TIMEOUT_MS);
    if (!result) { result = write_enable(); }
    if (!result) { result = command(0x20, TITAN_FLASH_STORAGE_OFFSET + offset, 3, NULL, 0, true); }
    if (!result) { result = wait_ready(FLASH_ERASE_TIMEOUT_MS); }
    for (size_t i = 0; !result && i < TITAN_FLASH_ERASE_SIZE; i += 8) {
        uint64_t verify;
        result = command(0x03, TITAN_FLASH_STORAGE_OFFSET + offset + (uint32_t)i, 3, &verify, 8, false);
        if (!result && verify != UINT64_MAX) {
            result = fail(-EIO);
        }
    }
    if (!result && flash_bridge_probe_pending) { result = wait_ready(FLASH_ERASE_TIMEOUT_MS); }
    return result;
}

int titan_flash_sync(void)
{
    if (!flash_ready) { return flash_fault ? flash_fault : -ENODEV; }
    return wait_ready(FLASH_ERASE_TIMEOUT_MS);
}
