/* Simplified CAN Transport Protocol layer: handles ISO-TP single frames only. */
#include "can_tp.h"

#include "can_if.h"
#include "pdu_router.h"

#include <stdio.h>
#include <string.h>

static void CanTp_PrintBytes(const uint8_t *data, size_t data_length)
{
    size_t index;

    for (index = 0U; index < data_length; index++)
    {
        printf("%02X%s",
               data[index],
               (index + 1U < data_length) ? " " : "");
    }
}

void CanTp_RxIndication(const uint8_t *data, size_t data_length)
{
    /* Validate the single-frame PCI/length and pass its UDS payload to PduR. */
    size_t uds_length;

    if (data == NULL || data_length != DEMO_CAN_FRAME_LENGTH)
    {
        puts("[CanTp] Dropped malformed CAN frame.");
        return;
    }

    /* This learning project intentionally implements ISO-TP single frames only. */
    if ((data[0] & 0xF0U) != 0x00U)
    {
        puts("[CanTp] Dropped unsupported ISO-TP frame (single frames only).");
        return;
    }

    uds_length = (size_t)(data[0] & 0x0FU);
    if (uds_length == 0U || uds_length > DEMO_MAX_UDS_LENGTH)
    {
        puts("[CanTp] Dropped invalid single-frame payload length.");
        return;
    }

    printf("[CanTp] RX UDS | Data: ");
    CanTp_PrintBytes(&data[1], uds_length);
    putchar('\n');
    // Pass the UDS payload to the PDU Router for further processing by DCM. The first byte of the CAN frame is the PCI byte, so we pass the data starting from index 1.
    PduR_DcmRxIndication(&data[1], uds_length);
}

Std_ReturnType CanTp_Transmit(const uint8_t *uds, size_t uds_length)
{
    /* Add the single-frame PCI byte to a response and transmit it via CanIf. */
    uint8_t frame[DEMO_CAN_FRAME_LENGTH];

    if (uds == NULL || uds_length == 0U || uds_length > DEMO_MAX_UDS_LENGTH)
    {
        puts("[CanTp] Cannot transmit: response does not fit a single frame.");
        return E_NOT_OK;
    }

    (void)memset(frame, 0x00, sizeof(frame));
    frame[0] = (uint8_t)uds_length;
    (void)memcpy(&frame[1], uds, uds_length);

    printf("[CanTp] TX CAN | Data: ");
    CanTp_PrintBytes(frame, sizeof(frame));
    putchar('\n');
    return CanIf_Transmit(frame, sizeof(frame));
}
