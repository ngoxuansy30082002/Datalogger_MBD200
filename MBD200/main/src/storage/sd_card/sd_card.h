/* 
 * File:   SDcard.h
 * Author: Syxn
 *
 * Created on August 18, 2024, 8:53 PM
 */

#ifndef SDCARD_H
#define	SDCARD_H

#include <stdio.h>
#include <string.h>
#include "definitions.h"

#define SDCARD_FOLDER_PATH_LEN                  256

#ifdef	__cplusplus
extern "C" {
#endif

    typedef struct {
        GPIO_PIN ctrlPwr;
        GPIO_PIN cardDetect;
    } SDCARD_PLIB;

    typedef enum {
        SDCARD_STS_ERROR = -1,
        SDCARD_STS_NOINSERT = 0,
        SDCARD_STS_INSERTED,
        SDCARD_STS_READY,
        SDCARD_STS_GOOD,
    } SDCARD_STATUS;

    typedef union {

        struct {
            unsigned int isMounted : 1;
            unsigned int isBusy : 1;
            unsigned int reserved : 2;
        } bits;
        uint8_t val;
    } SDCARD_FLAG;

    typedef enum {
        SDCARD_OPEN_IDLE = 0,
        SDCARD_OPEN_DETECT,
        SDCARD_OPEN_POWERUP,
        SDCARD_OPEN_MOUNT,
        SDCARD_OPEN_SET_DRIVER,
        SDCARD_OPEN_READY,
        SDCARD_OPEN_UNMOUNT,
        SDCARD_OPEN_ERROR,
    } SDCARD_OPEN_STATES;

    typedef struct {
        SDCARD_STATUS status;

    } SDCARD_DATA;

    extern SDCARD_DATA sdcardDt;

    void SDcard_Initialize(void);
    void SDcard_Task(void);

    bool SDcard_WriteLog(const char* path, const char* data);
    bool SDcard_FileIsExists(const char* path);
    bool SDcard_RemoveFile(const char* path);

#ifdef	__cplusplus
}
#endif

#endif	/* SDCARD_H */

