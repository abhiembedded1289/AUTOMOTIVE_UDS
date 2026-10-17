/* Simplified Diagnostic Communication Manager: dispatches UDS services,
 * enforces demo session/security rules, and requests response transmission. */
#include "dcm.h"

#include "ecum.h"
#include "pdu_router.h"
#include "rte_demo_app.h"

#include <stdio.h>

#define UDS_SID_SESSION_CONTROL 0x10U
#define UDS_SID_ECU_RESET       0x11U
#define UDS_SID_READ_DID        0x22U
#define UDS_SID_SECURITY_ACCESS 0x27U
#define UDS_SID_WRITE_DID       0x2EU
#define UDS_SID_TESTER_PRESENT  0x3EU
#define UDS_SID_NEGATIVE        0x7FU

#define NRC_SERVICE_NOT_SUPPORTED 0x11U
#define NRC_SUBFUNCTION_NOT_SUPPORTED 0x12U
#define NRC_INCORRECT_LENGTH 0x13U
#define NRC_CONDITIONS_NOT_CORRECT 0x22U
#define NRC_REQUEST_SEQUENCE_ERROR 0x24U
#define NRC_REQUEST_OUT_OF_RANGE 0x31U
#define NRC_INVALID_KEY 0x35U

#define SESSION_DEFAULT  0x01U
#define SESSION_EXTENDED 0x03U
#define SECURITY_SEED    0x1234U
#define SECURITY_XOR_KEY 0xA5A5U

static uint8_t g_session = SESSION_DEFAULT;
static uint8_t g_security_unlocked = 0U;
static uint8_t g_seed_requested = 0U;

static void Dcm_PrintBytes(const uint8_t *data, size_t data_length)
{
    size_t index;

    for (index = 0U; index < data_length; index++)
    {
        printf("%02X%s",
               data[index],
               (index + 1U < data_length) ? " " : "");
    }
}

void Dcm_Init(void)
{
    /* Start in the default session with security locked and no pending seed. */
    g_session = SESSION_DEFAULT;
    g_security_unlocked = 0U;
    g_seed_requested = 0U;
}

static void SendResponse(const uint8_t *response, size_t response_length)
{
    /* Ask PduR to route this positive response toward the tester. */
    printf("[DCM] TX | UDS: ");
    Dcm_PrintBytes(response, response_length);
    putchar('\n');
    if (PduR_DcmTransmit(response, response_length) != E_OK)
    {
        puts("[Dcm] Failed to transmit diagnostic response.");
    }
}

static void SendNegativeResponse(uint8_t request_sid, uint8_t nrc)
{
    /* Create the standard three-byte UDS negative response (7F, SID, NRC). */
    const uint8_t response[3] = { UDS_SID_NEGATIVE, request_sid, nrc };
    SendResponse(response, sizeof(response));
}

static uint16_t ReadDidFromRequest(const uint8_t *request)
{
    /* Combine the two big-endian DID bytes from a UDS request into one value. */
    return (uint16_t)(((uint16_t)request[1] << 8U) | request[2]);
}

static void HandleSessionControl(const uint8_t *request, size_t request_length)
{
    /* Validate and switch between supported sessions, clearing security state. */
    uint8_t response[2];

    if (request_length != 2U)
    {
        SendNegativeResponse(request[0], NRC_INCORRECT_LENGTH);
        return;
    }

    if (request[1] != SESSION_DEFAULT && request[1] != SESSION_EXTENDED)
    {
        SendNegativeResponse(request[0], NRC_SUBFUNCTION_NOT_SUPPORTED);
        return;
    }

    g_session = request[1];
    g_security_unlocked = 0U;
    g_seed_requested = 0U;
    response[0] = (uint8_t)(request[0] + 0x40U);
    response[1] = g_session;
    SendResponse(response, sizeof(response));
}

static void HandleEcuReset(const uint8_t *request, size_t request_length)
{
    /* Validate hard-reset subfunction, respond, then log a simulated reset. */
    const uint8_t response[2] = { 0x51U, 0x01U };

    if (request_length != 2U)
    {
        SendNegativeResponse(request[0], NRC_INCORRECT_LENGTH);
        return;
    }
    if (request[1] != 0x01U)
    {
        SendNegativeResponse(request[0], NRC_SUBFUNCTION_NOT_SUPPORTED);
        return;
    }

    /* Send the positive response before logging the simulated reset request. */
    SendResponse(response, sizeof(response));
    EcuM_RequestReset(request[1]);
    g_session = SESSION_DEFAULT;
    g_security_unlocked = 0U;
    g_seed_requested = 0U;
}

static void HandleReadDid(const uint8_t *request, size_t request_length)
{
    /* Read a supported DID through the demo RTE boundary and build a response. */
    uint8_t value[2];
    uint8_t response[5];
    
    uint16_t did;

    if (request_length != 3U)
    {
        SendNegativeResponse(request[0], NRC_INCORRECT_LENGTH);
        return;
    }

    did = ReadDidFromRequest(request);
    if (Rte_DemoApp_ReadDid(did, value) != E_OK)
    {
        SendNegativeResponse(request[0], NRC_REQUEST_OUT_OF_RANGE);
        return;
    }

    response[0] = (uint8_t)(request[0] + 0x40U);
    response[1] = request[1];
    response[2] = request[2];
    response[3] = value[0];
    response[4] = value[1];
    SendResponse(response, sizeof(response));
}

