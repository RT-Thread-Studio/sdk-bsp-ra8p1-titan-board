/* OpenMV's protocol includes cmsis_gcc.h directly. Select the FSP CMSIS
 * implementation for the active compiler without changing upstream files. */
#if defined(__ARMCC_VERSION)
#include <arm/CMSIS_6/CMSIS/Core/Include/cmsis_armclang.h>
#else
#include <arm/CMSIS_6/CMSIS/Core/Include/cmsis_gcc.h>
#endif
