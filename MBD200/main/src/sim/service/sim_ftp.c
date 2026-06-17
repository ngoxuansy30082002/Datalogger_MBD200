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

/* ???????????????????????????????????????????????????????????????????
 *  Static Variables
 * ??????????????????????????????????????????????????????????????????? */
static SIM_FTP_STATE _currentState = 0;
static bool _isWaitingResp = false;
static bool _isBuilded = false;
static int _currentTxLen = 0;
static uint8_t _attemptCount = 0;

/* Multi-server management (like eth_ftp.c) */
static SIM_FTP_RESULT _ftpResult = {false, SIM_FTP_SERVER_IDLE, SIM_FTP_SERVER_IDLE};
static bool _targetFtp1 = false;
static bool _targetFtp2 = false;
static uint8_t _idxServer = 0;

/* Variables for nested directory creation */
static char * _directories[16] = {0};
static uint8_t _dirDepth = 0;
static uint8_t _dirCount = 0;
static char _fullPath[SIM_FTP_PATH_LEN] = "";

/* ???????????????????????????????????????????????????????????????????
 *  Path & Server helpers (like eth_ftp.c)
 * ??????????????????????????????????????????????????????????????????? */

/* * Prepare upload path from server config and file time
 * (Similar to eth_ftp.c FTP_STATE_PREPARE_PATH)
 */
static void _preparePath(void) {
    FTP_SERVER_CONFIG* srvCfg = &gAppCfg.ftpServer[_idxServer];
    TIME fTime = FileMgr_GetUploadFileTime();
    size_t len;

    /* Copy base directory path */
    strncpy(_fullPath, srvCfg->dirPath, sizeof (_fullPath) - 1);
    _fullPath[sizeof (_fullPath) - 1] = '\0';

    /* Remove trailing slash for clean appending */
    len = strlen(_fullPath);
    if (len > 0 && (_fullPath[len - 1] == '/' || _fullPath[len - 1] == '\\')) {
        _fullPath[len - 1] = '\0';
    }

    /* Append dynamic folder based on makeFolder setting */
    if (srvCfg->makeFolder == MAKE_FOLDER_DAY) {
        snprintf(_fullPath + strlen(_fullPath),
                sizeof (_fullPath) - strlen(_fullPath),
                "/%04u%02u%02u", fTime.year, fTime.month, fTime.day);
    } else if (srvCfg->makeFolder == MAKE_FOLDER_MONTH) {
        snprintf(_fullPath + strlen(_fullPath),
                sizeof (_fullPath) - strlen(_fullPath),
                "/%04u%02u/%04u%02u%02u", fTime.year, fTime.month, fTime.year, fTime.month, fTime.day);
    }

    /* Parse directories for CWD/MKD sequence */
    static char _pathParseBuf[SIM_FTP_PATH_LEN] = "";
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
}

/* Forward declaration for mutual call */
static void _evaluateNext(bool success);

/* * Initialize upload session for a specific server
 * (Similar to eth_ftp.c FTP_STATE_INIT_SERVER)
 */
static void _initServer(uint8_t serverIdx) {
    _idxServer = serverIdx;

    FTP_SERVER_CONFIG* srvCfg = &gAppCfg.ftpServer[_idxServer];
    if (!srvCfg->enable) {
        /* Server not enabled ? skip to evaluate */
        _evaluateNext(false);
        return;
    }

    _preparePath();

    _currentState = SIM_FTP_CFG_ACCOUNT;
    _isWaitingResp = false;
    _isBuilded = false;
    _currentTxLen = 0;
    _attemptCount = 0;
}

/* * Evaluate next server after current one completes
 * (Similar to eth_ftp.c FTP_STATE_EVALUATE_NEXT)
 */
static void _evaluateNext(bool success) {
    /* 1. Record the result of the completed server */
    if (success) {
        if (_idxServer == 0) _ftpResult.server1 = SIM_FTP_SERVER_SUCCESS;
        else _ftpResult.server2 = SIM_FTP_SERVER_SUCCESS;
    } else {
        if (_idxServer == 0) _ftpResult.server1 = SIM_FTP_SERVER_FAILED;
        else _ftpResult.server2 = SIM_FTP_SERVER_FAILED;
    }

    /* 2. Check if the next server is pending */
    if (_targetFtp1) {
        _targetFtp1 = false;
        _initServer(0);
    } else if (_targetFtp2) {
        _targetFtp2 = false;
        _initServer(1);
    } else {
        /* 3. Both servers handled. Clear uploading flag */
        _ftpResult.isUploading = false;
        _currentState = SIM_FTP_IDLE;
    }
}

