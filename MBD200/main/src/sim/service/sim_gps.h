#ifndef SIM_GPS_H
#define	SIM_GPS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "definitions.h"

#ifdef	__cplusplus
extern "C" {
#endif

    typedef enum {
        SIM_GPS_IDLE = 0,
        SIM_GPS_CHECK_ON,
        SIM_GPS_TURN_ON,
        SIM_GPS_CHECK_SAT, 
        SIM_GPS_READ_LOC,
        SIM_GPS_READY,
        SIM_GPS_ERROR,
        SIM_GPS_COUNT
    } SIM_GPS_STATE;

    typedef struct {
        bool hasFix;
        char rawData[128];
    } SIM_GPS_INFO;

    void SIMGps_Initialize(void);
    void SIMGps_Process(void);
    bool SIMGps_IsReady(void);
    bool SIMGps_HasError(void);
    bool SIMGps_UpdateLocation(void);
    SIM_GPS_INFO* SIMGps_GetInfo(void);

#ifdef	__cplusplus
}
#endif

#endif