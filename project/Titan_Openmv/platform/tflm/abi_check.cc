#include "tensorflow/lite/core/c/common.h"
#include "arm_nn_types.h"
#if defined(__FINITE_MATH_ONLY__) && __FINITE_MATH_ONLY__
#error OpenMV and MicroPython require NaN and infinity support
#endif
// This project builds TFLM, CMSIS-NN, Ethos-U and FSP from source with the
// same GCC flags. TfLiteType/TfLiteStatus are native C enums, not serialized
// 32-bit fields: FlatBuffers uses its own explicitly sized schema enums and
// ConvertTensorType maps between them. No precompiled TFLM ABI is imported.
// The specified GCC 13.3 newlib/libgcc use the default small-enum ABI. Retain
// linker ABI diagnostics, and reject a local flag/header override here; do
// not reintroduce -fno-short-enums for only the ML source group.
#ifndef __ARM_SIZEOF_MINIMAL_ENUM
#error "Titan Board requires the Arm GCC enum ABI definition"
#endif
static_assert(__ARM_SIZEOF_MINIMAL_ENUM == 1,
              "Match the specified GCC 13.3 small-enum runtime ABI");
static_assert(sizeof(TfLiteType) == __ARM_SIZEOF_MINIMAL_ENUM &&
              sizeof(TfLiteStatus) == __ARM_SIZEOF_MINIMAL_ENUM,
              "TFLM public enums differ from the active compiler ABI");
static_assert(sizeof(void *) == 4, "The RA8P1 port requires 32-bit pointers");
static_assert(sizeof(int32_t) == 4 && sizeof(float) == 4,
              "TFLM/CMSIS-NN require 32-bit tensor scalars");
static_assert(sizeof(TfLiteQuantizationParams) == 8,
              "Unexpected TFLM quantization parameter ABI");
static_assert(sizeof(cmsis_nn_dims) == 16, "Unexpected CMSIS-NN dimension ABI");
