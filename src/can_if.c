/* Simulated CAN Interface: filters the configured request ID and handles
 * forwarding of response frames to the virtual CAN endpoint. */
#include "can_if.h"

#include "can_tp.h"
#include "sim_can.h"

#include <stdio.h>

#define CANIF_DIAGNOSTIC_REQUEST_ID  0x7E0U
#define CANIF_DIAGNOSTIC_RESPONSE_ID 0x7E8U

static void CanIf_PrintBytes(const uint8_t *data, size_t data_length)
{
    size_t index;

    for (index = 0U; index < data_length; index++)
    {
        printf("%02X%s",
               data[index],
               (index + 1U < data_length) ? " " : "");
    }
}

void CanIf_RxIndication(uint32_t can_id,
                        const uint8_t *data,
                        size_t data_length)
{
    /* Reject invalid/unconfigured frames; pass valid diagnostic frames to CanTp. */
    if (data == NULL || can_id != CANIF_DIAGNOSTIC_REQUEST_ID)
    {
        puts("[CanIf] Ignored frame: invalid pointer or unconfigured CAN ID.");
        return;
    }

    if (data_length != DEMO_CAN_FRAME_LENGTH)
    {
        puts("[CanIf] Ignored frame: expected an 8-byte classic CAN frame.");
        return;
    }

    printf("[CanIf] RX | CAN ID: 0x%03X | Data: ", (unsigned int)can_id);
    CanIf_PrintBytes(data, data_length);
    putchar('\n');
    CanTp_RxIndication(data, data_length);
}

Std_ReturnType CanIf_Transmit(const uint8_t *data, size_t data_length)
{
    /* Validate an outgoing classic CAN frame and simulate its bus transmission. */
    if (data == NULL || data_length != DEMO_CAN_FRAME_LENGTH)
    {
        puts("[CanIf] Transmit rejected: invalid frame.");
        return E_NOT_OK;
    }

    printf("[CanIf] TX | CAN ID: 0x%03X | Data: ",
           CANIF_DIAGNOSTIC_RESPONSE_ID);
    CanIf_PrintBytes(data, data_length);
    putchar('\n');
    SimCan_OnCanTransmit(CANIF_DIAGNOSTIC_RESPONSE_ID, data, data_length);
    return E_OK;
}
