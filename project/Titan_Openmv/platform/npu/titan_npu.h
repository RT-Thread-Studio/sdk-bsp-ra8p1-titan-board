/* SPDX-License-Identifier: Apache-2.0 */
#ifndef TITAN_NPU_H
#define TITAN_NPU_H
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
struct ethosu_driver;
int titan_npu_init(void);
int titan_npu_invoke(struct ethosu_driver *driver, const void *commands, int command_size,
                     uint64_t *const addresses, const size_t *sizes, int count, void *user);
#ifdef __cplusplus
}
#endif
#endif
