/* 
 * File:   logger.h
 * Author: LENOVO
 *
 * Created on October 6, 2025, 3:30 PM
 */

#ifndef LOGGER_H
#define	LOGGER_H

#include <stdio.h>
#include <string.h>
#include "definitions.h"
#include <time.h>

#define LOGGER_ROOT_FOLDER                      "/logs"
#define LOGGER_DIAG_FOLDER                      "/diags"
#define LOGGER_MAX_TYPE                         4
#define LOGGER_LINE_MAX_SIZE                    256
#define LOGGER_WRITE_BUFFER_SIZE                2048       // 2KB buffer
#define LOGGER_FOLDER_PATH_LEN                  256
#define LOGGER_READ_BUFFER_SIZE                 1024       // 2KB buffer

#define LOGGER_FILENAME_LEN                     64

#ifdef	__cplusplus
extern "C" {
#endif

    typedef enum {
        LOGGER_LEVEL_SUCCESS = 0,
        LOGGER_LEVEL_INFO,
        LOGGER_LEVEL_WARN,
        LOGGER_LEVEL_ERROR,
    } LOGGER_LEVEL;

    typedef enum {
        LOG_TYPE_SYSTEM = 0,
        LOG_TYPE_CHARGING,
        LOG_TYPE_OCPP,
        LOG_TYPE_CONFIG
    } LOG_TYPE;

    typedef enum {
        LOGGER_MODULE_DCPS = 0,
        LOGGER_MODULE_ENERGY_METER,
        LOGGER_MODULE_CMM_INTERFACE,
        LOGGER_MODULE_FAN_MANAGER,
    } LOGGER_MODULE;

    typedef enum {
        LOGGER_WRITE_IDLE = 0,
        LOGGER_WRITE_FLUSH_BUFFER,
    } LOGGER_WRITE_STATE;

    typedef enum {
        LOGGER_DIAG_IDLE = 0,
        LOGGER_DIAG_PREPARE_LIST,
        LOGGER_DIAG_PROCESSING_ZIP,
        LOGGER_DIAG_COMPLETED,
        LOGGER_DIAG_ERROR,
    } LOGGER_DIAG_STATE;

    typedef enum {
        LOGGER_READ_IDLE = 0,
        LOGGER_READ_NEXT_FILE,
        LOGGER_READ_GET_CHUNK,
        LOGGER_READ_WAIT_UPLOAD,
    } LOGGER_READ_STATE;

    typedef union {

        struct {
            uint8_t collectLogs : 1;
            uint8_t dataReady : 1;
            uint8_t uploadDone : 1;
            uint8_t endOfFile : 1;
            uint8_t reserved : 3;
        } bits;
        uint8_t val;
    } LOGGER_FLAG;

    typedef struct {
        LOGGER_WRITE_STATE state;
        char data[2][LOGGER_WRITE_BUFFER_SIZE];
        uint8_t writeIdx;
        size_t position;
        uint32_t flushTick;
        uint8_t forceFlush;

        uint8_t bufferToFlush;
        size_t sizeToFlush;
        TIME logTime;
    } LOGGER_BUFFER;

    typedef struct {
        LOGGER_DIAG_STATE state;
        time_t startTime;
        time_t stopTime;
        time_t currentScanTime;
        uint8_t currentLogTypeIdx;
        char outputZipPath[LOGGER_FOLDER_PATH_LEN];
        uint8_t isBusy;
    } LOGGER_DIAG_CONTROL;

    typedef struct {
        uint16_t pollingTime;
    } LOGGER_GLOBAL_CONFIG;

    typedef struct {
        LOGGER_GLOBAL_CONFIG config;
    } LOGGER_OBJECT;

    void Logger_Initialize(void);
    void Logger_Task(void);
    void Logger_WriteSystem(uint8_t evseId, LOGGER_LEVEL level, LOGGER_MODULE module, const char *msg);
    void Logger_WriteCharging(uint8_t evseId, LOGGER_LEVEL level, LOGGER_MODULE module, const char *msg);
    void Logger_WriteOCPP(uint8_t evseId, LOGGER_LEVEL level, LOGGER_MODULE module, const char *msg);
    void Logger_WriteConfig(uint8_t evseId, LOGGER_LEVEL level, LOGGER_MODULE module, const char *msg);

    void Logger_buildListLogFile(TIME startTime, TIME stopTime);
    bool Logger_dataLogIsReady();
    bool Logger_dataLogIsDone();
    bool Logger_getDataLog(void *data, uint16_t maxSize, uint16_t *size);

#ifdef	__cplusplus
}
#endif

#endif	/* LOGGER_H */

