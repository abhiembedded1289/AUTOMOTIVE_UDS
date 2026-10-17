#ifndef PROJECT_TYPES_H
#define PROJECT_TYPES_H

/* Shared primitive types and frame-size limits used by every simulator layer. */
#include <stddef.h>
#include <stdint.h>

/* Project-local AUTOSAR-style success/failure return type and values. */
typedef uint8_t Std_ReturnType;

#define E_OK       ((Std_ReturnType)0U)
#define E_NOT_OK   ((Std_ReturnType)1U)

/* The simulator models a classic CAN frame and ISO-TP single frames only. */
#define DEMO_CAN_FRAME_LENGTH 8U
#define DEMO_MAX_UDS_LENGTH   7U

#endif
