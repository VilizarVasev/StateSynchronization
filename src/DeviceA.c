/************************************************************************/
/*                              Includes                                */
/************************************************************************/
#include "DeviceA.h"

#include <stdio.h>
#include <stdint.h>

#include "Common.h"
#include "task.h"

/************************************************************************/
/*                              Defines                                 */
/************************************************************************/
#define DEVICE_A_EVALUATION_PERIOD_MS       1000U
#define DEVICE_B_FAULT_COUNTER_MAX          3u

/************************************************************************/
/*                   Static Variables Initialization                    */
/************************************************************************/
static DeviceAState_t currentState = DEVICE_A_STATE_IDLE;
static DeviceBState_t deviceBState = DEVICE_B_STATE_SLEEP;
static uint8_t faultCounter = 0u;


/************************************************************************/
/*                   Private Functions Declarations                     */
/************************************************************************/

/* Function which sends reset request to the reset queue */
static void DeviceA_RequestDeviceBReset( void );

/* Function which handles DeviceA IDLE state */
static void DeviceA_HandleIdleState( void );

/* Function which handles DeviceA PROCESSING state */
static void DeviceA_HandleProcessingState( void );

/* Function which handles DeviceA ERROR state */
static void DeviceA_HandleErrorState( void );

/* Funcion which handles invalid state transition */
static void DeviceA_HandleInvalidState( void );

/* Function which handles DeviceA state transition */
static void DeviceA_EvaluateState( void );

/* DeviceA task function */
static void DeviceA_Task( void * parameters );

/************************************************************************/
/*                   Private Functions Implementation                   */
/************************************************************************/

/* DeviceA_RequestDeviceBReset */
static void DeviceA_RequestDeviceBReset( void )
{
    /* Prepare reset command */
    DeviceBCommand_t command = DEVICE_B_COMMAND_RESET;

    /* Send the reset command to the reset queue */
    if( xQueueOverwrite( Common_GetDeviceBResetQueue(), &command ) != pdPASS )
    {
        printf( "[LOG_ERROR] [DeviceA]: Unable to queue DeviceB reset command.\r\n" );
    }
}

/* DeviceA_HandleIdleState */
static void DeviceA_HandleIdleState( void )
{
    /* Check DeviceB current state*/
    if( deviceBState == DEVICE_B_STATE_ACTIVE )
    {
        printf( "[LOG_INFO] [DeviceA]: DeviceB is ACTIVE. Entering PROCESSING state.\r\n" );
        /* Set DeviceA current state to PROCESSING*/
        currentState = DEVICE_A_STATE_PROCESSING;
    }
    else if( deviceBState == DEVICE_B_STATE_FAULT )
    {
        printf( "[LOG_ERROR] [DeviceA]: DeviceB fault detected.\r\n" );
        printf( "[LOG_INFO] [DeviceA]: Entering ERROR state.\r\n" );
        /* Set DeviceA current state to ERROR */
        currentState = DEVICE_A_STATE_ERROR;
    }
    else if( deviceBState == DEVICE_B_STATE_SLEEP )
    {
        printf( "[LOG_INFO] [DeviceA]: DeviceB in SLEEP state.\r\n" );
    }
    else
    {
        printf( "[LOG_ERROR] [DeviceA]: Invalid DeviceB state.\r\n" );
        /* Reset DeiveB */
        DeviceA_RequestDeviceBReset();
    }
}

/* DeviceA_HandleProcessingState */
static void DeviceA_HandleProcessingState( void )
{
    /* Check DeviceB current state*/
    if( deviceBState == DEVICE_B_STATE_SLEEP )
    {
        printf( "[LOG_INFO] [DeviceA]: DeviceB in SLEEP state. Entering IDLE state.\r\n" );
        /* Set DeviceA current state to IDLE */
        currentState = DEVICE_A_STATE_IDLE;
    }
    else if( deviceBState == DEVICE_B_STATE_FAULT )
    {
        printf( "[LOG_ERROR] [DeviceA]: DeviceB fault detected.\r\n" );
        printf( "[LOG_INFO] [DeviceA]: Entering ERROR state.\r\n" );
        /* Set DeviceA current state to ERROR */
        currentState = DEVICE_A_STATE_ERROR;
    }
    else if( deviceBState == DEVICE_B_STATE_ACTIVE )
    {
        printf( "[LOG_INFO] [DeviceA]: DeviceB in ACTIVE state.\r\n" );
    }
    else
    {
        printf( "[LOG_ERROR] [DeviceA]: Invalid DeviceB state.\r\n" );
        /* Reset DeviceB */
        DeviceA_RequestDeviceBReset();
    }
}

