#ifndef _APP_H
#define _APP_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "definitions.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif

    typedef enum {
        APP_INIT_MODULE = 0,
        APP_BOOT_CONFIG,
        APP_MOUNT_DISK,
        APP_LOAD_DEVICE_INFO,
        APP_SAVE_DEVICE_INFO,
        APP_TCPIP_INIT,
        APP_TCPIP_WAIT_INIT,
        APP_TCPIP_TRANSACT,
        APP_TCPIP_ERROR
    } APP_STATES;


    void App_Initialize(void);
    void App_Tasks(void);

    //DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* _APP_H */

/*******************************************************************************
 End of File
 */

