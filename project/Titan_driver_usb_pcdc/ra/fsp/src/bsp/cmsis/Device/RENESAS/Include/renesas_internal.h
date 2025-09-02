/*
* Copyright (c) 2020 - 2025 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
*/

/* Ensure Renesas MCU variation definitions are included to ensure MCU
 * specific register variations are handled correctly. */
#ifndef RENESAS_INTERNAL_H
 #define RENESAS_INTERNAL_H

/** @addtogroup Renesas
 * @{
 */

/** @addtogroup RA
 * @{
 */

 #ifdef __cplusplus
extern "C" {
 #endif

 #if BSP_MCU_GROUP_RA2T1
  #include "R7FA2T107.h"
 #elif BSP_MCU_GROUP_RA8P1
  #if 0U == BSP_CFG_CPU_CORE
   #include "R7KA8P1KF_core0.h"
  #elif 1U == BSP_CFG_CPU_CORE
   #include "R7KA8P1KF_core1.h"
  #else
   #warning "Unsupported CPU number"
  #endif
 #elif BSP_MCU_GROUP_RA8M2
  #if 0U == BSP_CFG_CPU_CORE
   #include "R7KA8M2JF_core0.h"
  #elif 1U == BSP_CFG_CPU_CORE
   #include "R7KA8M2JF_core1.h"
  #else
   #warning "Unsupported CPU number"
  #endif
 #elif BSP_MCU_GROUP_RA8T2
  #if 0U == BSP_CFG_CPU_CORE
   #include "R7KA8T2LF_core0.h"
  #elif 1U == BSP_CFG_CPU_CORE
   #include "R7KA8T2LF_core1.h"
  #else
   #warning "Unsupported CPU number"
  #endif
 #elif BSP_MCU_GROUP_RA8D2
  #if 0U == BSP_CFG_CPU_CORE
   #include "R7KA8D2KF_core0.h"
  #elif 1U == BSP_CFG_CPU_CORE
   #include "R7KA8D2KF_core1.h"
  #else
   #warning "Unsupported CPU number"
  #endif
 #elif BSP_MCU_GROUP_RA4C1
  #include "R7FA4C1BD.h"
 #elif BSP_MCU_GROUP_RA0L1
  #include "R7FA0L107.h"
 #else
  #warning "Unsupported MCU"
 #endif

 #ifdef __cplusplus
}
 #endif

/** @} */ /* End of group RA */

/** @} */ /* End of group Renesas */

#endif
