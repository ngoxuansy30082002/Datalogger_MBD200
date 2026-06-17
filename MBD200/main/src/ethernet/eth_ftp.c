#include "eth_ftp.h"

static const char * __TAG__ = "ETH_FTP";
/* Static Variables */
static TCPIP_FTPC_CTRL_CONN_TYPE _ctrlConn;
static IP_MULTI_ADDRESS _srvAddr;
static TCPIP_FTPC_DATA_CONN_TYPE _dataConn;
static TCPIP_FTPC_FILE_OPT_TYPE _fileOpt;

static ETH_FTP_INTERNAL_STATE _ftpState = FTP_STATE_IDLE;
static ETH_FTP_RESULT _ftpResult = {false, ETH_FTP_SERVER_IDLE, ETH_FTP_SERVER_IDLE};

static bool _targetFtp1 = false;
static bool _targetFtp2 = false;
static uint8_t _currentServerIdx = 0;
static uint8_t _retryCount = 0;

static TCPIP_FTPC_CONN_HANDLE_TYPE _ftpHandle = NULL;

/* Callback synchronization flags */
static bool _isCmdPending = false;
static bool _isCtrlEventDone = false;
static bool _isCtrlSuccess = false;
static bool _isDataEventDone = false;
static bool _isDataSuccess = false;

/* Data pointer for sending */
static uint32_t _dataSentOffset = 0;

/* Variables for nested directory creation */
static char _fullDirPath[DIR_PATH_LEN + 20];
static char _pathBuffer[DIR_PATH_LEN + 20];
static char* _currentToken = NULL;

/* * Control Socket Callback 
 * Triggered by Harmony TCP/IP stack when control commands finish
 */
static void _FtpCtrlCallback(TCPIP_FTPC_CONN_HANDLE_TYPE ftpcHandle,
        TCPIP_FTPC_CTRL_EVENT_TYPE ftpcEvent,
        TCPIP_FTPC_CMD cmd, char * ctrlbuff, uint16_t ctrllen) {

    if (ftpcEvent == TCPIP_FTPC_CTRL_EVENT_SUCCESS) {
        _isCtrlSuccess = true;
        _isCtrlEventDone = true;
    } else if (ftpcEvent == TCPIP_FTPC_CTRL_EVENT_FAILURE ||
            ftpcEvent == TCPIP_FTPC_CTRL_EVENT_DISCONNECTED) {
        _isCtrlSuccess = false;
        _isCtrlEventDone = true;
    }
}

/* * Data Socket Callback 
 * Triggered by Harmony TCP/IP stack when ready to send file data
 */
static bool _FtpDataCallback(TCPIP_FTPC_CONN_HANDLE_TYPE ftpcHandle,
        TCPIP_FTPC_DATA_EVENT_TYPE ftpcEvent,
        TCPIP_FTPC_CMD cmd, char * databuff, uint16_t * datalen) {

    if (ftpcEvent == TCPIP_FTPC_DATA_SEND_READY) {

        uint32_t totalSize = FileMgr_GetUploadFileSize();
        const char* fileContent = FileMgr_GetUploadFileData();

        uint32_t remaining = totalSize - _dataSentOffset;
        uint16_t chunk = *datalen; // Max size stack allows us to send right now

        if (remaining < chunk) {
            chunk = (uint16_t) remaining;
        }

        if (chunk > 0) {
            memcpy(databuff, &fileContent[_dataSentOffset], chunk);
            _dataSentOffset += chunk;
        }

        *datalen = chunk; // Tell stack how much we actually put in buffer
        return true; // true means we processed the data
    } else if (ftpcEvent == TCPIP_FTPC_DATA_SEND_DONE) {
        _isDataSuccess = true;
        _isDataEventDone = true;
        return true;
    }

    return true;
}

/* * API: Initialize
 */
void EthFtp_Initialize(void) {
    _ftpState = FTP_STATE_IDLE;
    _ftpResult.isUploading = false;
    _ftpResult.server1 = ETH_FTP_SERVER_IDLE;
    _ftpResult.server2 = ETH_FTP_SERVER_IDLE;
    _targetFtp1 = false;
    _targetFtp2 = false;
}

