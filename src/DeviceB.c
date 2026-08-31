/************************************************************************/
/*                              Includes                                */
/************************************************************************/

#include "DeviceB.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "Common.h"
#include "task.h"

/************************************************************************/
/*                              Defines                                 */
/************************************************************************/
#define DEVICE_B_TRANSITION_PERIOD_MS    2000U
#define DEVICE_B_RANDOM_SEED             25U
#define DEVICE_B_SLEEP_TO_ACTIVE_MAX     40U
#define DEVICE_B_SLEEP_TO_FAULT_MIN      90U
#define DEVICE_B_ACTIVE_TO_SLEEP_MAX     30U
#define DEVICE_B_ACTIVE_TO_FAULT_MIN     80U
#define DEVICE_B_FAULT_TO_SLEEP_MAX      20U
#define DEVICE_B_FAULT_TO_ACTIVE_MAX     40U

/************************************************************************/
/*                   Static Variables Initialization                    */
/************************************************************************/
static DeviceBState_t currentState = DEVICE_B_STATE_SLEEP;


/************************************************************************/
/*                   Private Functions Declarations                     */
/************************************************************************/

/* Function which publishes the current DeviceB state to the event queue */
static void DeviceB_PublishState( void );

/* Function which handles DeviceB SLEEP state */
static void DeviceB_HandleSleepState( uint32_t randomRoll );

/* Function which handles DeviceB ACTIVE state */
static void DeviceB_HandleActiveState( uint32_t randomRoll );

/* Function which handles DeviceB FAULT state */
static void DeviceB_HandleFaultState( uint32_t randomRoll );

/* Function which handles invalid state transition */
static void DeviceB_HandleInvalidState( void );

/* Function to evaluate recovery condition based on rabdom number */
static void DeviceB_RecoveryEvaluation( uint32_t randomRoll);

/* Function which handles DeviceB state transition */
static void DeviceB_EvaluateState( void );

/* DeviceB task function */
static void DeviceB_Task( void * parameters );

/************************************************************************/
/*                   Private Functions Implementation                   */
/************************************************************************/

/* DeviceB_PublishState */
static void DeviceB_PublishState( void )
{
    /* Overwrite the event queue with the current state */
    if( xQueueOverwrite( Common_GetDeviceEventQueue(),
                         &currentState ) != pdPASS )
    {
        printf( "[LOG_ERROR] [DeviceB]: Unable to queue DeviceB state.\r\n" );
    }
}

/* DeviceB_HandleSleepState */
static void DeviceB_HandleSleepState( uint32_t randomRoll )
{
    /* Check the random roll and evaluate the next state */
    if( randomRoll < DEVICE_B_SLEEP_TO_ACTIVE_MAX )
    {
        /* Set DeviceB current state to ACTIVE */
        currentState = DEVICE_B_STATE_ACTIVE;
        /* Publish the new DeviceB state */
        DeviceB_PublishState();
        printf( "[LOG_INFO] [DeviceB]: Entering ACTIVE state. Event queued.\r\n" );
    }
    else if( randomRoll >= DEVICE_B_SLEEP_TO_FAULT_MIN )
    {
        /* Set DeviceB current state to FAULT */
        currentState = DEVICE_B_STATE_FAULT;
        /* Publish the new DeviceB state */
        DeviceB_PublishState();
        printf( "[LOG_INFO] [DeviceB]: Entering FAULT state. Event queued.\r\n" );
    }
    else
    {
        printf( "[LOG_INFO] [DeviceB]: Current state SLEEP.\r\n" );
    }
}

/* DeviceB_HandleActiveState */
static void DeviceB_HandleActiveState( uint32_t randomRoll )
{
    /* Check the random roll and evaluate the next state */
    if( randomRoll < DEVICE_B_ACTIVE_TO_SLEEP_MAX )
    {
        /* Set DeviceB current state to SLEEP */
        currentState = DEVICE_B_STATE_SLEEP;
        /* Publish the new DeviceB state */
        DeviceB_PublishState();
        printf( "[LOG_INFO] [DeviceB]: Entering SLEEP state. Event queued.\r\n" );
    }
    else if( randomRoll >= DEVICE_B_ACTIVE_TO_FAULT_MIN )
    {
        /* Set DeviceB current state to FAULT */
        currentState = DEVICE_B_STATE_FAULT;
        /* Publish the new DeviceB state */
        DeviceB_PublishState();
        printf( "[LOG_INFO] [DeviceB]: Entering FAULT state. Event queued.\r\n" );
    }
    else
    {
        printf( "[LOG_INFO] [DeviceB]: Current state ACTIVE.\r\n" );
    }
}

