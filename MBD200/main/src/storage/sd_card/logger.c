#include "logger.h"

static const char * __TAG__ = "LOGGER";

static const char* _logFileNames[] = {"system", "charging", "ocpp", "config"};
static const char * _levelStrs[] = {"SUCCESS", "INFO", "WARN", "ERROR"};
static const char * _moduleStrs[] = {"DCPS", "EM", "CMM"};
static const char * _evseidStrs[] = {"EVSE00", "EVSE01", "EVSE01"};

static LOGGER_BUFFER _logBuffers[LOGGER_MAX_TYPE] = {0};
static LOGGER_DIAG_CONTROL _diagCtrl = {0};
static LOGGER_OBJECT _loggerObj;

//static LOGGER_FLAG _f;

static void Logger_AddLine(LOG_TYPE type, uint8_t evseId, LOGGER_LEVEL level, LOGGER_MODULE module, const char *msg) {
    char line[LOGGER_LINE_MAX_SIZE];
    const char *lStr = (level < 4) ? _levelStrs[level] : "UNK";
    const char *mStr = (module < 3) ? _moduleStrs[module] : "UNK";
    const char *eStr = (evseId < 3) ? _evseidStrs[evseId] : "UNK";
    const RTC_CONTEXT *rtcCtx = Rtc_GetContext();

    int len = snprintf(line, sizeof (line),
            "%04u-%02u-%02u %02u:%02u:%02u\t%-8s\t[%-8s]\t[%-8s]\t%-8s\r\n",
            rtcCtx->sysTime.year, rtcCtx->sysTime.month, rtcCtx->sysTime.day,
            rtcCtx->sysTime.hour, rtcCtx->sysTime.minute, rtcCtx->sysTime.second,
            eStr, lStr, mStr, msg);
    if (len <= 0) return;

    LOGGER_BUFFER *lb = &_logBuffers[type];

    if (lb->position + len >= LOGGER_WRITE_BUFFER_SIZE) {
        lb->forceFlush = 1;
        return;
    }

    memcpy(&lb->data[lb->writeIdx][lb->position], line, len);
    lb->position += len;
    if (level == LOGGER_LEVEL_ERROR)
        lb->forceFlush = 1;
}

//static int CompareTime(const TIME *a, const TIME *b) {
//    if (a->year != b->year) return (a->year < b->year) ? -1 : 1;
//    if (a->month != b->month) return (a->month < b->month) ? -1 : 1;
//    if (a->day != b->day) return (a->day < b->day) ? -1 : 1;
//    if (a->hour != b->hour) return (a->hour < b->hour) ? -1 : 1;
//    return 0;
//}

//void _callbackHandler(int size) {
//    if (size > 0) {
//        _f.bits.dataReady = 1;
//        _currentReadSize = (uint16_t) size;
//    } else if (size <= 0) {
//        _f.bits.endOfFile = 1;
//        _currentReadSize = 0;
//    }
//}

void Logger_Initialize(void) {
    _loggerObj.config.pollingTime = 20; //s
}

