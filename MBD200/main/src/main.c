/*******************************************************************************
  Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This file contains the "main" function for a project.

  Description:
    This file contains the "main" function for a project.  The
    "main" function calls the "SYS_Initialize" function to initialize the state
    machines of all modules in the system
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes


// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************
static void HMI_Force_Demo_Tasks(void) {
    static uint32_t lastTick = 0;
    uint32_t curTick = SYS_TMR_TickCountGet();
    uint32_t tickPerSec = SYS_TMR_TickCounterFrequencyGet();

    if (curTick - lastTick >= (tickPerSec * 1)) {
        lastTick = curTick;

        HMIDwin_TriggerSend(HMI_TAG_NETWORK_SIGNAL);

        HMIDwin_TriggerSend(HMI_TAG_DATETIME);

    }
}

int main ( void )
{
    /* Initialize all modules */
    SYS_Initialize ( NULL );
    HMIDwin_Initialize();
    while ( true )
    {
        /* Maintain state machines of all polled MPLAB Harmony modules. */
        SYS_Tasks ( );
        HMIDwin_Tasks();
        HMI_Force_Demo_Tasks();

    }

    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE );
}


/*******************************************************************************
 End of File
*/

