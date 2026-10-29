#ifndef RA8_MPCONFIGPORT_H
#define RA8_MPCONFIGPORT_H
#include <stdint.h>
#include <rtconfig.h>
#include <alloca.h>
#ifdef __ARMCC_VERSION
#include <sys/errno.h>
#endif
#define MICROPY_CONFIG_ROM_LEVEL MICROPY_CONFIG_ROM_LEVEL_EXTRA_FEATURES
#define MICROPY_ENABLE_GC (1)
#define MICROPY_GC_SPLIT_HEAP (1)
#define MICROPY_ENABLE_FINALISER (1)
#define MICROPY_ENABLE_EMERGENCY_EXCEPTION_BUF (1)
#define MICROPY_GC_STACK_ENTRY_TYPE uint32_t
#define MICROPY_LONGINT_IMPL MICROPY_LONGINT_IMPL_MPZ
#define MICROPY_FLOAT_IMPL MICROPY_FLOAT_IMPL_FLOAT
#define MICROPY_PERSISTENT_CODE_LOAD (1)
#define MICROPY_MODULE_FROZEN_MPY (1)
#define MICROPY_QSTR_EXTRA_POOL mp_qstr_frozen_const_pool
#define MICROPY_ENABLE_SCHEDULER (1)
#define MICROPY_ENABLE_VM_ABORT (1)
/* OpenMV uses these notifications to report script state to the IDE. */
#define MICROPY_BOARD_BEFORE_PYTHON_EXEC(input_kind, exec_flags) do { \
    extern void stdio_channel_pyexec_hook(bool); \
    stdio_channel_pyexec_hook(true); \
} while (0)
#define MICROPY_BOARD_AFTER_PYTHON_EXEC(input_kind, exec_flags, nlr, ret) do { \
    extern void stdio_channel_pyexec_hook(bool); \
    stdio_channel_pyexec_hook(false); \
} while (0)
#define MICROPY_VM_HOOK_EXC do { extern void uma_collect(void); uma_collect(); } while (0);
#define MICROPY_SCHEDULER_DEPTH (8)
#define MICROPY_PY_THREAD (0)
#define MICROPY_PY_SYS_PLATFORM "renesas-ra8p1"
#define MICROPY_HW_BOARD_NAME "RA8P1 Titan Board"
#define MICROPY_HW_MCU_NAME "R7KA8P1KF"
#define MICROPY_PY_SYS_STDFILES (1)
#define MICROPY_READER_VFS (1)
#define MICROPY_VFS (1)
#define MICROPY_VFS_POSIX (1)
#define MICROPY_VFS_POSIX_FILE (1)
#define MICROPY_PY_OS (1)
#define MICROPY_PY_OS_STATVFS (0)
#define MICROPY_SCHEDULER_STATIC_NODES (1)
#define MICROPY_PY_PENDSV_ENTER rt_base_t omv_irq_state = rt_hw_interrupt_disable()
#define MICROPY_PY_PENDSV_EXIT rt_hw_interrupt_enable(omv_irq_state)
#define MICROPY_PY_OS_INCLUDEFILE "modos_port.c"
#define MICROPY_PY_TIME (1)
#define MICROPY_PY_TIME_INCLUDEFILE "modtime_port.c"
#define MICROPY_EPOCH_IS_1970 (1)
#define MICROPY_PY_TIME_GMTIME_LOCALTIME_MKTIME (1)
#define MICROPY_PY_TIME_TIME_TIME_NS (1)
#define MICROPY_PY_MACHINE (1)
#define MICROPY_PY_MACHINE_PIN (1)
#define MICROPY_PY_MACHINE_UART (0)
#define MICROPY_PY_MACHINE_UART_READCHAR_WRITECHAR (1)
#define MICROPY_PY_MACHINE_PWM (0)
#define MICROPY_PY_MACHINE_PWM_DUTY (1)
#define MICROPY_PY_MACHINE_I2C (1)
#define MICROPY_PY_MACHINE_I2C_TRANSFER_WRITE1 (1)
#define MICROPY_PY_MACHINE_SOFTI2C (1)
#define MICROPY_PY_MACHINE_SPI (0)
#define MICROPY_PY_MACHINE_SOFTSPI (1)
#define MICROPY_PY_MACHINE_SIGNAL (1)
#define MICROPY_PY_MACHINE_PULSE (1)
#define MICROPY_PY_MACHINE_RTC (0)
#define MICROPY_PY_MACHINE_TIMER (1)
#define MICROPY_PY_NETWORK (0)
#define MICROPY_PY_NETWORK_RA8 (0)
#define MICROPY_PY_SOCKET (0)
/* Use RT-Thread's existing TCP/IP stack through SAL, not a second raw lwIP port. */
#define MICROPY_PY_LWIP (0)
/* Platform owns ADC_B single scans; the legacy RT ADC driver stays disabled. */
#define MICROPY_PY_MACHINE_ADC (0)
#ifdef RT_USING_WDT
#define MICROPY_PY_MACHINE_WDT (0)
#endif
uint32_t ra8_random_seed(void);
#define MICROPY_PY_RANDOM_SEED_INIT_FUNC (ra8_random_seed())
#ifndef MICROPY_PY_CSI
#define MICROPY_PY_CSI (1)
#endif
/* OpenMV 5 separates the Python csi.CSI API from the common CSI driver. */
#define MICROPY_PY_CSI_NG (1)
#define MICROPY_PY_IMAGE (1)
#define MICROPY_PY_ULAB (1)
/* OpenMV Image.to_ndarray and ML tensors require the same 4-D ulab ABI in
 * every translation unit. MICROPY_PY_ULAB alone does not enable that binding. */
#define MODULE_ULAB_ENABLED (1)
#define ULAB_MAX_DIMS (4)
#ifdef BSP_USING_OPENMV_ML
#define MICROPY_PY_ML (1)
#else
#define MICROPY_PY_ML (0)
#endif
#ifndef MICROPY_PY_ML_TFLM
#define MICROPY_PY_ML_TFLM MICROPY_PY_ML
#endif
#define MICROPY_PY_CLOCK (1)
#define MICROPY_PY_OMV (1)
#define MICROPY_PY_UMALLOC (1)
#define MICROPY_PY_PROTOCOL (1)
#define MICROPY_PY_CRC (1)
#ifndef MICROPY_PY_DISPLAY
#define MICROPY_PY_DISPLAY (0)
#endif
#define MICROPY_PY_FIR (0)
#define MICROPY_PY_IMU (0)
#define MICROPY_PY_TOF (0)
#define MICROPY_PY_TV (0)
#define MICROPY_PY_GIF (1)
#define MICROPY_PY_MJPEG (1)
#define MICROPY_EVENT_POLL_HOOK do { mp_handle_pending(true); mp_hal_poll(); } while (0);
#define MICROPY_INTERNAL_EVENT_HOOK mp_hal_poll()
#define MP_STATE_PORT MP_STATE_VM
#define MP_SSIZE_MAX (0x7fffffff)
typedef intptr_t mp_int_t;
typedef uintptr_t mp_uint_t;
typedef long mp_off_t;
#endif
