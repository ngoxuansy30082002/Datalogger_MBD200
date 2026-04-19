#include "sim/sim_general.h"
#include "sim/core/sim_driver.h"
#include "sim/core/sim_basic.h"
#include "sim/core/sim_net.h"
#include "sim_ftp.h"


static const char * __TAG__ = "SIMFTP";

static int _cmdBuilder(int state, char* buffer, size_t maxLen, const char* format);
static bool _respParser(int state, char* buffer, size_t maxLen);

static const SIM_CMD_SEQ _cmdTable[SIM_FTP_COUNT] = {
    /* { cmd, builderFunc, respOk, respFail, timeoutMs, attempts, parserFunc, nextStateOk, nextStateFail } */

    [SIM_FTP_CFG_ACCOUNT] =
    { "AT+QFTPCFG=\"account\",\"%s\",\"%s\"\r\n", _cmdBuilder, "OK", "ERROR", 2000, 3, NULL, SIM_FTP_CFG_FILETYPE, SIM_FTP_ERROR},

    [SIM_FTP_CFG_FILETYPE] =
    { "AT+QFTPCFG=\"filetype\",1\r\n", NULL, "OK", "ERROR", 2000, 3, NULL, SIM_FTP_CFG_TRANSMODE, SIM_FTP_ERROR},

    [SIM_FTP_CFG_TRANSMODE] =
    { "AT+QFTPCFG=\"transmode\",1\r\n", NULL, "OK", "ERROR", 2000, 3, NULL, SIM_FTP_CFG_CONTEXTID, SIM_FTP_ERROR},

    [SIM_FTP_CFG_CONTEXTID] =
    { "AT+QFTPCFG=\"contextid\",%u\r\n", _cmdBuilder, "OK", "ERROR", 2000, 3, NULL, SIM_FTP_CFG_RSPTIMEOUT, SIM_FTP_ERROR},

    [SIM_FTP_CFG_RSPTIMEOUT] =
    { "AT+QFTPCFG=\"rsptimeout\",%u\r\n", _cmdBuilder, "OK", "ERROR", 2000, 3, NULL, SIM_FTP_OPEN, SIM_FTP_ERROR},

    [SIM_FTP_OPEN] =
    { "AT+QFTPOPEN=\"%s\",%u\r\n", _cmdBuilder, "OK", "ERROR", 15000, 1, NULL, SIM_FTP_WAIT_LOGIN, SIM_FTP_TRY_CLOSE},

    [SIM_FTP_WAIT_LOGIN] =
    { "AT+QFTPSTAT\r\n", NULL, "+QFTPSTAT:", "ERROR", 120000, 10, _respParser, SIM_FTP_UPLOAD_ROOT, SIM_FTP_TRY_CLOSE},

    [SIM_FTP_TRY_CLOSE] =
    { "AT+QFTPCLOSE\r\n", NULL, "OK", "ERROR", 15000, 3, NULL, SIM_FTP_TRY_WAIT_LOGOUT, SIM_FTP_ERROR},

    [SIM_FTP_TRY_WAIT_LOGOUT] =
    { "AT+QFTPSTAT\r\n", NULL, "+QFTPSTAT:", "ERROR", 60000, 1, _respParser, SIM_FTP_OPEN, SIM_FTP_ERROR},

    [SIM_FTP_UPLOAD_ROOT] =
    { "AT+QFTPCWD=\"/\"\r\n", NULL, "+QFTPCWD:", "ERROR", 15000, 3, _respParser, SIM_FTP_UPLOAD_FULLPATH, SIM_FTP_ERROR},

    [SIM_FTP_UPLOAD_FULLPATH] =
    { "AT+QFTPCWD=\"%s\"\r\n", _cmdBuilder, "+QFTPCWD:", "ERROR", 15000, 1, _respParser, SIM_FTP_UPLOAD_PUT_CMD, SIM_FTP_UPLOAD_CWD},

    [SIM_FTP_UPLOAD_CWD] =
    { "AT+QFTPCWD=\"%s\"\r\n", _cmdBuilder, "+QFTPCWD:", "ERROR", 15000, 1, _respParser, SIM_FTP_UPLOAD_NEXT_DIR, SIM_FTP_UPLOAD_MKD},

    [SIM_FTP_UPLOAD_MKD] =
    { "AT+QFTPMKDIR=\"%s\"\r\n", _cmdBuilder, "+QFTPMKDIR:", "ERROR", 15000, 3, _respParser, SIM_FTP_UPLOAD_CWD, SIM_FTP_ERROR},

    [SIM_FTP_UPLOAD_NEXT_DIR] =
    { "AT\r\n", NULL, "OK", "ERROR", 2000, 1, _respParser, SIM_FTP_UPLOAD_CWD, SIM_FTP_UPLOAD_PUT_CMD},

    [SIM_FTP_UPLOAD_PUT_CMD] =
    { "AT+QFTPPUT=\"%s\",\"COM:\",0,%u,1\r\n", _cmdBuilder, "CONNECT", "ERROR", 60000, 3, NULL, SIM_FTP_UPLOAD_PUT_DATA, SIM_FTP_ERROR},

    [SIM_FTP_UPLOAD_PUT_DATA] =
    { NULL, _cmdBuilder, "+QFTPPUT:", "ERROR", 120000, 1, _respParser, SIM_FTP_CLOSE, SIM_FTP_ERROR},

    [SIM_FTP_CLOSE] =
    { "AT+QFTPCLOSE\r\n", NULL, "OK", "ERROR", 15000, 3, NULL, SIM_FTP_WAIT_LOGOUT, SIM_FTP_ERROR},

    [SIM_FTP_WAIT_LOGOUT] =
    { "AT+QFTPSTAT\r\n", NULL, "+QFTPSTAT:", "ERROR", 60000, 1, _respParser, SIM_FTP_READY, SIM_FTP_ERROR},
};

