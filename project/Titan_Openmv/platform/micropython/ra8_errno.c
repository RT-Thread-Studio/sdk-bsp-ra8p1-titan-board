/* SPDX-License-Identifier: Apache-2.0
 * Arm libc and RT-Thread must use the same per-thread errno storage.
 */
#include <rtthread.h>

#ifdef __ARMCC_VERSION
/* armlink substitution covers calls from lwIP, the Python VFS/socket layer,
 * and library objects without changing either upstream libc or RT sources.
 * The public Arm ABI returns a volatile int pointer. The original library
 * implementation returned one global word, invisible to rt_get_errno().
 */
volatile int *$Sub$$__aeabi_errno_addr(void);
volatile int *$Sub$$__aeabi_errno_addr(void) {
    return _rt_errno();
}
#endif
