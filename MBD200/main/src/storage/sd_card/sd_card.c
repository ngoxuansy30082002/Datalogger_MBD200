#include "sd_card.h"

SDCARD_DATA sdcardDt;

static const char * __TAG__ = "SDCARD";
static const SDCARD_PLIB _sdcardPlib = {
    .cardDetect = GPIO_PIN_RC3,
    .ctrlPwr = GPIO_PIN_RA0,
};

static const char * _configFile = "configurations.ini";
static const char * _configFileUpdated = "configurations_updated.ini";

static SDCARD_FLAG _f = {.val = 0};
static uint8_t _totalErr = 0;
static char _dirPath[SDCARD_FOLDER_PATH_LEN] = {0};
static char _lastDirPath[SDCARD_FOLDER_PATH_LEN] = {0};
static bool _loadedIni = false;
static char _chunkBuffer[512];
static char _lineProcessingBuffer[256];
static char _residualBuffer[256];
static size_t _residualLen = 0;

static inline bool _isCardDetect() {
    return (GPIO_PinRead(_sdcardPlib.cardDetect) == true);
}

static inline void _enablePowerForCard(bool en) {
    GPIO_PinWrite(_sdcardPlib.ctrlPwr, en);
}

static bool _openTask() {
    static const uint8_t openNumRetry = 20;
    static SDCARD_OPEN_STATES openState = 0;
    static uint32_t openTick = 0;
    static uint8_t openRetry = 0;

#define SET_BOOT_STATE(nextState) do { openState = nextState; openTick = TICK_NOW(); } while(0)

    if (_totalErr >= 5) {
        _totalErr = 0;
        SET_BOOT_STATE(SDCARD_OPEN_ERROR);
    }

    if (!_isCardDetect() &&
            openState >= SDCARD_OPEN_MOUNT && openState <= SDCARD_OPEN_UNMOUNT) {
        SYS_CONSOLE_PRINT("%s - %s\t Removed unexpectedly\r\n", __TAG__, __func__);
        //        lenLog = snprintf(logs, sizeof (logs), "%s: Removed", __TAG__);
        //        ConsoleLos_Push(logs, lenLog, CONSOLE_INFO);

        _loadedIni = false;
        sdcardDt.status = SDCARD_STS_NOINSERT;
        SET_BOOT_STATE(SDCARD_OPEN_UNMOUNT);
    }

    switch (openState) {
        case SDCARD_OPEN_IDLE:
            if (!TIME_IS_EXPIRED(openTick, 5000))
                SET_BOOT_STATE(SDCARD_OPEN_DETECT);

            break;

        case SDCARD_OPEN_DETECT:
            if (_isCardDetect()) {
                SYS_CONSOLE_PRINT("%s - %s\t Inserted\r\n", __TAG__, __func__);
                //                lenLog = snprintf(logs, sizeof (logs), "%s: Inserted", __TAG__);
                //                ConsoleLos_Push(logs, lenLog, CONSOLE_INFO);

                _enablePowerForCard(true);
                sdcardDt.status = SDCARD_STS_INSERTED;
                SET_BOOT_STATE(SDCARD_OPEN_POWERUP);
            } else {
                _enablePowerForCard(false);
                sdcardDt.status = SDCARD_STS_NOINSERT;
            }
            break;

        case SDCARD_OPEN_POWERUP:
            if (!TIME_IS_EXPIRED(openTick, 2000)) break;
            SET_BOOT_STATE(SDCARD_OPEN_MOUNT);
            break;

        case SDCARD_OPEN_MOUNT:
            if (_f.bits.isMounted)
                openState = SDCARD_OPEN_SET_DRIVER;
            else {
                if (SYS_FS_Mount(SYS_FS_SDCARD_VOL, SYS_FS_SDCARD_MOUNT_POINT,
                        SYS_FS_SDCARD_TYPE, 0, NULL) == SYS_FS_RES_SUCCESS) {
                    _f.bits.isMounted = true;
                    openRetry = 0;
                    openState = SDCARD_OPEN_SET_DRIVER;
                } else {
                    if (++openRetry > openNumRetry) {
                        SYS_CONSOLE_PRINT("%s - %s\t Mount failed after retries\r\n", __TAG__, __func__);
                        _totalErr++;
                        SET_BOOT_STATE(SDCARD_OPEN_IDLE);
                    }
                }
            }
            break;

        case SDCARD_OPEN_SET_DRIVER:
            if (SYS_FS_CurrentDriveSet(SYS_FS_SDCARD_MOUNT_POINT) == SYS_FS_RES_SUCCESS) {
                SYS_CONSOLE_PRINT("%s - %s\t Init SUCCESS\r\n", __TAG__, __func__);
                //                lenLog = snprintf(logs, sizeof (logs), "%s: Init SUCCESS", __TAG__);
                //                ConsoleLos_Push(logs, lenLog, CONSOLE_SUCCESS);

                sdcardDt.status = SDCARD_STS_READY;
                openState = SDCARD_OPEN_READY;
            } else {
                _totalErr++;
                SET_BOOT_STATE(SDCARD_OPEN_IDLE);
            }
            break;

        case SDCARD_OPEN_READY:
            break;

        case SDCARD_OPEN_UNMOUNT:
            if (_f.bits.isMounted) {
                if (SYS_FS_Unmount(SYS_FS_SDCARD_MOUNT_POINT) == SYS_FS_RES_SUCCESS || ++openRetry > openNumRetry) {
                    SYS_CONSOLE_PRINT("%s - %s\t Unmounted\r\n", __TAG__, __func__);
                    _f.bits.isMounted = false;
                    openRetry = 0;
                    _enablePowerForCard(false);
                    SET_BOOT_STATE(SDCARD_OPEN_IDLE);
                }
            } else {
                _enablePowerForCard(false);
                SET_BOOT_STATE(SDCARD_OPEN_IDLE);
            }
            break;

        case SDCARD_OPEN_ERROR:
            SYS_CONSOLE_PRINT("%s - %s\t Critical FS Error: %u\r\n", __TAG__, __func__, SYS_FS_Error());
            sdcardDt.status = SDCARD_STS_ERROR;
            SET_BOOT_STATE(SDCARD_OPEN_UNMOUNT);
            break;
    }

    return (sdcardDt.status >= SDCARD_STS_READY);
}
//
//static bool _extractInfoFromFileName(char * fileName) {
//    char *underscore = strrchr(fileName, '_');
//    if (underscore != NULL) {
//        memmove(appData.fmRetry.file.fileName, underscore + 1, strlen(underscore + 1) + 1);
//    }
//
//    char *dot = strrchr(appData.fmRetry.file.fileName, '.');
//    if (dot == NULL) return false;
//
//    TIME time;
//    char timeString[15];
//    size_t timeLen = dot - appData.fmRetry.file.fileName;
//
//    if (timeLen >= sizeof (timeString)) return false;
//
//    strncpy(timeString, appData.fmRetry.file.fileName, timeLen);
//    timeString[timeLen] = '\0';
//
//    char buffer[5];
//
//    strncpy(buffer, timeString, 4);
//    buffer[4] = '\0';
//    time.year = (uint16_t) atoi(buffer);
//
//    strncpy(buffer, timeString + 4, 2);
//    buffer[2] = '\0';
//    time.month = (uint8_t) atoi(buffer);
//
//    strncpy(buffer, timeString + 6, 2);
//    buffer[2] = '\0';
//    time.day = (uint8_t) atoi(buffer);
//
//    strncpy(buffer, timeString + 8, 2);
//    buffer[2] = '\0';
//    time.hour = (uint8_t) atoi(buffer);
//
//    strncpy(buffer, timeString + 10, 2);
//    buffer[2] = '\0';
//    time.minute = (uint8_t) atoi(buffer);
//
//    strncpy(buffer, timeString + 12, 2);
//    buffer[2] = '\0';
//    time.second = (uint8_t) atoi(buffer);
//
//    appData.fmRetry.file.time = time;
//
//    return true;
//}
//
//static void _processFileAndEnqueue(uint8_t index, char * fileName) {
//    bool ret = _extractInfoFromFileName(fileName);
//    if (ret) {
//        appData.fmRetry.isRetry = true;
//        appData.fmRetry.retryServer = index;
//        for (uint8_t i = 0; i < NUM_FTP_SERVER; i++) {
//            if (index == i && glbAppCfg.ftpServer[i].enable) {
//                appData.fmRetry.server[index].error.ETH_status = BAD;
//                appData.fmRetry.server[index].error.GSM_status = BAD;
//            } else {
//                appData.fmRetry.server[i].error.ETH_status = GOOD;
//                appData.fmRetry.server[i].error.GSM_status = GOOD;
//            }
//        }
//
//        lenLog = snprintf(logs, sizeof (logs), "%s: FTP-%u retry file - %s", __TAG__, index + 1, fileName);
//        ConsoleLos_Push(logs, lenLog, CONSOLE_INFO);
//
//        appData.fmRetry.numFile = 1;
//        APP_FileManager_Enqueue(&appData.fmQueue, &appData.fmRetry);
//    }
//}
//
//static void _scanErrorFileTask() {
//    static const char * dirName[NUM_FTP_SERVER] = {"SFTP1", "SFTP2"};
//    static SDCARD_SCAN_ERROR_FILE_STATE scanError = 0;
//    static uint32_t scanTick = 0;
//    static uint8_t index = 0;
//    static char fileName[100] = "";
//
//    switch (scanError) {
//        case SDCARD_SCAN_ERROR_IDLE:
//
//            if (TIME_IS_EXPIRED(scanTick, 90000)) {
//                scanTick = TICK_NOW();
//
//                if (APP_FileManager_GetSize(&appData.fmQueue) > (FILE_QUEUE_SIZE / 2) ||
//                        appData.ftpStatus != GOOD ||
//                        _f.bits.isBusy) break;
//
//                _f.bits.isBusy = 1;
//                index = 0;
//                scanError = SDCARD_SCAN_ERROR_PROCESS;
//            }
//            break;
//
//        case SDCARD_SCAN_ERROR_PROCESS:
//            while (index < NUM_FTP_SERVER) {
//                if (!glbAppCfg.ftpServer[index].enable) {
//                    index++;
//                    continue;
//                }
//
//                SYS_FS_FSTAT stat;
//                stat.lfname = NULL;
//
//                SYS_FS_DirectoryChange("/");
//                SYS_FS_HANDLE dirHandle = SYS_FS_DirOpen(dirName[index]);
//
//                if (dirHandle != SYS_FS_HANDLE_INVALID) {
//                    if (SYS_FS_DirSearch(dirHandle, "*", SYS_FS_ATTR_FILE, &stat) == SYS_FS_RES_SUCCESS) {
//                        snprintf(fileName, sizeof (fileName), "%.99s", stat.fname);
//                        SYS_FS_DirClose(dirHandle);
//
//                        char fullPath[150];
//                        snprintf(fullPath, sizeof (fullPath), "%s/%s", dirName[index], fileName);
//
//                        SYS_FS_HANDLE fileHandle = SYS_FS_FileOpen(fullPath, SYS_FS_FILE_OPEN_READ);
//                        if (fileHandle != SYS_FS_HANDLE_INVALID) {
//                            size_t bytes_read = SYS_FS_FileRead(fileHandle, &appData.fmRetry.file.fileContent, sizeof (appData.fmRetry.file.fileContent));
//                            SYS_FS_FileClose(fileHandle);
//
//                            if (bytes_read != (size_t) - 1) {
//                                appData.fmRetry.file.fileSize = bytes_read;
//                                _processFileAndEnqueue(index, fileName);
//                                SDcard_RemoveFile(fullPath);
//                            }
//                        }
//                    } else {
//                        SYS_FS_DirClose(dirHandle);
//                    }
//                }
//
//                index++;
//            }
//
//            _f.bits.isBusy = 0;
//            scanError = SDCARD_SCAN_ERROR_IDLE;
//            break;
//    }
//}
//
//static uint8_t _getDirRemove(uint8_t month) {
//    int sub = 0;
//    uint8_t totalMonthRemove = 0;
//    sub = month - (glbAppCfg.sdCard.timeRemove + 1);
//    if (sub < 0) {
//        totalMonthRemove = 12 + sub;
//    } else {
//        totalMonthRemove = 12 + sub;
//    }
//    return totalMonthRemove;
//}
//
//static void _removeLogMonthlyTask() {
//    static SDCARD_REMOVE_MONTHLY_STATES removeState = 0;
//
//    switch (removeState) {
//        case SDCARD_REMOVE_MONTHLY_IDLE:
//        {
//            if (RTC_Dt.sysTime.month != glbAppCfg.sdCard.lastMonth &&
//                    glbAppCfg.sdCard.lastMonth != 0 &&
//                    !_f.bits.isBusy &&
//                    RTC_Dt.flag.Flags.isValidTime) {
//
//                SYS_CONSOLE_PRINT("%s - %s\t Start\r\n", __TAG__, __func__);
//                _f.bits.isBusy = 1;
//                glbAppCfg.sdCard.lastMonth = RTC_Dt.sysTime.month;
//                SaveAppConfig(false);
//
//                removeState = SDCARD_REMOVE_MONTHLY_PROCESS;
//            }
//            break;
//        }
//
//        case SDCARD_REMOVE_MONTHLY_PROCESS:
//        {
//            uint8_t totalDir = _getDirRemove(RTC_Dt.sysTime.month);
//            FILINFO fno;
//
//            for (uint8_t i = 1; i <= totalDir; i++) {
//                uint16_t targetYear = (i <= 12) ? (RTC_Dt.sysTime.year - 1) : RTC_Dt.sysTime.year;
//                uint8_t targetMonth = (i <= 12) ? i : (i - 12);
//                snprintf(_dirPath, sizeof (_dirPath), "%04u%02u", targetYear, targetMonth);
//
//                delete_node(_dirPath, 256, &fno);
//
//                SYS_CONSOLE_PRINT("%s - %s\t  Remove directory %s\r\n", __TAG__, __func__, _dirPath);
//            }
//
//            for (uint8_t i = 1; i <= totalDir; i++) {
//                uint16_t targetYear = (i <= 12) ? (RTC_Dt.sysTime.year - 1) : RTC_Dt.sysTime.year;
//                uint8_t targetMonth = (i <= 12) ? i : (i - 12);
//                snprintf(_dirPath, sizeof (_dirPath), "History/%04u%02u", targetYear, targetMonth);
//
//                delete_node(_dirPath, 256, &fno);
//
//                SYS_CONSOLE_PRINT("%s - %s\t  Remove directory %s\r\n", __TAG__, __func__, _dirPath);
//            }
//
//            _f.bits.isBusy = 0;
//            appData.resetAll = true;
//            removeState = SDCARD_REMOVE_MONTHLY_IDLE;
//            break;
//        }
//    }
//}
//
//static char* _trimString(char *str) {
//    char *end;
//    while (*str == ' ' || *str == '\t') str++;
//    if (*str == 0) return str;
//    end = str + strlen(str) - 1;
//    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) end--;
//    end[1] = '\0';
//    return str;
//}
//
//static void _processIniEntry(const char* mainGroup, const char* subGroup, const char* key, const char* value) {
//    int index = 0;
//
//    if (strcmp(mainGroup, "APP - Network") == 0) {
//        if (strcmp(key, "DHCP_Enable") == 0) glbAppCfg.network.isDHCPEn = (bool) atoi(value);
//        else if (strcmp(key, "NetBIOSName") == 0) {
//            strncpy(glbAppCfg.network.NetBIOSName, value, 15);
//            glbAppCfg.network.NetBIOSName[15] = '\0';
//        } else if (strcmp(key, "AppUserDevice") == 0) {
//            strncpy(glbAppCfg.network.app_username_device, value, 23);
//            glbAppCfg.network.app_username_device[23] = '\0';
//        } else if (strcmp(key, "AppPassDevice") == 0) {
//            strncpy(glbAppCfg.network.app_password_device, value, 23);
//            glbAppCfg.network.app_password_device[23] = '\0';
//        } else if (strcmp(key, "IP_Addr") == 0) {
//            char ipStr[24] = "";
//            strncpy(ipStr, value, sizeof (ipStr) - 1);
//            ipStr[sizeof (ipStr) - 1] = '\0';
//            TCPIP_Helper_StringToIPAddress(ipStr, &glbAppCfg.network.ipAddr);
//        }
//    } else if (strcmp(mainGroup, "APP - Log file") == 0) {
//        if (strcmp(key, "UpLink") == 0) glbAppCfg.ftpGeneral.uplink = (UPLINK) atoi(value);
//        else if (strcmp(key, "FormatData") == 0) glbAppCfg.ftpGeneral.formatData = (FORMATDATA) atoi(value);
//        else if (strcmp(key, "FileType") == 0) glbAppCfg.ftpGeneral.typefile = (TYPEFILE) atoi(value);
//        else if (strcmp(key, "TimeMode") == 0) glbAppCfg.ftpGeneral.timeMode = (TIMEMODE) atoi(value);
//        else if (strcmp(key, "SendInterval") == 0) glbAppCfg.ftpGeneral.SendInterval = (uint16_t) atoi(value);
//    } else if (sscanf(mainGroup, "APP - FTPServer%d", &index) == 1 && index < NUM_FTP_SERVER) {
//        if (strcmp(key, "Enable") == 0) glbAppCfg.ftpServer[index].enable = (bool) atoi(value);
//        else if (strcmp(key, "Port") == 0) glbAppCfg.ftpServer[index].port = (uint8_t) atoi(value);
//        else if (strcmp(key, "MakeFolderType") == 0) glbAppCfg.ftpServer[index].makeFolder = (MAKEFOLDER) atoi(value);
//        else if (strcmp(key, "Hostname") == 0) {
//            strncpy(glbAppCfg.ftpServer[index].hostname, value, sizeof (glbAppCfg.ftpServer[index].hostname) - 1);
//            glbAppCfg.ftpServer[index].hostname[sizeof (glbAppCfg.ftpServer[index].hostname) - 1] = '\0';
//        } else if (strcmp(key, "Path") == 0) {
//            strncpy(glbAppCfg.ftpServer[index].path, value, sizeof (glbAppCfg.ftpServer[index].path) - 1);
//            glbAppCfg.ftpServer[index].path[sizeof (glbAppCfg.ftpServer[index].path) - 1] = '\0';
//        } else if (strcmp(key, "NamePrefix") == 0) {
//            strncpy(glbAppCfg.ftpServer[index].namePrefix, value, sizeof (glbAppCfg.ftpServer[index].namePrefix) - 1);
//            glbAppCfg.ftpServer[index].namePrefix[sizeof (glbAppCfg.ftpServer[index].namePrefix) - 1] = '\0';
//        } else if (strcmp(key, "Username") == 0) {
//            strncpy(glbAppCfg.ftpServer[index].username, value, sizeof (glbAppCfg.ftpServer[index].username) - 1);
//            glbAppCfg.ftpServer[index].username[sizeof (glbAppCfg.ftpServer[index].username) - 1] = '\0';
//        } else if (strcmp(key, "Password") == 0) {
//            strncpy(glbAppCfg.ftpServer[index].password, value, sizeof (glbAppCfg.ftpServer[index].password) - 1);
//            glbAppCfg.ftpServer[index].password[sizeof (glbAppCfg.ftpServer[index].password) - 1] = '\0';
//        }
//    } else if (strcmp(mainGroup, "APP - ModbusRTU Phy") == 0) {
//        if (strcmp(key, "BaudRate") == 0) glbAppCfg.modbusRTU.baudRate = (uint32_t) atoi(value);
//        else if (strcmp(key, "Parity") == 0) glbAppCfg.modbusRTU.parity = (uint8_t) atoi(value);
//        else if (strcmp(key, "Stopbit") == 0) glbAppCfg.modbusRTU.stopbits = (uint8_t) atoi(value);
//        else if (strcmp(key, "Timeout") == 0) glbAppCfg.modbusRTU.timeout = (uint16_t) atoi(value);
//        else if (strcmp(key, "PollInterval") == 0) glbAppCfg.modbusRTU.pollInterval = (uint16_t) atoi(value);
//        else if (strcmp(key, "Retries") == 0) glbAppCfg.modbusRTU.retries = (uint8_t) atoi(value);
//        else if (strcmp(key, "Latency") == 0) glbAppCfg.modbusRTU.latency = (uint16_t) atoi(value);
//    } else if (strcmp(mainGroup, "APP - GSM") == 0) {
//        if (strcmp(key, "APN") == 0) {
//            strncpy(glbAppCfg.GSM.APN, value, 14);
//            glbAppCfg.GSM.APN[14] = '\0';
//        } else if (strcmp(key, "UserAPN") == 0) {
//            strncpy(glbAppCfg.GSM.usernameAPN, value, 14);
//            glbAppCfg.GSM.usernameAPN[14] = '\0';
//        } else if (strcmp(key, "PassAPN") == 0) {
//            strncpy(glbAppCfg.GSM.passAPN, value, 14);
//            glbAppCfg.GSM.passAPN[14] = '\0';
//        } else if (strcmp(key, "Mode") == 0) glbAppCfg.GSM.mode = (GSM_SIM_MODE) atoi(value);
//    } else if (strcmp(mainGroup, "APP - Time") == 0) {
//        if (strcmp(key, "Timezone") == 0) {
//            strncpy(glbAppCfg.time.Timezone, value, 9);
//            glbAppCfg.time.Timezone[9] = '\0';
//        } else if (strcmp(key, "NTPIndex") == 0) glbAppCfg.time.indexNTP = (uint8_t) atoi(value);
//        else if (strcmp(key, "TimeAuto") == 0) glbAppCfg.time.Time_auto = (uint8_t) atoi(value);
//    } else if (strcmp(mainGroup, "APP - Input") == 0) {
//        int inIdx;
//        if (sscanf(key, "IN%dDescribe", &inIdx) == 1 && inIdx >= 1 && inIdx <= 16) {
//            char* target = NULL;
//            if (inIdx == 1) target = glbAppCfg.io.describeIN1;
//            else if (inIdx == 2) target = glbAppCfg.io.describeIN2;
//            else if (inIdx == 3) target = glbAppCfg.io.describeIN3;
//            else if (inIdx == 4) target = glbAppCfg.io.describeIN4;
//            else if (inIdx == 5) target = glbAppCfg.io.describeIN5;
//            else if (inIdx == 6) target = glbAppCfg.io.describeIN6;
//            else if (inIdx == 7) target = glbAppCfg.io.describeIN7;
//            else if (inIdx == 8) target = glbAppCfg.io.describeIN8;
//            else if (inIdx == 9) target = glbAppCfg.io.describeIN9;
//            else if (inIdx == 10) target = glbAppCfg.io.describeIN10;
//            else if (inIdx == 11) target = glbAppCfg.io.describeIN11;
//            else if (inIdx == 12) target = glbAppCfg.io.describeIN12;
//            else if (inIdx == 13) target = glbAppCfg.io.describeIN13;
//            else if (inIdx == 14) target = glbAppCfg.io.describeIN14;
//            else if (inIdx == 15) target = glbAppCfg.io.describeIN15;
//            else if (inIdx == 16) target = glbAppCfg.io.describeIN16;
//
//            if (target != NULL) {
//                strncpy(target, value, MAX_DESCRIPTION_LENGHT - 1);
//                target[MAX_DESCRIPTION_LENGHT - 1] = '\0';
//            }
//        }
//    } else if (strcmp(mainGroup, "APP - Output") == 0) {
//        if (strcmp(key, "OUT1Describe") == 0) {
//            strncpy(glbAppCfg.io.describeOUT1, value, MAX_DESCRIPTION_LENGHT - 1);
//            glbAppCfg.io.describeOUT1[MAX_DESCRIPTION_LENGHT - 1] = '\0';
//        } else if (strcmp(key, "OUT1Type") == 0) glbAppCfg.io.typeCtrlOut1 = (TYPE_CTRL_OUT) atoi(value);
//        else if (strcmp(key, "OUT1Time") == 0) glbAppCfg.io.timeCtrlOut1 = (uint16_t) atoi(value);
//        else if (strcmp(key, "OUT2Describe") == 0) {
//            strncpy(glbAppCfg.io.describeOUT2, value, MAX_DESCRIPTION_LENGHT - 1);
//            glbAppCfg.io.describeOUT2[MAX_DESCRIPTION_LENGHT - 1] = '\0';
//        } else if (strcmp(key, "OUT2Type") == 0) glbAppCfg.io.typeCtrlOut2 = (TYPE_CTRL_OUT) atoi(value);
//        else if (strcmp(key, "OUT2Time") == 0) glbAppCfg.io.timeCtrlOut2 = (uint16_t) atoi(value);
//    } else if (strcmp(mainGroup, "APP - SDcard") == 0) {
//        if (strcmp(key, "TimeRemove") == 0) glbAppCfg.sdCard.timeRemove = (uint8_t) atoi(value);
//    } else if (strcmp(mainGroup, "APP - Get sample API") == 0) {
//        if (strcmp(key, "WhichOut") == 0) glbAppCfg.sampleApi.whichOut = (uint8_t) atoi(value);
//        else if (strcmp(key, "MaxBottle") == 0) glbAppCfg.sampleApi.maxBottle = (uint8_t) atoi(value);
//        else if (strcmp(key, "BottleIdx") == 0) glbAppCfg.sampleApi.bottleIdx = (uint8_t) atoi(value);
//        else if (strcmp(key, "SampleTime") == 0) glbAppCfg.sampleApi.sampleTime = (uint8_t) atoi(value);
//        else if (strcmp(key, "RemotePort") == 0) glbAppCfg.sampleApi.remotePort = (uint16_t) atoi(value);
//        else if (strcmp(key, "LocalPort") == 0) glbAppCfg.sampleApi.localPort = (uint16_t) atoi(value);
//        else if (strcmp(key, "RemoteHost") == 0) {
//            strncpy(glbAppCfg.sampleApi.remoteHost, value, 49);
//            glbAppCfg.sampleApi.remoteHost[49] = '\0';
//        } else if (strcmp(key, "RemotePath") == 0) {
//            strncpy(glbAppCfg.sampleApi.remotePath, value, 49);
//            glbAppCfg.sampleApi.remotePath[49] = '\0';
//        } else if (strcmp(key, "LocalPath") == 0) {
//            strncpy(glbAppCfg.sampleApi.localPath, value, 49);
//            glbAppCfg.sampleApi.localPath[49] = '\0';
//        }
//
//    } else if (strcmp(mainGroup, "APP - HMI Tag") == 0) {
//        if (sscanf(key, "TagHMI%d", &index) == 1 && index < MAX_HMI_PARA)
//            glbAppCfg.tag_hmi[index] = (uint8_t) atoi(value);
//    } else if (strcmp(mainGroup, "APP - ModbusTCP position") == 0) {
//        if (sscanf(key, "Position%d", &index) == 1 && index < MAX_POSITION_SIZE)
//            glbAppCfg.position[index] = (uint16_t) atoi(value);
//    } else if (strcmp(mainGroup, "SENSOR") == 0) {
//        if (subGroup[0] == '\0') {
//            if (strcmp(key, "TotalSensor") == 0) glbAppCfg.sensor.total_sensor = (uint8_t) atoi(value);
//        } else if (sscanf(subGroup, "Entry%d", &index) == 1 && index < MAX_SENSOR) {
//            if (strcmp(key, "Enable") == 0) glbAppCfg.sensor.entry[index].enable = (bool) atoi(value);
//            else if (strcmp(key, "Type") == 0) glbAppCfg.sensor.entry[index].type = (SENSORTYPE) atoi(value);
//            else if (strcmp(key, "IndexOfType") == 0) glbAppCfg.sensor.entry[index].idxInType = (uint8_t) atoi(value);
//            else if (strcmp(key, "Calibrated") == 0) glbAppCfg.sensor.entry[index].calibrated = (bool) atoi(value);
//            else if (strcmp(key, "TypeSuccess") == 0) glbAppCfg.sensor.entry[index].typeRun = (GET_STATUS_TYPE) atoi(value);
//            else if (strcmp(key, "IndexOfTypeSuccess") == 0) glbAppCfg.sensor.entry[index].idxInTypeRun = (uint8_t) atoi(value);
//            else if (strcmp(key, "TypeCalib") == 0) glbAppCfg.sensor.entry[index].typeCalib = (GET_STATUS_TYPE) atoi(value);
//            else if (strcmp(key, "IndexOfTypeCalib") == 0) glbAppCfg.sensor.entry[index].idxInTypeCalib = (uint8_t) atoi(value);
//            else if (strcmp(key, "TypeErr") == 0) glbAppCfg.sensor.entry[index].typeErr = (GET_STATUS_TYPE) atoi(value);
//            else if (strcmp(key, "IndexOfTypeErr") == 0) glbAppCfg.sensor.entry[index].idxInTypeErr = (uint8_t) atoi(value);
//            else if (strcmp(key, "TypeStatus") == 0) glbAppCfg.sensor.entry[index].typeStatus = (GET_STATUS_TYPE) atoi(value);
//            else if (strcmp(key, "SuccessValueAnd") == 0) glbAppCfg.sensor.entry[index].runvalueAND = (uint16_t) atoi(value);
//            else if (strcmp(key, "SuccessValueCompare") == 0) glbAppCfg.sensor.entry[index].runvalueCompare = (uint16_t) atoi(value);
//            else if (strcmp(key, "CalibValueAnd") == 0) glbAppCfg.sensor.entry[index].calibvalueAND = (uint16_t) atoi(value);
//            else if (strcmp(key, "CalibValueCompare") == 0) glbAppCfg.sensor.entry[index].calibvalueCompare = (uint16_t) atoi(value);
//            else if (strcmp(key, "ErrorValueAnd") == 0) glbAppCfg.sensor.entry[index].errorvalueAND = (uint16_t) atoi(value);
//            else if (strcmp(key, "ErrorValueCompare") == 0) glbAppCfg.sensor.entry[index].errorvalueCompare = (uint16_t) atoi(value);
//        }
//    } else if (strcmp(mainGroup, "MODBUSRTU TAG") == 0) {
//        if (strcmp(key, "TotalTag") == 0)
//            glbAppRtu.total_row = (uint8_t) atoi(value);
//        else if (sscanf(subGroup, "Tag%d", &index) == 1 && index < MAX_BUFFER_TAG) {
//            if (strcmp(key, "SlaveAddress") == 0) glbAppRtu.app_rtu_table[index].addr = (uint8_t) atoi(value);
//            else if (strcmp(key, "Function") == 0) glbAppRtu.app_rtu_table[index].func = (uint8_t) atoi(value);
//            else if (strcmp(key, "Register") == 0) glbAppRtu.app_rtu_table[index].addr_reg = (uint16_t) atoi(value);
//            else if (strcmp(key, "Quantity") == 0) glbAppRtu.app_rtu_table[index].bytes = (uint8_t) atoi(value);
//            else if (strcmp(key, "DataType") == 0) glbAppRtu.app_rtu_table[index].type = (uint8_t) atoi(value);
//            else if (strcmp(key, "Enable") == 0) glbAppRtu.app_rtu_table[index].enable = (bool) atoi(value);
//            else if (strcmp(key, "BigEndian") == 0) glbAppRtu.app_rtu_table[index].big_endian = (bool) atoi(value);
//
//            else if (strcmp(key, "Unit") == 0) {
//                strncpy(glbAppRtu.analog_modbus[index].unit, value, sizeof (glbAppRtu.analog_modbus[index].unit) - 1);
//                glbAppRtu.analog_modbus[index].unit[sizeof (glbAppRtu.analog_modbus[index].unit) - 1] = '\0';
//            } else if (strcmp(key, "Name") == 0) {
//                strncpy(glbAppRtu.analog_modbus[index].des, value, sizeof (glbAppRtu.analog_modbus[index].des) - 1);
//                glbAppRtu.analog_modbus[index].des[sizeof (glbAppRtu.analog_modbus[index].des) - 1] = '\0';
//            } else if (strcmp(key, "ScaleType") == 0) glbAppRtu.analog_modbus[index].scale_type = (uint8_t) atoi(value);
//            else if (strcmp(key, "ScaledDataType") == 0) glbAppRtu.analog_modbus[index].scaled_data_type = (uint8_t) atoi(value);
//            else if (strcmp(key, "ScaleValue") == 0) glbAppRtu.analog_modbus[index].scale_value = atof(value);
//            else if (strcmp(key, "ADCType") == 0) glbAppRtu.analog_modbus[index].ADCtype = (uint8_t) atoi(value);
//            else if (strcmp(key, "ADCLow") == 0) glbAppRtu.analog_modbus[index].ADClow = atof(value);
//            else if (strcmp(key, "ADCHigh") == 0) glbAppRtu.analog_modbus[index].ADChigh = atof(value);
//            else if (strcmp(key, "OffsetPreValue") == 0) glbAppRtu.analog_modbus[index].ADCofset_pre = atof(value);
//            else if (strcmp(key, "OffsetSubValue") == 0) glbAppRtu.analog_modbus[index].ADCofset_sub = atof(value);
//            else if (strcmp(key, "OffsetPreOperator") == 0) glbAppRtu.analog_modbus[index].ADCtypepre = (uint8_t) atoi(value);
//            else if (strcmp(key, "OffsetSubOperator") == 0) glbAppRtu.analog_modbus[index].ADCtypesub = (uint8_t) atoi(value);
//        }
//    } else if (strcmp(mainGroup, "ANALOG") == 0) {
//        if (sscanf(subGroup, "Channel%d", &index) == 1 && index < MAX_ANALOG_CHANNEL) {
//            if (strcmp(key, "Name") == 0) {
//                strncpy(glbAppAnlg.entry[index].des, value, sizeof (glbAppAnlg.entry[index].des) - 1);
//                glbAppAnlg.entry[index].des[sizeof (glbAppAnlg.entry[index].des) - 1] = '\0';
//            } else if (strcmp(key, "Unit") == 0) {
//                strncpy(glbAppAnlg.entry[index].unit, value, sizeof (glbAppAnlg.entry[index].unit) - 1);
//                glbAppAnlg.entry[index].unit[sizeof (glbAppAnlg.entry[index].unit) - 1] = '\0';
//            } else if (strcmp(key, "Enable") == 0) glbAppAnlg.entry[index].enable = (bool) atoi(value);
//            else if (strcmp(key, "ScaleType") == 0) glbAppAnlg.entry[index].scale_type = (uint8_t) atoi(value);
//            else if (strcmp(key, "ScaledDataType") == 0) glbAppAnlg.entry[index].scaled_data_type = (uint8_t) atoi(value);
//            else if (strcmp(key, "ScaleValue") == 0) glbAppAnlg.entry[index].scale_value = atof(value);
//            else if (strcmp(key, "ADCType") == 0) glbAppAnlg.entry[index].ADCtype = (uint8_t) atoi(value);
//
//            else if (strcmp(key, "ADCLow") == 0) glbAppAnlg.entry[index].ADClow = atof(value);
//            else if (strcmp(key, "ADCHigh") == 0) glbAppAnlg.entry[index].ADChigh = atof(value);
//            else if (strcmp(key, "OffsetPreOperator") == 0) glbAppAnlg.entry[index].ADCtypepre = (uint8_t) atoi(value);
//            else if (strcmp(key, "OffsetSubOperator") == 0) glbAppAnlg.entry[index].ADCtypesub = (uint8_t) atoi(value);
//            else if (strcmp(key, "OffsetPreValue") == 0) glbAppAnlg.entry[index].ADCofset_pre = atof(value);
//            else if (strcmp(key, "OffsetSubValue") == 0) glbAppAnlg.entry[index].ADCofset_sub = atof(value);
//        }
//    } else if (strcmp(mainGroup, "COUNTER") == 0) {
//        if (sscanf(subGroup, "Counter%d", &index) == 1 && index < (MAX_COUNTER * 2)) {
//            if (strcmp(key, "Name") == 0) {
//                strncpy(glbAppCnter.counter[index].name, value, sizeof (glbAppCnter.counter[index].name) - 1);
//                glbAppCnter.counter[index].name[sizeof (glbAppCnter.counter[index].name) - 1] = '\0';
//            } else if (strcmp(key, "Unit") == 0) {
//                strncpy(glbAppCnter.counter[index].unit, value, sizeof (glbAppCnter.counter[index].unit) - 1);
//                glbAppCnter.counter[index].unit[sizeof (glbAppCnter.counter[index].unit) - 1] = '\0';
//            } else if (strcmp(key, "Enable") == 0) glbAppCnter.counter[index].enable = (bool) atoi(value);
//            else if (strcmp(key, "MinFreq") == 0) glbAppCnter.counter[index].minFreq = (float) atof(value);
//            else if (strcmp(key, "ValueOfPulse") == 0) glbAppCnter.counter[index].pulse = atof(value);
//            else if (strcmp(key, "Scale") == 0) glbAppCnter.counter[index].scale = atof(value);
//            else if (strcmp(key, "MinFreq") == 0) glbAppCnter.counter[index].minFreq = atof(value);
//        }
//    } else if (strcmp(mainGroup, "EXTEND") == 0) {
//        if (subGroup[0] == '\0') {
//            if (strcmp(key, "Enable") == 0) glbExtend.enable = (bool) atoi(value);
//            else if (strcmp(key, "Address") == 0) glbExtend.address = (uint8_t) atoi(value);
//            else if (sscanf(key, "Position%d", &index) == 1 && index < EXTEND_MAX_POSITION_SIZE) glbExtend.position[index] = (uint16_t) atoi(value);
//        } else if (sscanf(subGroup, "IO%d", &index) == 1 && index < EXTEND_MAX_INPUT_OUTPUT) {
//            if (strcmp(key, "Name") == 0) {
//                strncpy(glbExtend.inputOutput[index].des, value, MAX_DESCRIPTION_LENGHT - 1);
//                glbExtend.inputOutput[index].des[MAX_DESCRIPTION_LENGHT - 1] = '\0';
//            } else if (strcmp(key, "Mode") == 0) glbExtend.inputOutput[index].mode = (EXTEND_IO_MODE) atoi(value);
//        } else if (sscanf(subGroup, "CAPTURE%d", &index) == 1 && index < EXTEND_MAX_INPUT_CAPTURE) {
//            if (strcmp(key, "Name") == 0) {
//                strncpy(glbExtend.inputCapture[index].des, value, MAX_DESCRIPTION_LENGHT - 1);
//                glbExtend.inputCapture[index].des[MAX_DESCRIPTION_LENGHT - 1] = '\0';
//            } else if (strcmp(key, "Mode") == 0) glbExtend.inputCapture[index].mode = (EXTEND_IO_MODE) atoi(value);
//            else if (strcmp(key, "PulseRateName") == 0) {
//                strncpy(glbExtend.inputCapture[index].pulseRate.name, value, 23);
//                glbExtend.inputCapture[index].pulseRate.name[23] = '\0';
//            } else if (strcmp(key, "PulseRateUnit") == 0) {
//                strncpy(glbExtend.inputCapture[index].pulseRate.unit, value, 9);
//                glbExtend.inputCapture[index].pulseRate.unit[9] = '\0';
//            } else if (strcmp(key, "PulseRateValuePerPulse") == 0) glbExtend.inputCapture[index].pulseRate.pulse = (float) atof(value);
//            else if (strcmp(key, "PulseRateMinFreq") == 0) glbExtend.inputCapture[index].pulseRate.minFreq = (float) atof(value);
//            else if (strcmp(key, "PulseRateEnable") == 0) glbExtend.inputCapture[index].pulseRate.enable = (bool) atoi(value);
//            else if (strcmp(key, "PulseRateScale") == 0) glbExtend.inputCapture[index].pulseRate.scale = (float) atof(value);
//            else if (strcmp(key, "CounterName") == 0) {
//                strncpy(glbExtend.inputCapture[index].counter.name, value, 23);
//                glbExtend.inputCapture[index].counter.name[23] = '\0';
//            } else if (strcmp(key, "CounterUnit") == 0) {
//                strncpy(glbExtend.inputCapture[index].counter.unit, value, 9);
//                glbExtend.inputCapture[index].counter.unit[9] = '\0';
//            } else if (strcmp(key, "CounterValuePerPulse") == 0) glbExtend.inputCapture[index].counter.pulse = (float) atof(value);
//            else if (strcmp(key, "CounterEnable") == 0) glbExtend.inputCapture[index].counter.enable = (bool) atoi(value);
//        }
//    }
//}
//
//static void _handleLineData(char* lineData) {
//    static char mainGroup[64] = "";
//    static char subGroup[64] = "";
//
//    bool isSubNode = (lineData[0] == '\t' || lineData[0] == ' ');
//    char *line = _trimString(lineData);
//
//    if (strlen(line) == 0 || line[0] == ';' || line[0] == '#') return;
//
//    if (line[0] == '[' && line[strlen(line) - 1] == ']') {
//        line[strlen(line) - 1] = '\0';
//        char* nodeName = line + 1;
//
//        if (isSubNode) {
//            strncpy(subGroup, nodeName, sizeof (subGroup) - 1);
//            subGroup[sizeof (subGroup) - 1] = '\0';
//        } else {
//            strncpy(mainGroup, nodeName, sizeof (mainGroup) - 1);
//            mainGroup[sizeof (mainGroup) - 1] = '\0';
//            subGroup[0] = '\0';
//        }
//        return;
//    }
//
//    char *equalSign = strchr(line, '=');
//    if (equalSign != NULL) {
//        *equalSign = '\0';
//        char *key = _trimString(line);
//        char *value = _trimString(equalSign + 1);
//        _processIniEntry(mainGroup, subGroup, key, value);
//    }
//}
//
//bool _loadConfigurationsFromINI() {
//    if (sdcardDt.status < SDCARD_STS_READY || _f.bits.isBusy) return false;
//
//
//    SYS_FS_FSTAT stat;
//    SYS_FS_HANDLE fileHandle;
//    size_t bytesRead = 0;
//
//    if (SYS_FS_FileStat(_configFile, &stat) != SYS_FS_RES_SUCCESS) return false;
//    fileHandle = SYS_FS_FileOpen(_configFile, SYS_FS_FILE_OPEN_READ);
//    if (fileHandle == SYS_FS_HANDLE_INVALID) return false;
//
//    _f.bits.isBusy = 1;
//    SYS_CONSOLE_PRINT("%s - %s\t %s Open OK \r\n", __TAG__, __func__, _configFile);
//
//    _residualLen = 0;
//    memset(_residualBuffer, 0, sizeof (_residualBuffer));
//    while ((bytesRead = SYS_FS_FileRead(fileHandle, _chunkBuffer, sizeof (_chunkBuffer))) > 0) {
//        size_t chunkIdx = 0;
//        size_t lineStartIdx = 0;
//        for (chunkIdx = 0; chunkIdx < bytesRead; chunkIdx++) {
//            if (_chunkBuffer[chunkIdx] == '\n') {
//                size_t currentLinePartLen = chunkIdx - lineStartIdx;
//
//                memset(_lineProcessingBuffer, 0, sizeof (_lineProcessingBuffer));
//                if (_residualLen > 0)
//                    memcpy(_lineProcessingBuffer, _residualBuffer, _residualLen);
//                if (currentLinePartLen < (sizeof (_lineProcessingBuffer) - _residualLen - 1))
//                    memcpy(_lineProcessingBuffer + _residualLen, &_chunkBuffer[lineStartIdx], currentLinePartLen);
//
//                _handleLineData(_lineProcessingBuffer);
//                _residualLen = 0;
//                lineStartIdx = chunkIdx + 1;
//            }
//        }
//
//        if (lineStartIdx < bytesRead) {
//            _residualLen = bytesRead - lineStartIdx;
//            if (_residualLen < sizeof (_residualBuffer))
//                memcpy(_residualBuffer, &_chunkBuffer[lineStartIdx], _residualLen);
//        }
//    }
//
//    if (_residualLen > 0) {
//        _residualBuffer[_residualLen] = '\0';
//        _handleLineData(_residualBuffer);
//    }
//
//    SYS_FS_FileClose(fileHandle);
//
//    _f.bits.isBusy = 0;
//    SYS_CONSOLE_PRINT("%s - %s:\t LOAD SUCCESS %s\r\n", __TAG__, __func__);
//
//    return true;
//
//    /* Rename ini file */
//    if (SYS_FS_FileStat(_configFileUpdated, &stat) == SYS_FS_RES_SUCCESS)
//        SYS_FS_FileDirectoryRemove(_configFileUpdated);
//
//    if (SYS_FS_FileDirectoryRenameMove(_configFile, _configFileUpdated) == SYS_FS_RES_SUCCESS)
//        SYS_CONSOLE_PRINT("SUCCESS: Configurations loaded and file renamed to %s\r\n", __TAG__, __func__, _configFileUpdated);
//    else
//        SYS_CONSOLE_PRINT("WARNING: Configurations loaded but failed to rename file!\r\n");
//
//    return true;
//}