void Logger_Task(void) {
    static const uint8_t numRetry = 10;
    static uint8_t retry = 0;

    const LOGGER_GLOBAL_CONFIG *cfg = &_loggerObj.config;

    if (_diagCtrl.isBusy) {
        switch (_diagCtrl.state) {
            case LOGGER_DIAG_PREPARE_LIST:
            {
                _diagCtrl.currentScanTime = _diagCtrl.startTime;
                _diagCtrl.currentLogTypeIdx = 0;
                _diagCtrl.state = LOGGER_DIAG_PROCESSING_ZIP;

                SYS_CONSOLE_PRINT("Diag: Start archiving...\r\n");
                break;
            }

            case LOGGER_DIAG_PROCESSING_ZIP:
            {
                if (_diagCtrl.currentScanTime <= _diagCtrl.stopTime) {
                    struct tm *time_ptr = localtime(&_diagCtrl.currentScanTime);
                    char filePath[LOGGER_FOLDER_PATH_LEN];

                    /* Create the correct path to the file: /logs/YYYYMM/YYYYMMDD/type.log */
                    snprintf(filePath, sizeof (filePath), "%s/%04u%02u/%04u%02u%02u/%s.log",
                            LOGGER_ROOT_FOLDER,
                            time_ptr->tm_year + 1900, time_ptr->tm_mon + 1,
                            time_ptr->tm_year + 1900, time_ptr->tm_mon + 1, time_ptr->tm_mday,
                            _logFileNames[_diagCtrl.currentLogTypeIdx]);

                    /* Check file is Exits */
                    if (SDcard_FileIsExists(filePath)) {
                        /* Add file to compress */
                        int res = SDcard_ArchiveAndCompressFile(_diagCtrl.outputZipPath, filePath);
                        if (res == 0) /* Not yet complete */
                            return;
                        else if (res == -1) {
                            _diagCtrl.state = LOGGER_DIAG_ERROR;
                            break;
                        }
                    }

                    _diagCtrl.currentLogTypeIdx++;
                    if (_diagCtrl.currentLogTypeIdx >= LOGGER_MAX_TYPE) {
                        _diagCtrl.currentLogTypeIdx = 0;
                        _diagCtrl.currentScanTime += 86400; /* Go to next day */
                    }
                } else
                    _diagCtrl.state = LOGGER_DIAG_COMPLETED;

                break;
            }

            case LOGGER_DIAG_COMPLETED:
            {
                int res = SDcard_FinalizeTarGzip();
                if (res == 0) /* Not yet complete */
                    return;
                else if (res == -1) {
                    _diagCtrl.state = LOGGER_DIAG_ERROR;
                    break;
                }
                _diagCtrl.isBusy = 0;
                _diagCtrl.state = LOGGER_DIAG_IDLE;

                SYS_CONSOLE_PRINT("Diag: All done, resuming normal logs\r\n");
                break;
            }

            case LOGGER_DIAG_ERROR:
            {
                SDcard_RemoveFile(_diagCtrl.outputZipPath);
                _diagCtrl.isBusy = 0;
                _diagCtrl.state = LOGGER_DIAG_IDLE;

                break;
            }

            default: break;
        }

        /* Prioritize diagnostics, not regular logging */
        return;
    }

    /* Write log state machine (Only run when no diagnostic is required.) */
    static uint8_t currentLogIdx = 0;
    LOGGER_BUFFER *lb = &_logBuffers[currentLogIdx];

    switch (lb->state) {
        case LOGGER_WRITE_IDLE:
        {
            if (TIME_IS_EXPIRED(lb->flushTick, cfg->pollingTime) ||
                    (lb->position > (LOGGER_WRITE_BUFFER_SIZE * 0.75f)) ||
                    (lb->forceFlush == 1)) {
                if (lb->position == 0) {
                    /* This log no data -> goto next log type */
                    currentLogIdx = (currentLogIdx + 1) % LOGGER_MAX_TYPE;
                    break;
                }

                /* Prepare data and wrap log buffer */
                const RTC_CONTEXT *rtcCtx = Rtc_GetContext();
                lb->logTime = rtcCtx->sysTime;
                lb->bufferToFlush = lb->writeIdx;
                lb->sizeToFlush = lb->position;

                lb->writeIdx = (lb->writeIdx == 0) ? 1 : 0;
                lb->position = 0;
                lb->forceFlush = 0;
                retry = 0;

                lb->state = LOGGER_WRITE_FLUSH_BUFFER;
            } else /* Not yet -> goto next log type */
                currentLogIdx = (currentLogIdx + 1) % LOGGER_MAX_TYPE;

            break;
        }

        case LOGGER_WRITE_FLUSH_BUFFER:
        {
            char dirPath[LOGGER_FOLDER_PATH_LEN];
            /* create path: /logs/YYYYMM/YYYYMMDD/filename.log */
            snprintf(dirPath, sizeof (dirPath), "%s/%04u%02u/%04u%02u%02u/%s.log",
                    LOGGER_ROOT_FOLDER,
                    lb->logTime.year, lb->logTime.month,
                    lb->logTime.year, lb->logTime.month, lb->logTime.day,
                    _logFileNames[currentLogIdx]);
            lb->data[lb->bufferToFlush][lb->sizeToFlush] = '\0';

            if (SDcard_WriteLog(dirPath, (const char*) lb->data[lb->bufferToFlush])) {
                memset(lb->data[lb->bufferToFlush], 0, LOGGER_WRITE_BUFFER_SIZE);
                lb->flushTick = TICK_NOW();
                lb->state = LOGGER_WRITE_IDLE;

                SYS_CONSOLE_PRINT("%s - %s:\t Log [%s] Flush OK\r\n", __TAG__, __func__, _logFileNames[currentLogIdx]);
                /* Write done -> goto next log type */
                currentLogIdx = (currentLogIdx + 1) % LOGGER_MAX_TYPE;
            } else {
                retry++;
                if (retry > numRetry) {
                    SYS_CONSOLE_PRINT("%s - %s:\t Log [%s] Critical Error, Data Lost\r\n", __TAG__, __func__, _logFileNames[currentLogIdx]);
                    memset(lb->data[lb->bufferToFlush], 0, LOGGER_WRITE_BUFFER_SIZE);
                    lb->state = LOGGER_WRITE_IDLE;
                    currentLogIdx = (currentLogIdx + 1) % LOGGER_MAX_TYPE;
                }
            }
            break;
        }
    }
}

