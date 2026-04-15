/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It
    implements the logic of the application's state machine and it may call
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "app.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_Initialize function.

    Application strings and buffers are be defined outside this structure.
 */

APP_DATA appData;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary callback functions.
 */

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************


/* TODO:  Add any necessary local functions.
 */


// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_Initialize ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Initialize(void) {
    /* Place the App state machine in its initial state. */
    appData.state = APP_STATE_INIT;



    /* TODO: Initialize your application's state machine and other
     * parameters.
     */
}

/******************************************************************************
  Function:
    void APP_Tasks ( void )

  Remarks:
    See prototype in app.h.
 */
static void HMI_Test_Init(void) {
    for(uint8_t i = 0; i < 8; i++) {
        // ?ánh d?u dòng i là ?ang ho?t ??ng (Enable)
        gSensorCfg.entry[i].enable = true;
        gSensorCfg.entry[i].type = SENSOR_ANALOG;
        gSensorCfg.entry[i].indexOfType = i;
        
//         Map dòng i c?a màn hình vào ?úng sensor i
        gAppCfg.hmi[i] = i; 

        // Gán tên và ??n v? m?u (?? hi?n luôn tên/??n v?)
        snprintf(gAnalogCfg.entry[i].name, 16, "Sen %d", i+1);
        snprintf(gAnalogCfg.entry[i].unit, 8, "U");
        
       
    }

}
void APP_Tasks(void) {

    /* Check the application's current state. */
    switch (appData.state) {
            /* Application's initial state. */
        case APP_STATE_INIT:
        {
            bool appInitialized = true;
            Rtc_Initialize();
            SIMMain_Initialize();
            SDcard_Initialize();
            BootConfig_Initialize();
            ExtFlash_Initialize();
            Fram_Initialize();

            if (appInitialized) {

                appData.state = APP_STATE_BOOT_CONFIG;
            }
            break;
        }

        case APP_STATE_BOOT_CONFIG:
        {
            ExtFlash_Task();
            if (BootConfig_Task())
                appData.state = APP_STATE_SERVICE_TASKS;
            
            HMI_Test_Init();
            break;
        }

        case APP_STATE_SERVICE_TASKS:
        {
            Rtc_Task();
            SIMMain_Task();
            SDcard_Task();
            ExtFlash_Task();
            Fram_Task();
            break;
        }

            /* TODO: implement your application state machine.*/


            /* The default state should never be executed. */
        default:
        {
            /* TODO: Handle error in application's state machine. */
            break;
        }
    }
}


/*******************************************************************************
 End of File
 */