static SIM_FTP_STATE _currentState = 0;
static bool _isWaitingResp = false;
static bool _isBuilded = false;
static int _currentTxLen = 0;
static uint8_t _attemptCount = 0;

static uint8_t _idxServer = 0;
static char * _directories[16] = {0};
static uint8_t _dirDepth = 0;
static uint8_t _dirCount = 0;
static char _fileData[SIM_FTP_FILE_LEN] = "";
static uint16_t _fileSize = 0;
static char _fullPath[SIM_FTP_PATH_LEN] = "";
static char _fileName[SIM_FTP_PATH_LEN] = "";

static int _cmdBuilder(int state, char* buffer, size_t maxLen, const char* format) {
    if (buffer == NULL || maxLen == 0) return 0;

    switch (state) {
        case SIM_FTP_CFG_ACCOUNT:
            return snprintf(buffer, maxLen, format,
                    gAppCfg.ftpServer[_idxServer].username,
                    gAppCfg.ftpServer[_idxServer].password);

        case SIM_FTP_CFG_CONTEXTID:
            return snprintf(buffer, maxLen, format,
                    SIM_CONTEXT_ID);

        case SIM_FTP_CFG_RSPTIMEOUT:
            return snprintf(buffer, maxLen, format,
                    SIM_FTP_TIMEOUT);

        case SIM_FTP_OPEN:
            return snprintf(buffer, maxLen, format,
                    gAppCfg.ftpServer[_idxServer].hostname,
                    gAppCfg.ftpServer[_idxServer].port);
            break;

        case SIM_FTP_UPLOAD_FULLPATH:
            _dirCount = 0;
            return snprintf(buffer, maxLen, format, _fullPath);

        case SIM_FTP_UPLOAD_CWD:
        case SIM_FTP_UPLOAD_MKD:
            if (_directories[_dirCount] != NULL)
                return snprintf(buffer, maxLen, format, _directories[_dirCount]);

        case SIM_FTP_UPLOAD_PUT_CMD:
            return snprintf(buffer, maxLen, format,
                    _fileName, _fileSize);

        case SIM_FTP_UPLOAD_PUT_DATA:
            return snprintf(buffer, maxLen, "%s",
                    _fileData);

        default:
            if (format) return snprintf(buffer, maxLen, "%s", format);
            return 0;
    }
}

