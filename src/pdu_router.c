/* Simplified PDU Router: connects diagnostic transport data with the DCM. */
#include "pdu_router.h"

#include "can_tp.h"
#include "dcm.h"

#include <stdio.h>

static void PduR_PrintBytes(const uint8_t *data, size_t data_length)
{
    size_t index;

    for (index = 0U; index < data_length; index++)
    {
        printf("%02X%s",
               data[index],
               (index + 1U < data_length) ? " " : "");
    }
}

void PduR_DcmRxIndication(const uint8_t *uds, size_t uds_length)
{
    /* Validate a received UDS PDU before routing it to diagnostic processing. */
    if (uds == NULL || uds_length == 0U || uds_length > DEMO_MAX_UDS_LENGTH)
    {
        puts("[PduR] Dropped invalid diagnostic PDU.");
        return;
    }

    printf("[PduR] ROUTE | UDS: ");
    PduR_PrintBytes(uds, uds_length);
    putchar('\n');
    Dcm_ProcessRequest(uds, uds_length);
}

Std_ReturnType PduR_DcmTransmit(const uint8_t *uds, size_t uds_length)
{
    /* Forward a DCM response down to CanTp for ISO-TP framing. */
    printf("[PduR] ROUTE TX | UDS: ");
    PduR_PrintBytes(uds, uds_length);
    putchar('\n');
    return CanTp_Transmit(uds, uds_length);
}
