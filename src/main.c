/************************************************************************/
/*                              Includes                                */
/************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "Common.h"
#include "DeviceA.h"
#include "DeviceB.h"

/************************************************************************/
/*                      Application Main Function                       */
/************************************************************************/

/* main */
int main( void )
{
    /* Disable standard output buffering */
    setvbuf( stdout, NULL, _IONBF, 0 );
    printf( "[LOG_INFO] [Main]: Starting the DeviceA/DeviceB FreeRTOS simulation.\r\n" );

    /* Initialize the queues for events and reset command */
    if( Common_Init() != pdPASS )
    {
        printf("[LOG_ERROR] [System]: Could not create the event and reset queues.\r\n" );
        return EXIT_FAILURE;
    }

    /* Initialize DeviceA */
    if( DeviceA_Init() != pdPASS )
    {
        printf("[LOG_ERROR] [Main]: Could not create the DeviceA task.\r\n" );
        return EXIT_FAILURE;
    }

    /* Initialize DeviceB */
    if( DeviceB_Init() != pdPASS )
    {
        printf("[LOG_ERROR] [Main]: Could not create the DeviceB task.\r\n" );
        return EXIT_FAILURE;
    }

    /* Start FreeRTOS scheduler */
    vTaskStartScheduler();

    printf("[LOG_ERROR] [Main]: The FreeRTOS scheduler stopped unexpectedly.\r\n" );
    return EXIT_FAILURE;
}

/************************************************************************/
/*                  FreeRTOS Hooks Implementation                       */
/************************************************************************/

/* vApplicationMallocFailedHook */
void vApplicationMallocFailedHook( void )
{
    /* Log the allocation failure and terminate the simulation */
    printf("[LOG_ERROR] [FreeRTOS]: Heap allocation failed.\r\n" );
    abort();
}

/* vApplicationStackOverflowHook */
void vApplicationStackOverflowHook( TaskHandle_t task, char * taskName )
{
    ( void ) task;
    ( void ) taskName;

    /* Log the stack overflow and terminate the simulation */
    printf("[LOG_ERROR] [FreeRTOS]: Stack overflow detected.\r\n" );
    abort();
}

/* vAssertCalled */
void vAssertCalled( const char * file, int line )
{
    ( void ) file;
    ( void ) line;

    /* Log the assertion failure and terminate the simulation */
    printf("[LOG_ERROR] [FreeRTOS]: Assertion failed.\r\n" );
    abort();
}