static bool _respParser(int state, char* buffer, size_t maxLen) {
    if (buffer == NULL) return false;

    char *pMatch = NULL;
    int errCode = -1;
    int protocolErr = -1;
    int parsedItems = 0;

    switch (state) {
        case SIM_FTP_WAIT_LOGIN:
            pMatch = strstr(buffer, "+QFTPSTAT:");
            if (pMatch != NULL) {
                parsedItems = sscanf(pMatch, "+QFTPSTAT: %d,%d", &errCode, &protocolErr);
                if (parsedItems == 2 && errCode == 0 && protocolErr == 1)
                    return true;
            }
            return false;

        case SIM_FTP_WAIT_LOGOUT:
            pMatch = strstr(buffer, "+QFTPSTAT:");
            if (pMatch != NULL) {
                parsedItems = sscanf(pMatch, "+QFTPSTAT: %d,%d", &errCode, &protocolErr);
                if (parsedItems == 2 && errCode == 0 && protocolErr == 4)
                    return true;
            }
            return false;

        case SIM_FTP_UPLOAD_ROOT:
        case SIM_FTP_UPLOAD_FULLPATH:
        case SIM_FTP_UPLOAD_CWD:
            pMatch = strstr(buffer, "+QFTPCWD:");
            if (pMatch != NULL) {
                parsedItems = sscanf(pMatch, "+QFTPCWD: %d,%d", &errCode, &protocolErr);
                if (parsedItems >= 1 && errCode == 0)
                    return true;
            }
            return false;

        case SIM_FTP_UPLOAD_MKD:
            pMatch = strstr(buffer, "+QFTPMKDIR:");
            if (pMatch != NULL) {
                parsedItems = sscanf(pMatch, "+QFTPMKDIR: %d,%d", &errCode, &protocolErr);
                if (parsedItems >= 1 && errCode == 0)
                    return true;
            }
            return false;

        case SIM_FTP_UPLOAD_NEXT_DIR:
            _dirCount++;
            if (_dirCount < _dirDepth)
                return true;
            else return false;

        case SIM_FTP_UPLOAD_PUT_DATA:
            pMatch = strstr(buffer, "+QFTPPUT:");
            if (pMatch != NULL) {
                int uploadedSize = -1;
                parsedItems = sscanf(pMatch, "+QFTPPUT: %d,%d", &errCode, &uploadedSize);

                if (parsedItems == 2 && errCode == 0)
                    return true;
            }
            return false;

        default:
            return true;
    }
}

static void _handleErrorOrTimeout(void) {
    const SIM_CMD_SEQ * cmdInfo = &_cmdTable[_currentState];

    if (_attemptCount < cmdInfo->attempts - 1) {
        /* Increase retry counter and retry current command */
        _attemptCount++;
    } else {

        /* Retry exhausted -> move to fail state (ERROR) */
        _currentState = cmdInfo->nextStateFail;
        _attemptCount = 0; /* Reset counter for next command */
    }

    _isWaitingResp = false;
    _isBuilded = false;
}

bool SIMFtp_Start(uint8_t idxServer, const char * path, uint16_t pathLen, const char * fileData, uint16_t fileSize) {
    if (_currentState > SIM_FTP_IDLE)
        return false;

    _currentState = SIM_FTP_CFG_ACCOUNT;
    _isWaitingResp = false;
    _isBuilded = false;
    _currentTxLen = 0;
    _attemptCount = 0;
    _idxServer = idxServer;

    _fileSize = fileSize;
    uint16_t copyLen = (fileSize > SIM_FTP_FILE_LEN) ? SIM_FTP_FILE_LEN : fileSize;
    strncpy(_fileData, fileData, copyLen);
    _fileData[copyLen] = '\0';

    copyLen = (pathLen > SIM_FTP_PATH_LEN) ? SIM_FTP_PATH_LEN : pathLen;
    strncpy(_fullPath, path, copyLen);
    _fullPath[copyLen] = '\0';

    static char _pathParseBuf[SIM_FTP_PATH_LEN] = "";
    char *lastSlash = strrchr(_fullPath, '/');
    if (lastSlash != NULL) {
        strncpy(_fileName, lastSlash + 1, sizeof (_fileName) - 1);
        _fileName[sizeof (_fileName) - 1] = '\0';
        *lastSlash = '\0';
    } else {
        /* no slash -> path is root */
        strncpy(_fileName, _fullPath, sizeof (_fileName) - 1);
        _fileName[sizeof (_fileName) - 1] = '\0';
        _fullPath[0] = '\0';
    }

    _dirDepth = 0;
    _dirCount = 0;
    if (_fullPath[0] != '\0') {
        memcpy(_pathParseBuf, _fullPath, strlen(_fullPath) + 1);

        char *token = strtok(_pathParseBuf, "/");
        while (token != NULL && _dirDepth < 16) {
            _directories[_dirDepth] = token;
            _dirDepth++;
            token = strtok(NULL, "/");
        }
    }

    return true;
}

