#ifndef __SUB_0000_TENSORS_H__
#define __SUB_0000_TENSORS_H__

#include <stddef.h>
#include <stdint.h>
#include "ethosu_common.h"

extern const TensorInfo sub_0000_tensors[];
extern const size_t sub_0000_tensors_count;

#define kArenaSize_sub_0000 442368

// Addresses for each input and output buffer inside of the arena
extern const uint32_t sub_0000_address_serving_default_image_input_0;
extern const uint32_t sub_0000_address_StatefulPartitionedCall_1_70283;
extern const uint32_t sub_0000_address_StatefulPartitionedCall_0_70273;


#endif // __SUB_0000_TENSORS_H__
