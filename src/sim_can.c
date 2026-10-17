/* Virtual CAN endpoint: adds/removes visible framing and prints bus traffic.
 * No physical CAN controller or external device is used. */
#include "can_if.h"
#include "sim_can.h"

#include <stdio.h>
#include <string.h>

#define SIM_CAN_REQUEST_ID  0x7E0U

static void PrintBytes(const uint8_t *data, size_t length)
{
    /* Print a byte buffer in the hexadecimal form used by diagnostic traces. */
    size_t index;

    for (index = 0U; index < length; index++)
    {
        printf("%02X", data[index]);
        if (index + 1U < length)
        {
            putchar(' ');
        }
    }
}

Std_ReturnType SimCan_SendUdsRequest(const uint8_t *uds, size_t uds_length)
{
    /* Build a virtual tester CAN frame and deliver it into the CanIf receive path. */
    uint8_t frame[DEMO_CAN_FRAME_LENGTH];

    if (uds == NULL || uds_length == 0U || uds_length > DEMO_MAX_UDS_LENGTH)
    {
        return E_NOT_OK;
    }

    /* Byte 0 is the ISO-TP single-frame length; unused CAN bytes are zero padded. */
    (void)memset(frame, 0x00, sizeof(frame));
    //in can frame first byte is the length of the uds data, so we set it to the uds_length
    frame[0] = (uint8_t)uds_length;
    // length +data is copied into the frame starting at index 1
    (void)memcpy(&frame[1], uds, uds_length);

    printf("\n[SimCan] TX | CAN ID: 0x%03X | Data: ", SIM_CAN_REQUEST_ID);
    PrintBytes(frame, sizeof(frame));
    putchar('\n');

    CanIf_RxIndication(SIM_CAN_REQUEST_ID, frame, sizeof(frame));
    return E_OK;
}

void SimCan_OnCanTransmit(uint32_t can_id,
                          const uint8_t *data,
                          size_t data_length)
{
    /* Show the transmitted CAN frame and, when valid, its UDS response bytes. */
    size_t uds_length;

    printf("[SimCan] TX | CAN ID: 0x%03X | Data: ", (unsigned int)can_id);
    PrintBytes(data, data_length);
    putchar('\n');

    if (data_length == DEMO_CAN_FRAME_LENGTH &&
        (data[0] & 0xF0U) == 0x00U)
    {
        uds_length = (size_t)(data[0] & 0x0FU);
        if (uds_length > 0U && uds_length <= DEMO_MAX_UDS_LENGTH)
        {
        }
    }
}