void SIMFtp_Abort(void) {
    _currentState = SIM_FTP_IDLE;
}

bool SIMFtp_IsReady(void) {

    return (_currentState == SIM_FTP_READY);
}

bool SIMFtp_HasError(void) {

    return (_currentState == SIM_FTP_ERROR);
}

void SIMFtp_Process(void) {
    if (!SIMBasic_IsReady() || !SIMNet_IsReady() ||
            _currentState == SIM_FTP_IDLE ||
            _currentState == SIM_FTP_READY ||
            _currentState == SIM_FTP_ERROR) {
        return;
    }

    /* 1. COMMAND NOT SENT YET -> PREPARE AND SEND */
    if (!_isWaitingResp) {
        const SIM_CMD_SEQ * cmdInfo = &_cmdTable[_currentState];

        if (!_isBuilded) {
            uint8_t* txbuf = SIMDriver_GetBuffer(SIM_DRV_TX_BUSY);
            if (txbuf == NULL) return; /* Driver busy, wait next cycle */

            if (cmdInfo->builderFunc != NULL)
                _currentTxLen = cmdInfo->builderFunc((int) _currentState, (char*) txbuf, SIM_TRANSFER_BUFF_SIZE, cmdInfo->cmd);
            else
                _currentTxLen = snprintf((char*) txbuf, SIM_TRANSFER_BUFF_SIZE, "%s", cmdInfo->cmd);

            SYS_CONSOLE_PRINT("%s - %s:\t Builed: %s\r\n", __TAG__, __func__, (char *) txbuf);
            _isBuilded = true; /* Mark as built */
        }

        /* Execute sending to driver */
        if (_currentTxLen > 0) {
            if (SIMDriver_Execute((size_t) _currentTxLen, cmdInfo->timeoutMs)) {
                //                SYS_CONSOLE_PRINT("%s - %s:\t Sended\r\n", __TAG__, __func__);F
                _isWaitingResp = true;
                _isBuilded = false; /* Clear flag so next cycle/state can rebuild */
            }
        }
        return;
    } else { /* 2. COMMAND SENT -> WAIT FOR RESPONSE */
        SIM_DRV_STATUS status = SIMDriver_GetStatus();

        if (status == SIM_DRV_STATUS_RECV_RESP) {
            uint8_t* rxbuf = SIMDriver_GetBuffer(SIM_DRV_RX_BUSY);
            if (rxbuf != NULL) {
                SYS_CONSOLE_PRINT("%s - %s:\t Receive %s\r\n", __TAG__, __func__, (char *) rxbuf);

                const char* expectedOk = _cmdTable[_currentState].respOk;
                const char* expectedFail = _cmdTable[_currentState].respFail;

                if (strstr((char*) rxbuf, expectedOk) != NULL) {
                    _isWaitingResp = false;
                    /* Success: move to next state and reset retry counter */
                    bool parsed = true;
                    if (_cmdTable[_currentState].parserFunc)
                        parsed = _cmdTable[_currentState].parserFunc(_currentState, rxbuf, SIM_TRANSFER_BUFF_SIZE);
                    if (parsed) {
                        _attemptCount = 0;
                        _currentState = _cmdTable[_currentState].nextStateOk;
                    } else
                        _handleErrorOrTimeout();
                } else if (strstr((char*) rxbuf, expectedFail) != NULL) {
                    _isWaitingResp = false;
                    _handleErrorOrTimeout();
                }
            }
        } else if (status == SIM_DRV_STATUS_TIMEOUT) {
            SYS_CONSOLE_PRINT("%s - %s:\t Timeout\r\n", __TAG__, __func__);
            _handleErrorOrTimeout();
        }
    }
}
