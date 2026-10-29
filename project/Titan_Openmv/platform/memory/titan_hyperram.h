/* SPDX-License-Identifier: Apache-2.0 */
#ifndef TITAN_HYPERRAM_H
#define TITAN_HYPERRAM_H
#include "bsp_api.h"

/* Called from BSP_WARM_START_POST_C before external C runtime initialization. */
fsp_err_t titan_hyperram_init(void);
extern volatile fsp_err_t titan_hyperram_status;
#endif