/* DeviceB_HandleFaultState */
static void DeviceB_HandleFaultState( uint32_t randomRoll )
{
    /* Store the command received from DeviceA */
    DeviceBCommand_t command;

    /* Wait for a reset command from DeviceA until the recovery evaluation 
     with random number */
    if( xQueueReceive( Common_GetDeviceBResetQueue(),
                       &command,
                       pdMS_TO_TICKS( DEVICE_B_TRANSITION_PERIOD_MS ) ) == pdPASS )
    {
        if( command != DEVICE_B_COMMAND_RESET )
        {
            printf( "[LOG_ERROR] [DeviceB]: Invalid reset command.\r\n" );
        }
        else
        {
            printf( "[LOG_INFO] [DeviceB]: Reset command received.\r\n" );

            /* Set DeviceB current state to SLEEP */
            currentState = DEVICE_B_STATE_SLEEP;
            /* Publish the new DeviceB state */
            DeviceB_PublishState();
            printf( "[LOG_INFO] [DeviceB]: Current state SLEEP. Event queued.\r\n" );
        }
    }
    else
    {
        /* Try recovery */
        DeviceB_RecoveryEvaluation(randomRoll);
    }
}

/* DeviceB_HandleInvalidState */
static void DeviceB_HandleInvalidState( void )
{
    printf( "[LOG_ERROR] [DeviceB]: Invalid state. Entering SLEEP state.\r\n" );

    /* Set DeviceB current state to SLEEP */
    currentState = DEVICE_B_STATE_SLEEP;
    /* Publish the new DeviceB state */
    DeviceB_PublishState();
    printf( "[LOG_INFO] [DeviceB]: Current state SLEEP. Event queued.\r\n" );
}

/* DeviceB_RecoveryEvaluation */
static void DeviceB_RecoveryEvaluation( uint32_t randomRoll)
{
    if( randomRoll < DEVICE_B_FAULT_TO_SLEEP_MAX )
    {
        /* Recover DeviceB to SLEEP */
        currentState = DEVICE_B_STATE_SLEEP;
        /* Publish the new DeviceB state */
        DeviceB_PublishState();
        printf( "[LOG_INFO] [DeviceB]: Recovered from FAULT. Entering SLEEP state. Event queued.\r\n" );
    }
    else if( randomRoll < DEVICE_B_FAULT_TO_ACTIVE_MAX )
    {
        /* Recover DeviceB to ACTIVE */
        currentState = DEVICE_B_STATE_ACTIVE;
        /* Publish the new DeviceB state */
        DeviceB_PublishState();
        printf( "[LOG_INFO] [DeviceB]: Recovered from FAULT. Entering ACTIVE state. Event queued.\r\n" );
    }
    else
    {
        printf( "[LOG_INFO] [DeviceB]: Unable to recover from FAULT.\r\n" );
    }
}

/* DeviceB_EvaluateState */
static void DeviceB_EvaluateState( void )
{
    /* Store the generated random roll */
    uint32_t randomRoll = 0U;

    /* Check the current state */
    if( ( currentState == DEVICE_B_STATE_SLEEP ) ||
        ( currentState == DEVICE_B_STATE_ACTIVE ) )
    {
        /* Delay the task if not in FAULT state */
        vTaskDelay( pdMS_TO_TICKS( DEVICE_B_TRANSITION_PERIOD_MS ) );
    }

    /* Get random number */
    randomRoll = ( uint32_t ) ( rand() % 100 );

    /* Evaluate DeviceB current state */
    switch( currentState )
    {
        case DEVICE_B_STATE_SLEEP:
            /* Handle SLEEP state */
            DeviceB_HandleSleepState( randomRoll );
            break;

        case DEVICE_B_STATE_ACTIVE:
            /* Handle ACTIVE state */
            DeviceB_HandleActiveState( randomRoll );
            break;

        case DEVICE_B_STATE_FAULT:
            /* Handle FAULT state */
            DeviceB_HandleFaultState( randomRoll );
            break;

        default:
            /* Handle DeviceB invalid state */
            DeviceB_HandleInvalidState();
            break;
    }
}

/* DeviceB_Task */
static void DeviceB_Task( void * parameters )
{
    ( void ) parameters;

    /* Set the seed */
    srand( DEVICE_B_RANDOM_SEED );
    printf( "[LOG_INFO] [DeviceB]: Starting.\r\n" );

    /* Start the infinite loop */
    for( ; ; )
    {
        /* Evaluate DeviceB current state */
        DeviceB_EvaluateState();
    }
}

/************************************************************************/
/*                   Public Functions Implementation                    */
/************************************************************************/

/* DeviceB_Init */
BaseType_t DeviceB_Init( void )
{
    /* Init DeviceB current state value */
    currentState = DEVICE_B_STATE_SLEEP;

    /* Create DeviceB state machine task */
    return xTaskCreate( DeviceB_Task,
                        "DeviceB",
                        configMINIMAL_STACK_SIZE,
                        NULL,
                        tskIDLE_PRIORITY + 1U,
                        NULL );
}
