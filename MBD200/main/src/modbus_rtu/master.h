/* 
 * File:   master.h
 * Author: Syxn
 *
 * Created on June 14, 2024, 9:58 AM
 */

#ifndef MB_MASTER_H
#define	MB_MASTER_H

#include <stdio.h>
#include <string.h>
#include "definitions.h"
#include <math.h>

#ifdef	__cplusplus
extern "C" {
#endif

    typedef struct {
        uint64_t reuint;
        int64_t reint;

        union {
            /* data */
            float f;
            uint8_t b[4];
            uint32_t c;
        } refloat;
    } MBRTU_RAW_VALUE;

    typedef union {
        int64_t intVal;
        uint64_t uintVal;
        float floatVal;
    } MBRTU_PARSED_VALUE;

    typedef enum {
        MBRTU_MASTER_INIT = 0,
        MBRTU_MASTER_IDLE,
        MBRTU_MASTER_REQUEST,
        MBRTU_MASTER_RESPONSE,
        MBRTU_MASTER_PARSE,
        MBRTU_MASTER_COMPLETE,
    } MBRTU_MASTER_STATES;

    typedef struct {
        MBRTU_MASTER_STATES state;
        SENSOR_STATUS status[MAX_BUFFER_TAG];
        MBRTU_RAW_VALUE rawValue[MAX_BUFFER_TAG];
        MBRTU_PARSED_VALUE value[MAX_BUFFER_TAG];
        bool led;
        bool reInit;
    } MBRTU_MASTER_DATA;

    void MbrtuMaster_Initialize(void);
    void MbrtuMaster_Tasks(void);

    uint16_t MBRTU_getStatusFromMbRtu(uint8_t idxMb);

    extern MBRTU_MASTER_DATA mbrtuMasterDt;


#ifdef	__cplusplus
}
#endif

#endif	/* MB_MASTER_H */