void SDcard_Initialize() {
    memset(&sdcardDt, 0, sizeof (SDCARD_DATA));
    _f.val = 0;
}

void SDcard_Task() {
    static uint32_t ledTick = 0;
    static bool loadIniRes = false;
    static uint32_t loggTick = 0;

    if (sdcardDt.status == SDCARD_STS_NOINSERT) {
        if (TIME_IS_EXPIRED(ledTick, 1000)) {
            ledTick = TICK_NOW();
            sdcardDt.ledDisp = !sdcardDt.ledDisp;
        }
    } else if (sdcardDt.status == SDCARD_STS_ERROR) {
        if (TIME_IS_EXPIRED(ledTick, 100)) {
            ledTick = TICK_NOW();
            sdcardDt.ledDisp = !sdcardDt.ledDisp;
        }
    } else sdcardDt.ledDisp = 0;

    static uint8_t preStatus = 255;
    if (preStatus != sdcardDt.status) {
        //        HMIDwin_TriggerSend(HMI_TAG_DEVICE_STATUS);
        preStatus = sdcardDt.status;
    }

    bool opened = _openTask();
    if (!opened) return;

    //    _scanErrorFileTask();
    //    _removeLogMonthlyTask();

    //    if (!_loadedIni) {
    //        _loadedIni = true;
    //        loadIniRes = _loadConfigurationsFromINI();
    //        loggTick = TICK_NOW();
    //
    //        HMIDwin_TriggerSend(HMI_TAG_PAGE1_ROW_NAME);
    //        HMIDwin_TriggerSend(HMI_TAG_PAGE1_ROW_UNIT);
    //        HMIDwin_TriggerSend(HMI_TAG_PAGE2_ROW_NAME);
    //        HMIDwin_TriggerSend(HMI_TAG_PAGE2_ROW_UNIT);
    //        HMIDwin_TriggerSend(HMI_TAG_PAGE3_ROW_NAME);
    //        HMIDwin_TriggerSend(HMI_TAG_PAGE3_ROW_UNIT);
    //        HMIDwin_TriggerSend(HMI_TAG_PAGE4_ROW_NAME);
    //        HMIDwin_TriggerSend(HMI_TAG_PAGE4_ROW_UNIT);
    //    }
    //
    //    if (loadIniRes) {
    //        if (TIME_IS_EXPIRED(loggTick, 3000)) {
    //            loadIniRes = false;
    //            Logger_SaveFullConfig(LOGGER_CONFIG_SDCARD_IMPORT);
    //        }
    //    }

}