//    switch (_readState) {
//        case LOGGER_READ_IDLE:
//        {
//            if (!SDCARD_isBusy() && _f.bits.collectLogs) {
//                _f.bits.collectLogs = 0;
//                ethFTP_Dt.flag.bits.putFile = 1;
//                _readState = LOGGER_READ_NEXT_FILE;
//            }
//            break;
//        }
//        case LOGGER_READ_NEXT_FILE:
//        {
//            if (iqueue_dequeue(&_readQueue, &readItem) == I_OK) {
//                SYS_CONSOLE_PRINT("path: %s, name: %s\r\n", readItem.folderPath, readItem.fileName);
//                SDCARD_startGetDataFromFile(readItem.folderPath, readItem.fileName, _readBuffer, sizeof (_readBuffer), _callbackHandler);
//                _f.bits.dataReady = 0;
//                _readState = LOGGER_READ_GET_CHUNK;
//            } else
//                _readState = LOGGER_READ_IDLE;
//            break;
//        }
//        case LOGGER_READ_GET_CHUNK:
//        {
//            if (_f.bits.endOfFile) {
//                _f.bits.endOfFile = 0;
//                _readState = LOGGER_READ_NEXT_FILE;
//                break;
//            }
//
//            if (!_f.bits.dataReady) break;
//            _f.bits.dataReady = 0;
//
//            _readState = LOGGER_READ_WAIT_UPLOAD;
//            break;
//        }
//        case LOGGER_READ_WAIT_UPLOAD:
//        {
//            //            SYS_CONSOLE_PRINT("readBuff: %c %c %c\r\n", _readBuffer[0], _readBuffer[1], _readBuffer[2]);
//            if (!_f.bits.uploadDone) break;
//            _f.bits.uploadDone = 0;
//
//            SDCARD_resumeGetDataFromFile();
//            _readState = LOGGER_READ_GET_CHUNK;
//            break;
//        }
//}
//}

void Logger_WriteSystem(uint8_t evseId, LOGGER_LEVEL level, LOGGER_MODULE module, const char *msg) {
    Logger_AddLine(LOG_TYPE_SYSTEM, evseId, level, module, msg);
}

void Logger_WriteCharging(uint8_t evseId, LOGGER_LEVEL level, LOGGER_MODULE module, const char *msg) {
    Logger_AddLine(LOG_TYPE_CHARGING, evseId, level, module, msg);
}

void Logger_WriteOCPP(uint8_t evseId, LOGGER_LEVEL level, LOGGER_MODULE module, const char *msg) {
    Logger_AddLine(LOG_TYPE_OCPP, evseId, level, module, msg);
}

void Logger_WriteConfig(uint8_t evseId, LOGGER_LEVEL level, LOGGER_MODULE module, const char *msg) {
    Logger_AddLine(LOG_TYPE_CONFIG, evseId, level, module, msg);
}

bool Logger_StartArchiveDiagnostics(time_t start, time_t stop) {
    if (_diagCtrl.isBusy) return false;

    _diagCtrl.startTime = start;
    _diagCtrl.stopTime = stop;
    struct tm *startTimePtr = localtime(&_diagCtrl.startTime);
    struct tm *stopTimePtr = localtime(&_diagCtrl.stopTime);

    snprintf(_diagCtrl.outputZipPath, sizeof (_diagCtrl.outputZipPath), "%s/diag_%04u%02u%02u_%04u%02u%02u.log",
            LOGGER_DIAG_FOLDER,
            startTimePtr->tm_year + 1900, startTimePtr->tm_mon + 1, startTimePtr->tm_mday,
            stopTimePtr->tm_year + 1900, stopTimePtr->tm_mon + 1, stopTimePtr->tm_mday);

    _diagCtrl.state = LOGGER_DIAG_PREPARE_LIST;
    _diagCtrl.isBusy = 1;

    SYS_CONSOLE_PRINT("Diag: Request accepted from %ld to %ld\r\n", start, stop);
    return true;
}