#ifndef CAN_IF_H
#define CAN_IF_H

/* Public boundary for simulated CAN Interface (CanIf) frame handling. */
#include "project_types.h"

/* Validate a received CAN frame and forward accepted diagnostic frames to CanTp. */
void CanIf_RxIndication(uint32_t can_id,
                        const uint8_t *data,
                        size_t data_length);

/* Validate and send a response frame to the virtual CAN endpoint. */
Std_ReturnType CanIf_Transmit(const uint8_t *data, size_t data_length);

#endif