/*
 * @brief Creates all directories in a path safely if they do not exist.
 * @param path: The directory path (e.g., "/logs/charging/2026")
 * @return true if the entire path is ready, false if any step fails.
 */
bool SDcard_CreateRecursiveDir(const char* path) {
    size_t len;
    if (path == NULL) return false;
    len = strlen(path);
    if (len == 0 || len >= SDCARD_FOLDER_PATH_LEN) return false; /* Prevent buffer overflow */

    static char tempDirPath[SDCARD_FOLDER_PATH_LEN];
    char *p = NULL;
    SYS_FS_FSTAT stat;

    strncpy(tempDirPath, path, sizeof (tempDirPath));
    tempDirPath[SDCARD_FOLDER_PATH_LEN - 1] = '\0'; /* Ensure null-termination */

    if (len > 1 && tempDirPath[len - 1] == '/')
        tempDirPath[len - 1] = '\0';

    /* Skip the first '/' to avoid trying to create root directory */
    p = tempDirPath;
    if (*p == '/') p++;
    /* Iterate through the path and create each segment */
    for (; *p; p++) {
        if (*p == '/') {
            *p = '\0'; /* Temporarily terminate the string at the slash */

            /* Check if this segment exists, if not, create it */
            if (SYS_FS_FileStat(tempDirPath, &stat) != SYS_FS_RES_SUCCESS) {
                if (SYS_FS_DirectoryMake(tempDirPath) != SYS_FS_RES_SUCCESS) {
                    return false; /* Failed to create an intermediate node */
                }
            }
            *p = '/'; /* Restore the slash to continue */
        }
    }

    /* Create the final segment */
    if (SYS_FS_DirectoryMake(tempDirPath) != SYS_FS_RES_SUCCESS) {
        /* Double check if it failed because it already exists */
        if (SYS_FS_FileStat(tempDirPath, &stat) != SYS_FS_RES_SUCCESS) {
            return false;
        }
    }

    return true;
}

