#ifndef DEVICE_A_H
#define DEVICE_A_H

/************************************************************************/
/*                              Includes                                */
/************************************************************************/

#include "FreeRTOS.h"

/************************************************************************/
/*                              Type Definitions                        */
/************************************************************************/

/* DeviceA states */
typedef enum
{
    DEVICE_A_STATE_IDLE = 0,
    DEVICE_A_STATE_PROCESSING,
    DEVICE_A_STATE_ERROR
} DeviceAState_t;

/************************************************************************/
/*                              Public Functions                        */
/************************************************************************/

/* Function to initialize DeviceA */
BaseType_t DeviceA_Init( void );

#endif /* DEVICE_A_H */
