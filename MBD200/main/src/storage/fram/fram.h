/* 
 * File:   fram.h
 * Author: LENOVO
 *
 * Created on December 29, 2025, 3:37 PM
 */

#ifndef FRAM_H
#define	FRAM_H

#include <stdio.h>
#include <string.h>
#include "definitions.h"

#define FRAM_ADDR_CS_OPERATION                     0x20
#define FRAM_ADDR_TRANSACTION_EVSE1                0x30
#define FRAM_ADDR_METER                            0x830
#define FRAM_QUEUE_SIZE                            16

#ifdef	__cplusplus
extern "C" {
#endif

    typedef struct __attribute__((__packed__)) {
        uint8_t type;
        uint16_t len;
        uint32_t crc;
    }
    FRAM_METADATA;

    typedef enum {
        FRAM_DATA_UNKNOWN = 0,
        FRAM_DATA_CS_OPERATION,
        FRAM_DATA_TRANSACTION_EVSE1,
        FRAM_DATA_METER,
        FRAM_DATA_COUNT,
    } FRAM_DATA_TYPE;

    typedef enum {
        FRAM_RES_SUCCESS = 0,
        FRAM_RES_TIMEOUT,
        FRAM_RES_FAIL,
    } FRAM_RESULT;

    typedef enum {
        FRAM_IDLE = 0,
        FRAM_TRANSFER,
        FRAM_WAIT_TRANSFER,
        FRAM_DONE,
    } FRAM_STATES;

    typedef struct {
        FRAM_DATA_TYPE type;
        uint16_t address;
        uint16_t maxSize;
    } FRAM_PARTITION;

    typedef struct {
        FRAM_DATA_TYPE type;
        void * buffer;
        uint16_t size;
        void (*callback)(int type, int result);
    } FRAM_QUEUE_ITEM;

    void Fram_Initialize(void);
    void Fram_Task(void);
    bool Fram_SaveBlockData(FRAM_DATA_TYPE type, void * buffer, uint16_t size, void (*clb)(int type, int rlst));
    bool Fram_LoadBlockData(FRAM_DATA_TYPE type, void * buffer, uint16_t size, void (*clb)(int type, int rlst));

#ifdef	__cplusplus
}
#endif

#endif	/* FRAM_H */