/*
 * @brief Appends a string of data to a specific file on the SD Card.
 * @param path: Full path to the file (e.g., "/logs/system.log")
 * @param data: String content to be written
 * @return true if write operation is successful, false otherwise.
 */
bool SDcard_WriteLog(const char* path, const char* data) {
    /* Check safe state */
    if (sdcardDt.status < SDCARD_STS_READY || _f.bits.isBusy) return false;
    if (path == NULL || data == NULL) return false;

    SYS_FS_HANDLE fileHandle;
    char *lastSlash;
    bool success = false;
    bool dirReady = true;

    _f.bits.isBusy = 1;

    /* Extract directory path from file path */
    snprintf(_dirPath, sizeof (_dirPath), "%s", path);
    lastSlash = strrchr(_dirPath, '/');

    if (lastSlash != NULL && lastSlash != _dirPath) {
        *lastSlash = '\0'; /* Remove file name, keep only directory path */

        /* CHECK CACHE: Only verify/create directory if it differs from last time */
        if (strncmp(_dirPath, _lastDirPath, sizeof (_lastDirPath)) != 0) {
            dirReady = SDcard_CreateRecursiveDir(_dirPath);
            if (dirReady) {
                /* Cache this directory to skip future checks */
                strncpy(_lastDirPath, _dirPath, sizeof (_lastDirPath) - 1);
                _lastDirPath[sizeof (_lastDirPath) - 1] = '\0';
            }
        }
    }

    /* Proceed to write file if directory is ready */
    if (dirReady) {
        fileHandle = SYS_FS_FileOpen(path, SYS_FS_FILE_OPEN_APPEND);
        if (fileHandle != SYS_FS_HANDLE_INVALID) {
            size_t dataLen = strlen(data);
            if (SYS_FS_FileWrite(fileHandle, data, dataLen) > 0) {
                success = true;
            }
            SYS_FS_FileClose(fileHandle);
        }
    }

    if (success) {
        _totalErr = 0;
        SYS_CONSOLE_PRINT("%s - %s\t Write file %s Success \r\n", __TAG__, __func__, path);
        /* Note: Ensure global logs buffer exists or is large enough */
        //        lenLog = snprintf(logs, sizeof (logs), "%s: Write %s", __TAG__, path);
        //        ConsoleLos_Push(logs, lenLog, CONSOLE_SUCCESS);
    } else
        _totalErr++;

    _f.bits.isBusy = 0;
    sdcardDt.status = SDCARD_STS_GOOD;
    return success;
}

bool SDcard_FileIsExists(const char* path) {
    SYS_FS_FSTAT stat;
    if (SYS_FS_FileStat(path, &stat) == SYS_FS_RES_SUCCESS)
        return true;

    return false;
}

bool SDcard_RemoveFile(const char* path) {
    if (SYS_FS_FileDirectoryRemove(path) == SYS_FS_RES_SUCCESS)
        return true;

    return false;
}

bool SDcard_isBusy() {
    return _f.bits.isBusy;
}

bool SDcard_SetHidden(const char* path) {
    if (SYS_FS_FileDirectoryModeSet(path, SYS_FS_ATTR_HID, SYS_FS_ATTR_HID) == SYS_FS_RES_SUCCESS) {
        return true;
    }
    return false;
}