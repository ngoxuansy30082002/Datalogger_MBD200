/* 
 * File:   sim_basic.h
 * Author: LENOVO
 *
 * Created on March 31, 2026, 8:19 PM
 */

#ifndef SIM_BASIC_H
#define	SIM_BASIC_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "definitions.h"

#ifdef	__cplusplus
extern "C" {
#endif

    typedef enum {
        SIM_BASIC_IDLE = 0,
        SIM_BASIC_AT,
        SIM_BASIC_ATE0,
        SIM_BASIC_QSIMSTAT,
        SIM_BASIC_GSN,
        SIM_BASIC_QDSIMCFG,
        SIM_BASIC_QDSIM,
        SIM_BASIC_QSIMSTAT_QUERY,
        SIM_BASIC_QCCID,
        SIM_BASIC_CREG,
        SIM_BASIC_QSPN,
        SIM_BASIC_CMGF,
        SIM_BASIC_CSQ,
        SIM_BASIC_READY,
        SIM_BASIC_ERROR,
        SIM_BASIC_COUNT
    } SIM_BASIC_STATE;

    typedef struct {
        bool inserted;
        char imei[24];
        char ccid[32];
        char networkName[32];
        int rssi;
    } SIM_BASIC_INFO;

    /* CÁC HÀM CUNG C?P CHO APPLICATION LAYER */
    void SIMBasic_Initialize(uint8_t sim_slot);
    void SIMBasic_Process(void);
    bool SIMBasic_IsReady(void);
    bool SIMBasic_HasError(void);
    // Lay info qua hmi
    SIM_BASIC_INFO* SIMBasic_GetInfo(void);
#ifdef	__cplusplus
}
#endif

#endif	/* SIM_BASIC_H */

