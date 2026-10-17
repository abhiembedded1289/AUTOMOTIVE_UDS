/* Demo application layer: stores sample DID values behind an RTE-like API. */
#include "rte_demo_app.h"

/* Mutable/read-only data stands in for signals provided by application software. */
static uint8_t g_vehicle_speed[2] = { 0x00U, 0x14U };
static const uint8_t g_engine_speed[2] = { 0x0BU, 0xB8U };

Std_ReturnType Rte_DemoApp_ReadDid(uint16_t did, uint8_t value[2])
{
    /* Copy a supported DID's current two-byte value to the DCM's output buffer. */
    if (value == NULL)
    {
        return E_NOT_OK;
    }

    if (did == 0xF1A0U)
    {
        value[0] = g_vehicle_speed[0];
        value[1] = g_vehicle_speed[1];
        return E_OK;
    }
    if (did == 0xF1A1U)
    {
        value[0] = g_engine_speed[0];
        value[1] = g_engine_speed[1];
        return E_OK;
    }

    return E_NOT_OK;
}

Std_ReturnType Rte_DemoApp_WriteDid(uint16_t did, const uint8_t value[2])
{
    /* Update the one writable demo DID; other IDs are read-only/unsupported. */
    if (value == NULL || did != 0xF1A0U)
    {
        return E_NOT_OK;
    }

    g_vehicle_speed[0] = value[0];
    g_vehicle_speed[1] = value[1];
    return E_OK;
}