/* * API: Trigger Upload
 */
void EthFtp_TriggerUpload(bool ftp1, bool ftp2) {
    _targetFtp1 = ftp1;
    _targetFtp2 = ftp2;

    if (_ftpState == FTP_STATE_IDLE && (ftp1 || ftp2)) {

        /* Set general uploading flag */
        _ftpResult.isUploading = true;

        /* Reset status for triggered servers */
        if (ftp1) _ftpResult.server1 = ETH_FTP_SERVER_IDLE;
        if (ftp2) _ftpResult.server2 = ETH_FTP_SERVER_IDLE;

        if (ftp1) {
            _currentServerIdx = 0;
            _targetFtp1 = false;
        } else {
            _currentServerIdx = 1;
            _targetFtp2 = false;
        }
        _ftpState = FTP_STATE_INIT_SERVER;
    }
}

ETH_FTP_RESULT EthFtp_GetStatus(void) {
    return _ftpResult;
}

/* * Main Non-blocking Task
 */
void EthFtp_Task(void) {

    FTP_SERVER_CONFIG* srvCfg = &gAppCfg.ftpServer[_currentServerIdx];

    switch (_ftpState) {
        case FTP_STATE_IDLE:
            /* Do nothing, wait for trigger */
            break;

        case FTP_STATE_INIT_SERVER:
            if (!srvCfg->enable) {
                _isDataSuccess = false;
                _ftpState = FTP_STATE_EVALUATE_NEXT;
                break;
            }
            _retryCount = 0;
            _ftpState = FTP_STATE_CONNECT;
            break;

        case FTP_STATE_CONNECT:
            if (!_isCmdPending) {
                TCPIP_FTPC_RETURN_TYPE ret;

                /* Convert hostname string to IP address */
                TCPIP_Helper_StringToIPAddress(srvCfg->hostname, &_srvAddr.v4Add);
                //                LOG_DEBUG("%s - %s\t Host name: %s - IP: %u.%u.%u.%u", __TAG__, __func__, srvCfg->hostname,
                //                        _srvAddr.v4Add.v[0], _srvAddr.v4Add.v[1], _srvAddr.v4Add.v[2], _srvAddr.v4Add.v[3]);

                _ctrlConn.ftpcServerAddr = &_srvAddr;
                _ctrlConn.ftpcServerIpAddrType = IP_ADDRESS_TYPE_IPV4;
                _ctrlConn.serverCtrlPort = srvCfg->port;

                _ftpHandle = TCPIP_FTPC_Connect(&_ctrlConn, _FtpCtrlCallback, &ret);

                if (_ftpHandle != NULL) {
                    _isCmdPending = true;
                    _isCtrlEventDone = false;
                } else {
                    _ftpState = FTP_STATE_RETRY_DELAY; /* Failed to allocate handle */
                }
            } else if (_isCtrlEventDone) {
                _isCmdPending = false;
                if (_isCtrlSuccess) _ftpState = FTP_STATE_LOGIN;
                else _ftpState = FTP_STATE_RETRY_DELAY;
            }
            break;

        case FTP_STATE_LOGIN:
            if (!_isCmdPending) {
                if (TCPIP_FTPC_Login(_ftpHandle, srvCfg->username, srvCfg->password, NULL) == TCPIP_FTPC_RET_OK) {
                    _isCmdPending = true;
                    _isCtrlEventDone = false;
                }
            } else if (_isCtrlEventDone) {
                _isCmdPending = false;
                if (_isCtrlSuccess) _ftpState = FTP_STATE_PREPARE_PATH;
                else _ftpState = FTP_STATE_DISCONNECT;
            }
            break;

        case FTP_STATE_PREPARE_PATH:
        {
            TIME fTime = FileMgr_GetUploadFileTime();
            size_t len;

            /* Copy base directory path */
            strncpy(_fullDirPath, srvCfg->dirPath, sizeof (_fullDirPath) - 1);
            _fullDirPath[sizeof (_fullDirPath) - 1] = '\0';

            /* Remove trailing slash for clean appending */
            len = strlen(_fullDirPath);
            if (len > 0 && (_fullDirPath[len - 1] == '/' || _fullDirPath[len - 1] == '\\')) {
                _fullDirPath[len - 1] = '\0';
            }

            /* Append dynamic folder based on makeFolder setting */
            if (srvCfg->makeFolder == MAKE_FOLDER_DAY) {
                snprintf(_fullDirPath + strlen(_fullDirPath),
                        sizeof (_fullDirPath) - strlen(_fullDirPath),
                        "/%04u%02u%02u", fTime.year, fTime.month, fTime.day);
            } else if (srvCfg->makeFolder == MAKE_FOLDER_MONTH) {
                snprintf(_fullDirPath + strlen(_fullDirPath),
                        sizeof (_fullDirPath) - strlen(_fullDirPath),
                        "/%04u%02u/%04u%02u%02u", fTime.year, fTime.month, fTime.year, fTime.month, fTime.day);
            }

            //            LOG_DEBUG("%s - %s\t Path %s", __TAG__, __func__, _fullDirPath);
            _ftpState = FTP_STATE_CWD_ROOT;
            break;
        }

        case FTP_STATE_CWD_ROOT:
            if (!_isCmdPending) {
                if (TCPIP_FTPC_Change_Dir(_ftpHandle, _fullDirPath) == TCPIP_FTPC_RET_OK) {
                    _isCmdPending = true;
                    _isCtrlEventDone = false;
                }
            } else if (_isCtrlEventDone) {
                _isCmdPending = false;
                if (_isCtrlSuccess) {
                    /* CWD success, folder exists -> Proceed to Put File */
                    _ftpState = FTP_STATE_PUT_FILE;
                } else {
                    /* CWD failed, folder doesn't exist -> Fallback to MKD mode */
                    _ftpState = FTP_STATE_MKD_START;
                }
            }
            break;

        case FTP_STATE_MKD_START:
            /* Copy _fullDirPath into _pathBuffer for strtok processing 
             * (because strtok modifies the original string) */
            strncpy(_pathBuffer, _fullDirPath, sizeof (_pathBuffer) - 1);
            _pathBuffer[sizeof (_pathBuffer) - 1] = '\0';

            _currentToken = strtok(_pathBuffer, "/\\");
            if (_currentToken != NULL) {
                _ftpState = FTP_STATE_MKD_DO;
            } else {
                _ftpState = FTP_STATE_PUT_FILE;
            }
            break;

        case FTP_STATE_MKD_DO:
            if (!_isCmdPending) {
                if (TCPIP_FTPC_MakeDir(_ftpHandle, _currentToken) == TCPIP_FTPC_RET_OK) {
                    _isCmdPending = true;
                    _isCtrlEventDone = false;
                }
            } else if (_isCtrlEventDone) {
                _isCmdPending = false;
                /* Note: We ignore _isCtrlSuccess here because MKD might fail 
                 * if the folder already exists, which is fine. We proceed to CWD. */
                _ftpState = FTP_STATE_MKD_CWD;
            }
            break;

        case FTP_STATE_MKD_CWD:
            if (!_isCmdPending) {
                if (TCPIP_FTPC_Change_Dir(_ftpHandle, _currentToken) == TCPIP_FTPC_RET_OK) {
                    _isCmdPending = true;
                    _isCtrlEventDone = false;
                }
            } else if (_isCtrlEventDone) {
                _isCmdPending = false;
                if (_isCtrlSuccess) {
                    /* Get next nested folder */
                    _currentToken = strtok(NULL, "/\\");
                    if (_currentToken != NULL) {
                        _ftpState = FTP_STATE_MKD_DO; /* Iterate deeper */
                    } else {
                        _ftpState = FTP_STATE_PUT_FILE; /* Done making path */
                    }
                } else {
                    /* CWD failed completely -> Abort */
                    _ftpState = FTP_STATE_DISCONNECT;
                }
            }
            break;

        case FTP_STATE_PUT_FILE:
            if (!_isCmdPending) {
                TCPIP_FTPC_DATA_CONN_TYPE dataConn;
                TCPIP_FTPC_FILE_OPT_TYPE fileOpt;

                /* Config Passive, Stream, Image(Binary) mode */
                memset(&dataConn, 0, sizeof (dataConn));
                dataConn.ftpcIsPassiveMode = true;
                dataConn.ftpcTransferMode = TCPIP_FTPC_TRANS_STREAM_MODE;
                dataConn.ftpcDataType = TCPIP_FTPC_DATA_REP_IMAGE;

                /* Target File Name */
                fileOpt.serverPathName = (char*) FileMgr_GetUploadFileName();
                fileOpt.clientPathName = NULL;
                fileOpt.store_unique = false;

                _dataSentOffset = 0;

                TCPIP_FTPC_RETURN_TYPE ret = TCPIP_FTPC_PutFile(_ftpHandle, &dataConn, &fileOpt, _FtpDataCallback);

                if (ret == TCPIP_FTPC_RET_OK) {
                    _isCmdPending = true;
                    _isDataEventDone = false;
                    _isDataSuccess = false;

                    /* Reset Control flags to catch PASV/PORT connection failures 
                     * that might occur before the Data socket even opens. */
                    _isCtrlEventDone = false;
                    _isCtrlSuccess = false;
                } else if (ret == TCPIP_FTPC_RET_BUSY) {
                    /* Harmony FTP Client is busy preparing internal states. Wait for next tick. */
                } else {
                    _ftpState = FTP_STATE_DISCONNECT;
                }
            } else {
                /* We are pending. Check for Data completion OR Control failure. */
                if (_isDataEventDone) {
                    _isCmdPending = false;
                    _ftpState = FTP_STATE_DISCONNECT;
                } else if (_isCtrlEventDone && !_isCtrlSuccess) {
                    /* PASV setup failed or connection dropped during transfer. 
                     * Abort the put operation to prevent deadlock. */
                    _isCmdPending = false;
                    _isDataSuccess = false; /* Mark transfer as failed */
                    _ftpState = FTP_STATE_DISCONNECT;
                }
            }
            break;

        case FTP_STATE_DISCONNECT:
        {
            TCPIP_FTPC_RETURN_TYPE ret = TCPIP_FTPC_Disconnect(_ftpHandle);

            if (ret == TCPIP_FTPC_RET_OK) {
                if (_isDataSuccess) {
                    _retryCount = 0;
                    _ftpState = FTP_STATE_EVALUATE_NEXT;
                } else {
                    _ftpState = FTP_STATE_RETRY_DELAY;
                }
            }
            break;
        }

        case FTP_STATE_RETRY_DELAY:
            _retryCount++;
            if (_retryCount <= MAX_FTP_RETRIES) {
                _isCmdPending = false;
                _isCtrlEventDone = false;
                _ftpState = FTP_STATE_CONNECT;
            } else {
                _retryCount = 0;
                _isDataSuccess = false; /* Force failure mark */
                _ftpState = FTP_STATE_EVALUATE_NEXT;
            }
            break;

        case FTP_STATE_EVALUATE_NEXT:
            /* 1. Record the result of the completed server */
            if (_isDataSuccess) {
                if (_currentServerIdx == 0) _ftpResult.server1 = ETH_FTP_SERVER_SUCCESS;
                else _ftpResult.server2 = ETH_FTP_SERVER_SUCCESS;
            } else {
                if (_currentServerIdx == 0) _ftpResult.server1 = ETH_FTP_SERVER_FAILED;
                else _ftpResult.server2 = ETH_FTP_SERVER_FAILED;
            }

            /* 2. Check if the next server is pending */
            if (_targetFtp1) {
                _currentServerIdx = 0;
                _targetFtp1 = false;
                _ftpState = FTP_STATE_INIT_SERVER;
            } else if (_targetFtp2) {
                _currentServerIdx = 1;
                _targetFtp2 = false;
                _ftpState = FTP_STATE_INIT_SERVER;
            } else {
                /* 3. Both servers handled. Clear uploading flag */
                _ftpResult.isUploading = false;
                _ftpState = FTP_STATE_IDLE;
            }
            break;

        default:
            _ftpState = FTP_STATE_IDLE;
            break;
    }
}