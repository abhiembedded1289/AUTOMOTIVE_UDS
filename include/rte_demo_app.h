#ifndef RTE_DEMO_APP_H
#define RTE_DEMO_APP_H

/* Demo application data interface used by DCM as an RTE-like boundary. */
#include "project_types.h"

/* Read a supported two-byte demo DID into the caller's output buffer. */
Std_ReturnType Rte_DemoApp_ReadDid(uint16_t did, uint8_t value[2]);

/* Write a supported writable two-byte demo DID from the supplied value. */
Std_ReturnType Rte_DemoApp_WriteDid(uint16_t did, const uint8_t value[2]);

#endif
