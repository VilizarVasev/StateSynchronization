#include <stdio.h>
#include <stdlib.h>

#include "Common.h"
#include "DeviceA.h"
#include "DeviceB.h"


int main( void )
{
    setvbuf( stdout, NULL, _IONBF, 0 );
    printf( "[LOG_INFO] [Main]: Starting the DeviceA/DeviceB FreeRTOS simulation.\r\n" );

    if( Common_Init() != pdPASS )
    {
        printf("[LOG_ERROR] [System]: Could not create the event and reset queues.\r\n" );
        return EXIT_FAILURE;
    }

    if( DeviceA_Init() != pdPASS )
    {
        printf("[LOG_ERROR] [Main]: Could not create the DeviceA task.\r\n" );
        return EXIT_FAILURE;
    }

    if( DeviceB_Init() != pdPASS )
    {
        printf("[LOG_ERROR] [Main]: Could not create the DeviceB task.\r\n" );
        return EXIT_FAILURE;
    }

    vTaskStartScheduler();

    printf("[LOG_ERROR] [Main]: The FreeRTOS scheduler stopped unexpectedly.\r\n" );
    return EXIT_FAILURE;
}

void vApplicationMallocFailedHook( void )
{
    printf("[LOG_ERROR] [FreeRTOS]: Heap allocation failed.\r\n" );
    abort();
}

void vApplicationStackOverflowHook( TaskHandle_t task, char * taskName )
{
    ( void ) task;
    ( void ) taskName;
    printf("[LOG_ERROR] [FreeRTOS]: Stack overflow detected.\r\n" );
    abort();
}

void vAssertCalled( const char * file, int line )
{
    ( void ) file;
    ( void ) line;
    printf("[LOG_ERROR] [FreeRTOS]: Assertion failed.\r\n" );
    abort();
}
