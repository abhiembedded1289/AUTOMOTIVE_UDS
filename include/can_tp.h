#ifndef CAN_TP_H
#define CAN_TP_H

/* Public interface for the simulator's ISO-TP single-frame transport layer. */
#include "project_types.h"

/* Decode one supported ISO-TP frame and route the extracted UDS payload upward. */
void CanTp_RxIndication(const uint8_t *data, size_t data_length);

/* Encode a UDS response as one ISO-TP single frame and pass it to CanIf. */
Std_ReturnType CanTp_Transmit(const uint8_t *uds, size_t uds_length);

#endif
