#ifndef STATE_SYNC_COMMON_H
#define STATE_SYNC_COMMON_H

/************************************************************************/
/*                              Includes                                */
/************************************************************************/

#include "FreeRTOS.h"
#include "queue.h"

/************************************************************************/
/*                              Type Definitions                        */
/************************************************************************/

/* DeviceB states */
typedef enum
{
    DEVICE_B_STATE_SLEEP = 0,
    DEVICE_B_STATE_ACTIVE,
    DEVICE_B_STATE_FAULT
} DeviceBState_t;

/* Command for resetting DeviceB when stuck in FAULT state */
typedef enum
{
    DEVICE_B_COMMAND_RESET = 1
} DeviceBCommand_t;


/************************************************************************/
/*                              Public Functions                        */
/************************************************************************/

/* Function to initialize the Common component */
BaseType_t Common_Init( void );

/* Function to provide access to the event queue used */
/* by DeviceA and DeviceB */ 
QueueHandle_t Common_GetDeviceEventQueue( void );

/* Function to provide access to the reset queue used */
/* by DeviceA and DeviceB */
QueueHandle_t Common_GetDeviceBResetQueue( void );

#endif /* STATE_SYNC_COMMON_H */
