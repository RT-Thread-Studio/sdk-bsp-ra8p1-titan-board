/* SPDX-License-Identifier: Apache-2.0 */
#ifndef TITAN_ML_AUTO_H
#define TITAN_ML_AUTO_H
#include <stddef.h>
#include <stdint.h>
/* Admission is by reviewed model bytes, independent of the SD filename. */
int titan_ml_auto_model_supported(const uint8_t *data, size_t size);
#endif
