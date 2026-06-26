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

/* ===================== Rule Engine Runtime ===================== */
#define RULE_SCAN_INTERVAL_MS    100U     /* Period of rule scan         */
#define RULE_DELTA_DEFAULT_MS    3000U    /* Default window for DELTA    */
#define RULE_DELTA_HISTORY_SIZE  16U      /* Ring buffer per rule        */
#define RULE_NOTIFY_REARM_MS     30000U   /* Anti-spam: min gap          */

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

    typedef enum {
        FTP_STS_ERROR = -1,
        FTP_STS_IDENTIFYING = 0,
        FTP_STS_GOOD,
    } FTP_STATUS;

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

    typedef struct {
        /* Threshold debounce tracking */
        bool conditionActive;
        uint32_t conditionStartMs;

        /* Delta ring buffer (samples value1's sensor) */
        float histVal [RULE_DELTA_HISTORY_SIZE];
        uint32_t histTick[RULE_DELTA_HISTORY_SIZE];
        uint8_t histHead;
        uint8_t histCount;

        /* Anti-spam */
        uint32_t lastNotifyMs;
    } RULE_RUNTIME_STATE;

    typedef struct {
        FTP_STATUS ftpStatus;
    } SENSOR_GENERAL_DATA;

    extern SENSOR_GENERAL_DATA ssGeneralDt;

    void SensorGeneral_Initialize(void);
    void SensorGeneral_Task(void);
    int8_t SensorGeneral_calculateSensorStatusInput(uint8_t i);
    int8_t SensorGeneral_calculateSensorStatusAuto(SENSOR_TYPE type, uint8_t index);

    const char* FileMgr_GetUploadFileName(void);
    const char* FileMgr_GetUploadFileData(void);
    uint32_t FileMgr_GetUploadFileSize(void);
    TIME FileMgr_GetUploadFileTime(void);

#ifdef	__cplusplus
}
#endif

#endif	/* SENSOR_GENERAL_H */