static void HandleSecurityAccess(const uint8_t *request, size_t request_length)
{
    /* Demonstrate request-seed/send-key flow with a fixed, intentionally weak key. */
    uint8_t response[4];
    uint16_t supplied_key;
    uint16_t expected_key = (uint16_t)(SECURITY_SEED ^ SECURITY_XOR_KEY);

    if (request_length < 2U)
    {
        SendNegativeResponse(request[0], NRC_INCORRECT_LENGTH);
        return;
    }

    if (request[1] == 0x01U)
    {
        if (request_length != 2U)
        {
            SendNegativeResponse(request[0], NRC_INCORRECT_LENGTH);
            return;
        }
        g_seed_requested = 1U;
        response[0] = 0x67U;
        response[1] = 0x01U;
        response[2] = (uint8_t)(SECURITY_SEED >> 8U);
        response[3] = (uint8_t)(SECURITY_SEED & 0x00FFU);
        SendResponse(response, sizeof(response));
        return;
    }

    if (request[1] == 0x02U)
    {
        if (request_length != 4U)
        {
            SendNegativeResponse(request[0], NRC_INCORRECT_LENGTH);
            return;
        }
        if (g_seed_requested == 0U)
        {
            SendNegativeResponse(request[0], NRC_REQUEST_SEQUENCE_ERROR);
            return;
        }

        supplied_key = (uint16_t)(((uint16_t)request[2] << 8U) | request[3]);
        g_seed_requested = 0U;
        if (supplied_key != expected_key)
        {
            SendNegativeResponse(request[0], NRC_INVALID_KEY);
            return;
        }

        g_security_unlocked = 1U;
        response[0] = 0x67U;
        response[1] = 0x02U;
        SendResponse(response, 2U);
        return;
    }

    SendNegativeResponse(request[0], NRC_SUBFUNCTION_NOT_SUPPORTED);
}

static void HandleWriteDid(const uint8_t *request, size_t request_length)
{
    /* Write the demo vehicle-speed DID only in extended session after unlocking. */
    uint16_t did;
    uint8_t value[2];
    uint8_t response[3];

    if (request_length != 5U)
    {
        SendNegativeResponse(request[0], NRC_INCORRECT_LENGTH);
        return;
    }

    did = ReadDidFromRequest(request);
    if (did != 0xF1A0U)
    {
        SendNegativeResponse(request[0], NRC_REQUEST_OUT_OF_RANGE);
        return;
    }
    if (g_session != SESSION_EXTENDED || g_security_unlocked == 0U)
    {
        SendNegativeResponse(request[0], NRC_CONDITIONS_NOT_CORRECT);
        return;
    }

    value[0] = request[3];
    value[1] = request[4];
    if (Rte_DemoApp_WriteDid(did, value) != E_OK)
    {
        SendNegativeResponse(request[0], NRC_CONDITIONS_NOT_CORRECT);
        return;
    }

    response[0] = (uint8_t)(request[0] + 0x40U);
    response[1] = request[1];
    response[2] = request[2];
    SendResponse(response, sizeof(response));
}

static void HandleTesterPresent(const uint8_t *request, size_t request_length)
{
    /* Validate tester keep-alive and honor the positive-response-suppression bit. */
    uint8_t response[2];
    uint8_t subfunction;

    if (request_length != 2U)
    {
        SendNegativeResponse(request[0], NRC_INCORRECT_LENGTH);
        return;
    }

    subfunction = (uint8_t)(request[1] & 0x7FU);
    if (subfunction != 0x00U)
    {
        SendNegativeResponse(request[0], NRC_SUBFUNCTION_NOT_SUPPORTED);
        return;
    }

    /* Bit 7 suppresses the positive response as defined for this service. */
    if ((request[1] & 0x80U) != 0U)
    {
        puts("[Dcm] TesterPresent positive response suppressed.");
        return;
    }

    response[0] = 0x7EU;
    response[1] = 0x00U;
    SendResponse(response, sizeof(response));
}

void Dcm_ProcessRequest(const uint8_t *request, size_t request_length)
{
    /* Validate a UDS PDU, dispatch by service identifier, and reject unknown SIDs. */
    if (request == NULL || request_length == 0U ||
        request_length > DEMO_MAX_UDS_LENGTH)
    {
        puts("[Dcm] Dropped invalid request PDU.");
        return;
    }

    if ((request[0] == UDS_SID_READ_DID ||
         request[0] == UDS_SID_WRITE_DID) &&
        request_length >= 3U)
    {
        printf("[DCM] RX | SID: 0x%02X | DID: 0x%04X\n",request[0],(unsigned int)ReadDidFromRequest(request));
    }
    else
    {
        printf("[DCM] RX | SID: 0x%02X\n", request[0]);
    }

    switch (request[0])
    {
        case UDS_SID_SESSION_CONTROL:
            HandleSessionControl(request, request_length);
            break;
        case UDS_SID_ECU_RESET:
            HandleEcuReset(request, request_length);
            break;
        case UDS_SID_READ_DID:
            HandleReadDid(request, request_length);
            break;
        case UDS_SID_SECURITY_ACCESS:
            HandleSecurityAccess(request, request_length);
            break;
        case UDS_SID_WRITE_DID:
            HandleWriteDid(request, request_length);
            break;
        case UDS_SID_TESTER_PRESENT:
            HandleTesterPresent(request, request_length);
            break;
        default:
            SendNegativeResponse(request[0], NRC_SERVICE_NOT_SUPPORTED);
            break;
    }
}
