/* SPDX-License-Identifier: Apache-2.0 */
/* Use the matching vendored TFLM operator; platform guards DMA completion. */
#define ethosu_invoke_v3 titan_npu_invoke
#include "../../ThirdParty/tflm/tensorflow/lite/micro/kernels/ethos_u/ethosu.cc"
