/* SPDX-License-Identifier: Apache-2.0 */
#ifndef RA8_RUSB2_REGS_H
#define RA8_RUSB2_REGS_H

#include "common/tusb_common.h"

/* Armclang narrows packed volatile bitfields to byte MMIO accesses. USBHS
 * control registers use their declared 16/32-bit width, as in FSP. Only the
 * register declarations use natural alignment; wire descriptors stay packed.
 * The upstream header checks every register offset and its complete size.
 */
#ifdef TUSB_RUSB2_TYPE_H_
#error Include the RA8 register view before the upstream register declarations
#endif
#pragma push_macro("TU_ATTR_PACKED")
#undef TU_ATTR_PACKED
#define TU_ATTR_PACKED
#include "../../ThirdParty/tinyusb/src/portable/renesas/rusb2/rusb2_type.h"
#pragma pop_macro("TU_ATTR_PACKED")

#endif
