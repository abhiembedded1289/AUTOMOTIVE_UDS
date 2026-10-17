#ifndef ECUM_H
#define ECUM_H

/* Public interface for the simulator's minimal ECU Manager reset boundary. */
#include <stdint.h>

/* Log a reset request for learning; this function never resets real hardware. */
void EcuM_RequestReset(uint8_t reset_type);

#endif
