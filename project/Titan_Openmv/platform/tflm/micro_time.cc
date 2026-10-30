/* SPDX-License-Identifier: Apache-2.0 */
#include "tensorflow/lite/micro/micro_time.h"
// mp_uint_t is uintptr_t in this port. Declare only the C clock ABI here;
// pulling FSP/Arm C++ library headers through an extern-C block is invalid.
extern "C" uintptr_t mp_hal_ticks_us(void);

namespace tflite {
// Use the existing platform clock. Do not reset DWT: CSI timing and Python
// also use it, and TFLM diagnostics must not change their time base.
uint32_t ticks_per_second() { return 1000000U; }
uint32_t GetCurrentTimeTicks() { return static_cast<uint32_t>(mp_hal_ticks_us()); }
}
