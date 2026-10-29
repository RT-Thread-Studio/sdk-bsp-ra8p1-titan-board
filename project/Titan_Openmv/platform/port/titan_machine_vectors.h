#ifndef TITAN_MACHINE_VECTORS_H
#define TITAN_MACHINE_VECTORS_H
#include "bsp_api.h"
#define TITAN_IIC1_RXI_IRQ ((IRQn_Type)0)
#define TITAN_IIC1_TXI_IRQ ((IRQn_Type)1)
#define TITAN_IIC1_TEI_IRQ ((IRQn_Type)2)
#define TITAN_IIC1_ERI_IRQ ((IRQn_Type)74)
static const IRQn_Type titan_pin_irq_vectors[32] = {75, -1, -1, -1, 51, -1, 76, 77, 78, 79, 80, 81, 82, 83, -1, -1, 84, 85, 86, -1, 87, -1, -1, 88, 89, 90, 91, 92, 93, 94, -1, -1};
#define TITAN_PIN_IRQ_VECTOR(channel) (titan_pin_irq_vectors[(channel)])
#endif