/* ???????????????????????????????????????????????????????????????????
 *  Command Builder 
 * ??????????????????????????????????????????????????????????????????? */

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
                    FileMgr_GetUploadFileName(),
                    FileMgr_GetUploadFileSize());

        case SIM_FTP_UPLOAD_PUT_DATA:
        {
            uint32_t totalSize = FileMgr_GetUploadFileSize();
            const char* fileData = FileMgr_GetUploadFileData();
            uint16_t copyLen = (totalSize > maxLen) ? (uint16_t) maxLen : (uint16_t) totalSize;
            memcpy(buffer, fileData, copyLen);
            return (int) copyLen;
        }

        default:
            if (format) return snprintf(buffer, maxLen, "%s", format);
            return 0;
    }
}

/* ???????????????????????????????????????????????????????????????????
 *  Response Parser
 * ??????????????????????????????????????????????????????????????????? */

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

/* ???????????????????????????????????????????????????????????????????
 *  Error/Timeout handler 
 * ??????????????????????????????????????????????????????????????????? */

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

/* ???????????????????????????????????????????????????????????????????
 *  Public API
 * ??????????????????????????????????????????????????????????????????? */

/* * API: Trigger Upload
 */
bool SIMFtp_Start(bool ftp1, bool ftp2) {
    if (_currentState != SIM_FTP_IDLE)
        return false;
    if (!ftp1 && !ftp2)
        return false;

    _targetFtp1 = ftp1;
    _targetFtp2 = ftp2;

    /* Set general uploading flag */
    _ftpResult.isUploading = true;

    /* Reset status for triggered servers */
    if (ftp1) _ftpResult.server1 = SIM_FTP_SERVER_IDLE;
    if (ftp2) _ftpResult.server2 = SIM_FTP_SERVER_IDLE;

    /* Start with first requested server */
    if (ftp1) {
        _targetFtp1 = false;
        _initServer(0);
    } else {
        _targetFtp2 = false;
        _initServer(1);
    }

    return true;
}

void SIMFtp_Abort(void) {
    _currentState = SIM_FTP_IDLE;
    _targetFtp1 = false;
    _targetFtp2 = false;
    _ftpResult.isUploading = false;
}

bool SIMFtp_IsReady(void) {

    return (_currentState == SIM_FTP_IDLE && !_ftpResult.isUploading);
}

bool SIMFtp_HasError(void) {

    return (_currentState == SIM_FTP_IDLE && !_ftpResult.isUploading &&
            (_ftpResult.server1 == SIM_FTP_SERVER_FAILED ||
            _ftpResult.server2 == SIM_FTP_SERVER_FAILED));
}

/* * Get the detailed status of the FTP upload process
 */
SIM_FTP_RESULT SIMFtp_GetStatus(void) {
    return _ftpResult;
}

/* * Main Non-blocking Task
 */
void SIMFtp_Process(void) {
    if (!SIMBasic_IsReady() || !SIMNet_IsReady()) {
        return;
    }

    /* Evaluate next server when current one finishes */
    if (_currentState == SIM_FTP_READY) {
        _evaluateNext(true);
        return;
    }
    if (_currentState == SIM_FTP_ERROR) {
        _evaluateNext(false);
        return;
    }

    if (_currentState == SIM_FTP_IDLE) {
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

//            LOG_DEBUG("%s - %s:\t Builed: %s\r\n", __TAG__, __func__, (char *) txbuf);
            _isBuilded = true; /* Mark as built */
        }

        /* Execute sending to driver */
        if (_currentTxLen > 0) {
            if (SIMDriver_Execute((size_t) _currentTxLen, cmdInfo->timeoutMs)) {
                //                LOG_DEBUG("%s - %s:\t Sended\r\n", __TAG__, __func__);F
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
//                LOG_DEBUG("%s - %s:\t Receive %s\r\n", __TAG__, __func__, (char *) rxbuf);

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
//            LOG_DEBUG("%s - %s:\t Timeout\r\n", __TAG__, __func__);
            _handleErrorOrTimeout();
        }
    }
}