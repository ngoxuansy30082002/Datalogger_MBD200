/* 
 * File:   external_flash.h
 * Author: LENOVO
 *
 * Created on September 26, 2025, 3:01 PM
 */

#ifndef EXTERNAL_FLASH_H
#define	EXTERNAL_FLASH_H

#include <stdio.h>
#include <string.h>
#include "device_cache.h"
#include "definitions.h"

#ifdef	__cplusplus
extern "C" {
#endif

    typedef struct __attribute__((__packed__)) {
        uint16_t magic;
        uint8_t type;
        uint16_t length;
        uint32_t crc;
    }
    EXTFL_METADATA;


    typedef enum {
             EXTFL_DATA_NONE = 0,

    } EXTFL_DATA_TYPE;

    typedef enum {
        EXTFL_SUCCESS = 0,
        EXTFL_TRANSFER_TIMEOUT,
        EXTFL_ERASE_FAIL,
        EXTFL_WRITE_FAIL,
        EXTFL_READ_FAIL,
        EXTFL_VERIFY_FAIL
    } EXTFL_RESULT;

    typedef enum {
        EXTFL_WRITE_IDLE = 0,
        EXTFL_WRITE_ERASE_SECTOR,
        EXTFL_WRITE_TRANSFER,
        EXTFL_WRITE_DONE,
    } EXTFL_WRITE_STATE;

    typedef enum {
        EXTFL_READ_IDLE = 0,
        EXTFL_READ_TRANSFER,
        EXTFL_READ_WAIT_TRANSFER,
        EXTFL_READ_VERIFY_DATA,
        EXTFL_READ_DONE
    } EXTFL_READ_STATE;

    typedef struct {
        EXTFL_DATA_TYPE type;
        uint32_t startAddress;
    } EXTFL_PARTITION;

    typedef struct {
        EXTFL_DATA_TYPE type;
        uint16_t size;
        void (*callback)(int type, int result);
    } EXTFL_QUEUE_ENTRY;


    void ExtFlash_Initialize();
    void ExtFlash_Task();

    bool ExtFlash_SaveConfig(EXTFL_DATA_TYPE dataType, void (*clb)(int type, int rlst));
    bool ExtFlash_LoadConfig(EXTFL_DATA_TYPE dataType, void (*clb)(int type, int rlst));

#ifdef	__cplusplus
}
#endif

#endif	/* EXTERNAL_FLASH_H */