/* DeviceA_HandleErrorState */
static void DeviceA_HandleErrorState( void )
{
    /* Check DeviceB current state*/
    if( deviceBState == DEVICE_B_STATE_SLEEP )
    {
        printf( "[LOG_INFO] [DeviceA]: DeviceB recovered from fault. Entering IDLE state.\r\n" );
        /* Set DeviceA current state to IDLE */
        currentState = DEVICE_A_STATE_IDLE;
        /* Reset the fault counter */
        faultCounter = 0U;
    }
    else if( deviceBState == DEVICE_B_STATE_ACTIVE )
    {
        printf( "[LOG_INFO] [DeviceA]: DeviceB recovered from fault. Entering PROCESSING state.\r\n" );
        /* Set the current state to PROCESSING */
        currentState = DEVICE_A_STATE_PROCESSING;
        /* Reset the fault counter */
        faultCounter = 0U;
    }
    else if( deviceBState == DEVICE_B_STATE_FAULT )
    {
        /* Increment the fault counter */
        faultCounter++;
        printf("[LOG_INFO] [DeviceA]: Fault count: %d \r\n", faultCounter);

        /* Check if the fault counter reached its max value */
        if( faultCounter >= DEVICE_B_FAULT_COUNTER_MAX )
        {
            printf( "[LOG_ERROR] [DeviceA]: Persistent DeviceB fault. Reset command sent.\r\n" );
            /* Reset DeviceB */
            DeviceA_RequestDeviceBReset();
            /* Reset the fault counter */
            faultCounter = 0U;
        }
    }
    else
    {
        printf( "[LOG_ERROR] [DeviceA]: Invalid DeviceB state.\r\n" );
        /* Reset DeviceB */
        DeviceA_RequestDeviceBReset();
        /* Reset the fault counter */
        faultCounter = 0U;
    }
}

/* DeviceA_HandleInvalidState */
static void DeviceA_HandleInvalidState( void )
{
    printf( "[LOG_ERROR] [DeviceA]: Invalid current state. Returning to IDLE state.\r\n" );
    /* Set current state to idle */
    currentState = DEVICE_A_STATE_IDLE;
}

/* DeviceA_EvaluateState */
static void DeviceA_EvaluateState( void )
{
    /* Wait for event from DeviceB */
    ( void ) xQueueReceive( Common_GetDeviceEventQueue(),
                           &deviceBState,
                           pdMS_TO_TICKS( DEVICE_A_EVALUATION_PERIOD_MS ) );

    /* Evaluate DeviceA current state */
    switch( currentState )
    {
        case DEVICE_A_STATE_IDLE:
            /* Handle IDLE state */
            DeviceA_HandleIdleState();
            break;

        case DEVICE_A_STATE_PROCESSING:
            /* Handle PROCESSING state */
            DeviceA_HandleProcessingState();
            break;

        case DEVICE_A_STATE_ERROR:
            /* Handle ERROR state */
            DeviceA_HandleErrorState();
            break;

        default:
            /* Handle DeviceA invalid state */
            DeviceA_HandleInvalidState();
            break;
    }
}

/* DeviceA_Task */
static void DeviceA_Task( void * parameters )
{
    ( void ) parameters;

    printf( "[LOG_INFO] [DeviceA]: Starting.\r\n" );
    /* Start the infinite loop */
    for( ; ; )
    {
        /* Evaluate DeviceA current state */
        DeviceA_EvaluateState();
    }
}

/************************************************************************/
/*                   Public Functions Implementation                    */
/************************************************************************/

/* DeviceA_Init */
BaseType_t DeviceA_Init( void )
{
    /* Init the DeviceA and DeviceB current states values */
    currentState = DEVICE_A_STATE_IDLE;
    deviceBState = DEVICE_B_STATE_SLEEP;
    /* Reset the fault counter */
    faultCounter = 0U;

    /* Create DeviceA state machine task */
    return xTaskCreate( DeviceA_Task,
                        "DeviceA",
                        configMINIMAL_STACK_SIZE,
                        NULL,
                        tskIDLE_PRIORITY + 2U,
                        NULL );
}
