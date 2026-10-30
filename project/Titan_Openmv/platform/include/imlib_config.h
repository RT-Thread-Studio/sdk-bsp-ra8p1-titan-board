/* The NPU and software vision APIs are independent capabilities. Keep the
 * image module complete in both firmware variants; the linker/storage layout
 * owns the code-size budget, rather than replacing methods with unavailable
 * stubs when NPU inference is enabled.
 */
#ifndef TITAN_IMLIB_CONFIG_H
#define TITAN_IMLIB_CONFIG_H
#include <rtconfig.h>
#include "imlib_full_config.h"
#endif
