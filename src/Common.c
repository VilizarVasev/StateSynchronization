/************************************************************************/
/*                              Includes                                */
/************************************************************************/

#include "Common.h"

/************************************************************************/
/*                              Defines                                 */
/************************************************************************/

#define DEVICE_EVENT_QUEUE_LENGTH      1U
#define DEVICE_B_RESET_QUEUE_LENGTH    1U

/************************************************************************/
/*                   Static Variables Initialization                    */
/************************************************************************/

/* Handle of the event queue */
static QueueHandle_t deviceEventQueue = NULL;

/* Handle of the reset queue */
static QueueHandle_t deviceBResetQueue = NULL;

/************************************************************************/
/*                   Private Functions Declarations                     */
/************************************************************************/


/************************************************************************/
/*                   Private Functions Implementation                   */
/************************************************************************/



/************************************************************************/
/*                   Public Functions Implementation                    */
/************************************************************************/

/* Common_Init */
BaseType_t Common_Init( void )
{
    /* Create event queue for DeviceA notification */
    deviceEventQueue = xQueueCreate( DEVICE_EVENT_QUEUE_LENGTH,
                                     sizeof( DeviceBState_t ) );

    /* Check if the queue was created successfully */
    if( deviceEventQueue == NULL )
    {
        return pdFAIL;
    }

    /* Create queue for reset command when DeviceB stuck in FAULT state*/
    deviceBResetQueue = xQueueCreate(DEVICE_B_RESET_QUEUE_LENGTH,
                                     sizeof( DeviceBCommand_t ) );

    /* Check if the queue was created successfully */
    if( deviceBResetQueue == NULL )
    {
        return pdFAIL;
    }

    return pdPASS;
}

/* Common_GetDeviceEventQueue */
QueueHandle_t Common_GetDeviceEventQueue( void )
{
    return deviceEventQueue;
}

/* Common_GetDeviceBResetQueue */
QueueHandle_t Common_GetDeviceBResetQueue( void )
{
    return deviceBResetQueue;
}
