#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include "definitions.h"

int main(void) {
    SYS_Initialize(NULL);

    //    SIMGps_Initialize();    
    //
    //    uint32_t gpsTimer = 0;
    //    bool gpsRequested = false;
    //    static bool basicReadyPrinted = false;

    while (true) {
        SYS_Tasks();


        //        HMIDwin_Tasks();
        //
        //        SIMGps_Process();
        //
        //        uint32_t curTick = SYS_TMR_TickCountGet();
        //        uint32_t tickPerSec = SYS_TMR_TickCounterFrequencyGet();
        //
        //        if (SIMBasic_IsReady()) {
        //                if (!basicReadyPrinted) {
        //                SYS_CONSOLE_PRINT("\r\nGPS Init <<<\r\n");
        //                basicReadyPrinted = true;
        //            }
        //
        //            if (SIMGps_IsReady()) {
        //                if (curTick - gpsTimer >= (tickPerSec * 10)) {
        //                    gpsTimer = curTick;
        //                    if (SIMGps_UpdateLocation()) { 
        //                        gpsRequested = true;
        //                        SYS_CONSOLE_PRINT("\r\n[GPS] Updating.\r\n");
        //                    }
        //                }
        //
        //                if (gpsRequested) {
        //                    gpsRequested = false;
        //                    SIM_GPS_INFO* info = SIMGps_GetInfo(); 
        //
        //                    if (info->hasFix) {
        //                        SYS_CONSOLE_PRINT("[GPS SUCCESS] Location: %s\r\n", info->rawData);
        //                    } else {
        //                        SYS_CONSOLE_PRINT("[GPS WAIT] GPS FAIL\r\n");
        //                    }
        //                }
        //            }
        //        }
    }
    return (EXIT_FAILURE);
}