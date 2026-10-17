#ifndef PDU_ROUTER_H
#define PDU_ROUTER_H

/* Public interface for routing diagnostic protocol data units between layers. */
#include "project_types.h"

/* Route a complete received UDS payload from CanTp to the DCM. */
void PduR_DcmRxIndication(const uint8_t *uds, size_t uds_length);

/* Route a DCM response downward to CanTp for transport framing. */
Std_ReturnType PduR_DcmTransmit(const uint8_t *uds, size_t uds_length);

#endif
