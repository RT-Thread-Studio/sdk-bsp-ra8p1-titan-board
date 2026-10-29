#ifndef RA8_MPHALPORT_H
#define RA8_MPHALPORT_H
#include <rtthread.h>
#include <drivers/pin.h>
#include <hal_data.h>
#include "py/mpconfig.h"
#include "py/obj.h"
#include <errno.h>
#include "titan_memory.h"
#define MP_HAL_RETRY_SYSCALL(ret, call, failure) do { \
    for (;;) { \
        (ret) = (call); \
        if ((ret) != -1) { break; } \
        int err = errno; \
        if (err == EINTR) { mp_handle_pending(true); continue; } \
        failure; \
        break; \
    } \
} while (0)
#define mp_hal_quiet_timing_enter() rt_hw_interrupt_disable()
#define mp_hal_quiet_timing_exit(state) rt_hw_interrupt_enable(state)
#define MICROPY_BEGIN_ATOMIC_SECTION() rt_hw_interrupt_disable()
#define MICROPY_END_ATOMIC_SECTION(state) rt_hw_interrupt_enable(state)
void mp_hal_poll(void);
void ra8_vm_poll_start(void);
void ra8_vm_poll_stop(void);
void ra8_vm_poll_recover(void);
void mp_hal_delay_ms(mp_uint_t ms);
void mp_hal_delay_us(mp_uint_t us);
#define mp_hal_delay_us_fast(us) mp_hal_delay_us(us)

/* Define a macro, not just a typedef: py/mphal.h otherwise installs the
 * virtual-pin fallback, which has different argument and GPIO semantics.
 */
#define mp_hal_pin_obj_t rt_base_t
mp_hal_pin_obj_t mp_hal_get_pin_obj(mp_obj_t obj);
void ra8_mp_hal_pin_mode(mp_hal_pin_obj_t pin, unsigned mode);
int ra8_mp_hal_pin_read(mp_hal_pin_obj_t pin);
void ra8_mp_hal_pin_write(mp_hal_pin_obj_t pin, int value);
#define mp_hal_pin_read(pin) ra8_mp_hal_pin_read(pin)
#define mp_hal_pin_write(pin, value) ra8_mp_hal_pin_write((pin), (value))
#define mp_hal_pin_input(pin) ra8_mp_hal_pin_mode((pin), PIN_MODE_INPUT)
#define mp_hal_pin_output(pin) ra8_mp_hal_pin_mode((pin), PIN_MODE_OUTPUT)
#define mp_hal_pin_open_drain(pin) ra8_mp_hal_pin_mode((pin), PIN_MODE_OUTPUT_OD)
#define mp_hal_pin_od_low(pin) mp_hal_pin_write((pin), 0)
#define mp_hal_pin_od_high(pin) mp_hal_pin_write((pin), 1)
#define MP_HAL_PIN_FMT "0x%04x"
#define mp_hal_pin_name(pin) ((unsigned)(pin))
mp_uint_t mp_hal_ticks_ms(void);
mp_uint_t mp_hal_ticks_us(void);
mp_uint_t mp_hal_ticks_cpu(void);
int mp_hal_stdin_rx_chr(void);
mp_uint_t mp_hal_stdout_tx_strn(const char *str, size_t len);
int mp_hal_stdin_rx_any(void);
void mp_hal_set_interrupt_char(int c);
void mp_hal_get_mac(int idx, uint8_t buf[6]);
#define MP_HAL_CLEAN_DCACHE(addr, size) titan_cache_clean((const void *)(addr), (size_t)(size))
#define MP_HAL_INVALIDATE_DCACHE(addr, size) titan_cache_invalidate((void *)(addr), (size_t)(size))
#define MP_HAL_CLEANINVALIDATE_DCACHE(addr, size) titan_cache_clean_invalidate((void *)(addr), (size_t)(size))
#endif
