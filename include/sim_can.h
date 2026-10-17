#ifndef SIM_CAN_H
#define SIM_CAN_H

/* Interface between the terminal tester and the virtual CAN bus endpoint. */
#include "project_types.h"

/* Frame a tester UDS payload and inject it into the simulated receive path. */
Std_ReturnType SimCan_SendUdsRequest(const uint8_t *uds, size_t uds_length);

/* Display an ECU response frame and its decoded UDS payload in the terminal. */
void SimCan_OnCanTransmit(uint32_t can_id,
                          const uint8_t *data,
                          size_t data_length);

#endif
