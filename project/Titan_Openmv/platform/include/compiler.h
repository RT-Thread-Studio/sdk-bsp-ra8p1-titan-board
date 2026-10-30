#ifndef RA8_COMPILER_CONFIG_H
#define RA8_COMPILER_CONFIG_H
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#ifdef __ARMCC_VERSION
#include <sys/errno.h>
#endif
#ifndef isnanf
#define isnanf(value) isnan(value)
#endif
#ifndef isinff
#define isinff(value) isinf(value)
#endif
#ifdef __cplusplus
extern "C" {
#endif
long random(void);
#ifdef __cplusplus
}
#endif
#ifndef M_PI
#define M_PI 3.14159265358979323846
#define M_PI_2 1.57079632679489661923
#define M_PI_4 0.78539816339744830962
#define M_2_PI 0.63661977236758134308
#define M_1_PI 0.31830988618379067154
#define M_LN2 0.69314718055994530942
#define M_LN10 2.30258509299404568402
#endif
/* Passed with -include by both compilers, before any upstream header. */
#define CMSIS_MCU_H "hal_data.h"
#ifdef __ARMCC_VERSION
#define _sb_memory_start Image$$STREAM$$ZI$$Base
#define _sb_memory_end Image$$STREAM$$ZI$$Limit
#endif
#endif
