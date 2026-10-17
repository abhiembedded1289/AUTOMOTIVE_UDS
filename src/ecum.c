/* Minimal ECU Manager stand-in for demonstrating the reset request boundary. */
#include "ecum.h"

#include <stdio.h>

void EcuM_RequestReset(uint8_t reset_type)
{
    /* Print the requested reset type; host execution continues without hardware. */
    printf("[EcuM] Simulated ECU reset requested (type 0x%02X); "
           "no hardware reset performed.\n",
           reset_type);
}
