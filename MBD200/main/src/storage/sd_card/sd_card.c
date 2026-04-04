#include "sd_card.h"

SDCARD_DATA sdcardDt;

static char * __TAG__ = "SDCARD";
static const SDCARD_PLIB _sdcardPlib = {
    .cardDetect = GPIO_PIN_RC3,
    .ctrlPwr = GPIO_PIN_RA0,
};
static SDCARD_FLAG _f;
static uint8_t _totalErr = 0;

static inline bool _isCardDetect() {
    return (GPIO_PinRead(_sdcardPlib.cardDetect) == 1);
}

static inline void _enablePowerForCard(bool en) {
    GPIO_PinWrite(_sdcardPlib.ctrlPwr, en);
}

static bool _openTask() {
    static const uint8_t openNumRetry = 20;
    static SDCARD_OPEN_STATES openState = 0;
    static uint32_t openTick = 0;
    static uint8_t openRetry = 0;

    uint32_t currentTick = SYS_TMR_TickCountGet();
    uint32_t tickPerSecond = SYS_TMR_TickCounterFrequencyGet();

#define SET_BOOT_STATE(nextState) do { openState = nextState; openTick = currentTick; } while(0)

    if (_totalErr >= 5) {
        _totalErr = 0;
        SET_BOOT_STATE(SDCARD_OPEN_ERROR);
    }

    if (!_isCardDetect() &&
            openState >= SDCARD_OPEN_MOUNT && openState <= SDCARD_OPEN_UNMOUNT) {
        SYS_CONSOLE_PRINT("%s - %s\t Removed unexpectedly\r\n", __TAG__, __func__);
        sdcardDt.status = SDCARD_STS_NOINSERT;
        SET_BOOT_STATE(SDCARD_OPEN_UNMOUNT);
    }

    switch (openState) {
        case SDCARD_OPEN_IDLE:
            if (currentTick - openTick >= (5 * tickPerSecond))
                SET_BOOT_STATE(SDCARD_OPEN_DETECT);

            break;

        case SDCARD_OPEN_DETECT:
            if (_isCardDetect()) {
                SYS_CONSOLE_PRINT("%s - %s\t Inserted\r\n", __TAG__, __func__);
                _enablePowerForCard(true);
                sdcardDt.status = SDCARD_STS_INSERTED;
                SET_BOOT_STATE(SDCARD_OPEN_POWERUP);
            } else {
                _enablePowerForCard(false);
                sdcardDt.status = SDCARD_STS_NOINSERT;
            }
            break;

        case SDCARD_OPEN_POWERUP:
            if (currentTick - openTick < (2 * tickPerSecond)) break;
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
                sdcardDt.status = SDCARD_STS_READY;
                SYS_CONSOLE_PRINT("%s - %s\t Init SUCCESS\r\n", __TAG__, __func__);
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

void SDcard_Initialize() {
    memset(&sdcardDt, 0, sizeof (SDCARD_DATA));
    _f.val = 0;
}

void SDcard_Task() {
    bool opened = _openTask();
    if (!opened) return;

    /* test */
    //    static uint32_t testTick = 0;
    //    static char testStr[10] = "";
    //    static char testPath[64] = "/logs/2026/03/log.txt";
    //    uint32_t currentTick = SYS_TMR_TickCountGet();
    //    uint32_t tickPerSecond = SYS_TMR_TickCounterFrequencyGet();
    //
    //    if (currentTick - testTick > tickPerSecond * 5) {
    //        testTick = currentTick;
    //        snprintf(testStr, sizeof (testStr), "%u\n", testTick);
    //        SDcard_WriteLog(testPath, testStr);
    //    }
    /* End test */
}

/**
 * @brief Creates all directories in a path if they do not exist.
 * @param path: The directory path (e.g., "logs/charging/2026")
 * @return true if the entire path is ready, false if any step fails.
 */
bool SDcard_CreateRecursiveDir(char* path) {
    char *p = NULL;
    size_t len;
    SYS_FS_FSTAT stat;

    len = strlen(path);
    if (path == NULL || len == 0) return false;
    /* Remove trailing slash if any */
    if (path[len - 1] == '/')
        path[len - 1] = 0;

    /* Iterate through the path and create each segment */
    for (p = path + 1; *p; p++) {
        if (*p == '/') {
            *p = 0; /* Temporarily terminate the string at the slash */

            /* Check if this segment exists, if not, create it */
            if (SYS_FS_FileStat(path, &stat) != SYS_FS_RES_SUCCESS) {
                if (SYS_FS_DirectoryMake(path) != SYS_FS_RES_SUCCESS)
                    return false;
            }
            *p = '/'; /* Restore the slash to continue to the next segment */
        }
    }

    /* Create the final segment (or the only segment) */
    if (SYS_FS_DirectoryMake(path) != SYS_FS_RES_SUCCESS) {
        /* Double check if it failed because it already exists */
        if (SYS_FS_FileStat(path, &stat) != SYS_FS_RES_SUCCESS)
            return false;
    }

    return true;
}

/**
 * @brief Appends a string of data to a specific file on the SD Card.
 * @param path: Full path to the file (e.g., "/logs/system.log")
 * @param data: String content to be written
 * @return true if write operation is successful, false otherwise.
 */
bool SDcard_WriteLog(const char* path, const char* data) {
    char dirPath[SDCARD_FOLDER_PATH_LEN];
    SYS_FS_HANDLE fileHandle;
    char *lastSlash;
    bool success = false;

    if (sdcardDt.status < SDCARD_STS_READY || _f.bits.isBusy) return false;
    if (path == NULL || data == NULL) return false;
    _f.bits.isBusy = 1;

    snprintf(dirPath, sizeof (dirPath), "%s", path);
    lastSlash = strrchr(dirPath, '/');
    bool dirReady = false;
    if (lastSlash != NULL && lastSlash != dirPath) {
        *lastSlash = '\0';
        dirReady = SDcard_CreateRecursiveDir(dirPath);
    }

    if (dirReady) {
        fileHandle = SYS_FS_FileOpen(path, SYS_FS_FILE_OPEN_APPEND);
        if (fileHandle != SYS_FS_HANDLE_INVALID) {
            size_t dataLen = strlen(data);
            if (SYS_FS_FileWrite(fileHandle, data, dataLen) > 0)
                success = true;

            SYS_FS_FileClose(fileHandle);
        }
    }

    if (success) _totalErr = 0;
    else _totalErr++;
    _f.bits.isBusy = 0;
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

bool SDCARD_isBusy() {
    return _f.bits.isBusy;
}
