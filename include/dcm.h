#ifndef DCM_H
#define DCM_H

/* Public interface for the simplified Diagnostic Communication Manager. */
#include "project_types.h"

/* Reset the simulator's diagnostic session and security state to defaults. */
void Dcm_Init(void);

/* Dispatch a complete UDS request and send its positive or negative response. */
void Dcm_ProcessRequest(const uint8_t *request, size_t request_length);

#endif
