/* 
 * File:   sensor_general.h
 * Author: LENOVO
 *
 * Created on June 6, 2026, 8:49 AM
 */

#ifndef SENSOR_GENERAL_H
#define	SENSOR_GENERAL_H

#include <stdio.h>
#include <string.h>
#include "definitions.h"

#define FILE_QUEUE_SIZE              10

#ifdef	__cplusplus
extern "C" {
#endif

    /* ?? Uplink phase tracking for UPLINK_ALL ?? */
    typedef enum {
        UPLINK_PHASE_PRIMARY, /* ETH FTP  */
        UPLINK_PHASE_FALLBACK /* GSM FTP  */
    } UPLINK_PHASE;

    typedef enum {
        LOG_FILE_IDLE = 0,
        LOG_FILE_PROCESS_QUEUE,
        LOG_FILE_WAIT_UPLINK,
        LOG_FILE_HANDLE_ERROR
    } LOG_FILE_STATE;

    typedef struct {

        struct {
            TIME time;
            char content[FILE_MAX_SIZE];
            uint16_t size;
            char name[FILE_NAME_LEN];
        } file;
        uint8_t numFile;

        struct {
            bool isErr;
        } server[MAX_FTP_SERVER];

        bool isRetry;
        uint8_t retryFtpId;
    } LOG_FILE;

    typedef struct {
        LOG_FILE items[FILE_QUEUE_SIZE];
        int front;
        int rear;
        int size;
    } LOG_FILE_QUEUE;

    void SensorGeneral_Initialize(void);
    void SensorGeneral_Task(void);

    const char* FileMgr_GetUploadFileName(void);
    const char* FileMgr_GetUploadFileData(void);
    uint32_t FileMgr_GetUploadFileSize(void);
    TIME FileMgr_GetUploadFileTime(void);

#ifdef	__cplusplus
}
#endif

#endif	/* SENSOR_GENERAL_H */

