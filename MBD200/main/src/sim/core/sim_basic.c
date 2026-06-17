#include "sim/sim_general.h"
#include "sim_basic.h"
#include "sim_driver.h"

static const char * __TAG__ = "SIMBASIC";

static int _cmdBuilder(int state, char* buffer, size_t maxLen, const char* format);
static bool _respParser(int state, char* buffer, size_t maxLen);

static const SIM_CMD_SEQ _cmdTable[SIM_BASIC_COUNT] = {
    /* { cmd, builderFunc, respOk, respFail, timeoutMs, attempts , parserFunc, nextStateOk, nextStateFail } */
    [SIM_BASIC_IDLE] =
    { NULL, NULL, NULL, NULL, 0, 0, NULL, SIM_BASIC_IDLE, SIM_BASIC_IDLE},
    /* 1. "AT\r\n" */
    [SIM_BASIC_AT] =
    { "AT\r\n", NULL, "OK", "ERROR", 300, 10, NULL, SIM_BASIC_ATE0, SIM_BASIC_ERROR},
    /* 2. "ATE0\r\n" */
    [SIM_BASIC_ATE0] =
    { "ATE0\r\n", NULL, "OK", "ERROR", 300, 5, NULL, SIM_BASIC_QSIMSTAT, SIM_BASIC_ERROR},
    /* 3. "AT+QSIMSTAT=0\r\n" */
    [SIM_BASIC_QSIMSTAT] =
    { "AT+QSIMSTAT=0\r\n", NULL, "OK", "ERROR", 300, 3, NULL, SIM_BASIC_GSN, SIM_BASIC_ERROR},
    /* 4. "AT+GSN\r\n" */
    [SIM_BASIC_GSN] =
    { "AT+GSN\r\n", NULL, "OK", "ERROR", 300, 3, _respParser, SIM_BASIC_QDSIMCFG, SIM_BASIC_ERROR},
    /* 5. "AT+QDSIMCFG=\"dsss\",1\r\n" */
    [SIM_BASIC_QDSIMCFG] =
    { "AT+QDSIMCFG=\"dsss\",1\r\n", NULL, "OK", "ERROR", 300, 3, NULL, SIM_BASIC_QDSIM, SIM_BASIC_ERROR},
    /* 6. "AT+QDSIM=%u\r\n" */
    [SIM_BASIC_QDSIM] =
    { "AT+QDSIM=%u\r\n", _cmdBuilder, "OK", "ERROR", 5000, 3, NULL, SIM_BASIC_QSIMSTAT_QUERY, SIM_BASIC_ERROR},
    /* 7. "AT+QSIMSTAT?\r\n" */
    [SIM_BASIC_QSIMSTAT_QUERY] =
    { "AT+QSIMSTAT?\r\n", NULL, "OK", "ERROR", 1000, 50, _respParser, SIM_BASIC_QCCID, SIM_BASIC_ERROR},
    /* 8. "AT+QCCID\r\n" */
    [SIM_BASIC_QCCID] =
    { "AT+QCCID\r\n", NULL, "OK", "ERROR", 5000, 10, _respParser, SIM_BASIC_CREG, SIM_BASIC_ERROR},
    /* 9. "AT+CREG?\r\n" */
    [SIM_BASIC_CREG] =
    { "AT+CREG?\r\n", NULL, "OK", "ERROR", 300, 50, _respParser, SIM_BASIC_QSPN, SIM_BASIC_ERROR},
    /* 10. "AT+QSPN\r\n" */
    [SIM_BASIC_QSPN] =
    { "AT+QSPN\r\n", NULL, "OK", "ERROR", 300, 10, _respParser, SIM_BASIC_CMGF, SIM_BASIC_ERROR},
    /* 11. "AT+CMGF=1\r\n" */
    [SIM_BASIC_CMGF] =
    { "AT+CMGF=1\r\n", NULL, "OK", "ERROR", 300, 3, NULL, SIM_BASIC_CSQ, SIM_BASIC_ERROR},
    /* 12. "AT+CSQ\r\n" */
    [SIM_BASIC_CSQ] =
    { "AT+CSQ\r\n", NULL, "OK", "ERROR", 1000, 3, _respParser, SIM_BASIC_READY, SIM_BASIC_ERROR}
};

static SIM_BASIC_STATE _currentState = 0;
static SIM_BASIC_INFO _simInfo = {0};
static uint8_t _currentSimSlot = 1;
static bool _isWaitingResp = false;
static bool _isBuilded = false;
static int _currentTxLen = 0;
static uint8_t _attemptCount = 0;
static uint8_t _resetCount = 0;

static int _cmdBuilder(int state, char* buffer, size_t maxLen, const char* format) {
    switch (state) {
        case SIM_BASIC_QDSIM:
            return snprintf(buffer, maxLen, format, _currentSimSlot);
        default:
            return snprintf(buffer, maxLen, "%s", format);
    }
}

static bool _respParser(int state, char* buffer, size_t maxLen) {
    char *ptr = NULL;

    switch (state) {
        case SIM_BASIC_GSN:
            /* 1. Get IMEI: Response is usually a numeric string followed by OK.
             * Example: "\r\n860181061037724\r\n\r\nOK"
             * Skip whitespace/newline characters to find the first digit.
             */
            ptr = buffer;
            while (*ptr && !isdigit((unsigned char) *ptr)) ptr++;
            if (isdigit((unsigned char) *ptr)) {
                int i = 0;
                while (isdigit((unsigned char) *ptr) && (i < 19)) {
                    _simInfo.imei[i] = *ptr;
                    i++;
                    ptr++;
                }
                _simInfo.imei[i] = '\0';
                if (i >= 15) {
                    LOG_DEBUG("%s - %s:\t IMEI: %s", __TAG__, __func__, _simInfo.imei);
                    return true;
                }
            }
            return false;

        case SIM_BASIC_QSIMSTAT_QUERY:
            /* 2. Check SIM presence: +QSIMSTAT: <urc>,<status> */
            ptr = strstr(buffer, "+QSIMSTAT:");
            if (ptr != NULL) {
                ptr += 10; /* Skip "+QSIMSTAT:" */
                while (*ptr == ' ') ptr++; /* Skip spaces */

                /* Skip <urc> field */
                while (isdigit((unsigned char) *ptr)) ptr++;

                if (*ptr == ',') {
                    ptr++; /* Skip comma */
                    while (*ptr == ' ') ptr++; /* Skip spaces */

                    if (isdigit((unsigned char) *ptr)) {
                        int status = *ptr - '0'; /* Read <status> */
                        if (status == 1) {
                            LOG_DEBUG("%s - %s:\t SIM inserted", __TAG__, __func__);
                            _simInfo.inserted = true;
                            return true; /* SIM detected */
                        } else {
                            _simInfo.inserted = false;
                            LOG_DEBUG("%s - %s:\t SIM NOT inserted", __TAG__, __func__);
                            return false; /* SIM not detected */
                        }
                    }
                }
            }
            return false;

        case SIM_BASIC_QCCID:
            /* 3. Parse CCID: +QCCID: <ccid_string> */
            ptr = strstr(buffer, "+QCCID:");
            if (ptr != NULL) {
                ptr += 7; /* Skip "+QCCID:" */
                while (*ptr == ' ') ptr++; /* Skip spaces */

                int i = 0;
                /* Read CCID until delimiter or max length */
                while (*ptr && *ptr != '\r' && *ptr != '\n' && *ptr != ' ' && (i < 24))
                    _simInfo.ccid[i++] = *ptr++;

                _simInfo.ccid[i] = '\0';

                if (i > 0) {
                    LOG_DEBUG("%s - %s:\t CCID: %s", __TAG__, __func__, _simInfo.ccid);
                    return true;
                }
            }
            return false;


        case SIM_BASIC_CREG:
            /* 4. Check network registration: +CREG: <n>,<stat> */
            ptr = strstr(buffer, "+CREG:");
            if (ptr != NULL) {
                ptr += 6; /* Skip "+CREG:" */
                while (*ptr == ' ') ptr++; /* Skip spaces */

                /* Skip <n> field */
                while (isdigit((unsigned char) *ptr)) ptr++;

                if (*ptr == ',') {
                    ptr++; /* Skip comma */
                    while (*ptr == ' ') ptr++; /* Skip spaces */

                    if (isdigit((unsigned char) *ptr)) {
                        int stat = 0;
                        /* Parse <stat> (can be multiple digits) */
                        while (isdigit((unsigned char) *ptr)) {
                            stat = stat * 10 + (*ptr - '0');
                            ptr++;
                        }

                        LOG_DEBUG("%s - %s:\t CREG Status: %d", __TAG__, __func__, stat);
                        if (stat == 1 || stat == 5) {
                            return true; /* Registered to network */
                        }
                    }
                }
            }
            return false;

        case SIM_BASIC_QSPN:
            /* 5. Parse network name: +QSPN: "<name>",... or +QSPN: <name>,... */
            ptr = strstr(buffer, "+QSPN:");
            if (ptr != NULL) {
                ptr += 6; /* Skip "+QSPN:" */
                while (*ptr == ' ') ptr++; /* Skip spaces */

                int i = 0;
                if (*ptr == '"') {
                    /* Quoted string format */
                    ptr++; /* Skip opening quote */
                    while (*ptr && *ptr != '"' && (i < 31)) {
                        _simInfo.networkName[i++] = *ptr++;
                    }
                } else {
                    /* Non-quoted format */
                    while (*ptr && *ptr != ',' && *ptr != '\r' && *ptr != '\n' && (i < 31)) {
                        _simInfo.networkName[i++] = *ptr++;
                    }
                }
                _simInfo.networkName[i] = '\0';

                if (i > 0) {
                    LOG_DEBUG("%s - %s:\t Network: %s", __TAG__, __func__, _simInfo.networkName);
                }
            }
            return true;

        case SIM_BASIC_CSQ:
            /* 6. Parse RSSI: +CSQ: <rssi>,<ber> */
            ptr = strstr(buffer, "+CSQ:");
            if (ptr != NULL) {
                ptr += 5; /* Skip "+CSQ:" */
                while (*ptr == ' ') ptr++; /* Skip spaces */

                if (isdigit((unsigned char) *ptr)) {
                    int rssi = 0;
                    while (isdigit((unsigned char) *ptr)) {
                        rssi = rssi * 10 + (*ptr - '0');
                        ptr++;
                    }
                    _simInfo.rssi = rssi;
                    LOG_DEBUG("%s - %s:\t RSSI: %d", __TAG__, __func__, _simInfo.rssi);
                    return true;
                }
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

        /* Requirement: Call SIMDriver_Reset() if entering error state */
        if (_currentState == SIM_BASIC_ERROR) {
            _resetCount++;
            SIMBasic_Initialize(0);
            if (_resetCount < 3)
                SIMDriver_Reset();
            else {
                _resetCount = 0;
                SIMDriver_TurnOff();
            }
        }
    }

    _isWaitingResp = false;
    _isBuilded = false;
}

void SIMBasic_Initialize(uint8_t sim_slot) {
    _currentSimSlot = sim_slot;
    _currentState = SIM_BASIC_AT;
    memset(&_simInfo, 0, sizeof (SIM_BASIC_INFO));

    _isWaitingResp = false;
    _isBuilded = false;
    _currentTxLen = 0;
    _attemptCount = 0;
}

bool SIMBasic_IsReady(void) {
    return (_currentState == SIM_BASIC_READY);
}

bool SIMBasic_HasError(void) {
    return (_currentState == SIM_BASIC_ERROR);
}

void SIMBasic_Process(void) {
    static uint32_t reScanTick = 0;
    static bool preDetect = false;

    SIM_HW_STATUS hwStatus = SIMDriver_GetHWStatus();
    if (hwStatus == SIM_HW_STATUS_POWERDOWN)
        SIMDriver_TurnOn();

    if (hwStatus != SIM_HW_STATUS_READY)
        return;

    bool currentDetect = SIMDriver_isCardDetect();
    bool scan = false;

    if (currentDetect == true && preDetect == false) {
        LOG_DEBUG("%s - %s:\t SIMDriver_isCardDetect", __TAG__, __func__);
        scan = true;
        reScanTick = TICK_NOW();
    }
    preDetect = currentDetect;

    if (!_simInfo.inserted && TIME_IS_EXPIRED(reScanTick, 60000)) {
        reScanTick = TICK_NOW();
        scan = true;
    }

    if (scan)
        SIMBasic_Initialize(0);

    if (hwStatus != SIM_HW_STATUS_READY ||
            _currentState == SIM_BASIC_IDLE ||
            _currentState == SIM_BASIC_READY ||
            _currentState == SIM_BASIC_ERROR) {
        return;
    }

    /* 1. COMMAND NOT SENT YET -> PREPARE AND SEND */
    if (!_isWaitingResp) {
        const SIM_CMD_SEQ * cmdInfo = &_cmdTable[_currentState];

        if (!_isBuilded) {
            uint8_t* tx_buf = SIMDriver_GetBuffer(SIM_DRV_TX_BUSY);
            if (tx_buf == NULL) return; /* Driver busy, wait next cycle */

            if (cmdInfo->builderFunc != NULL)
                _currentTxLen = cmdInfo->builderFunc((int) _currentState, (char*) tx_buf, SIM_TRANSFER_BUFF_SIZE, cmdInfo->cmd);
            else
                _currentTxLen = snprintf((char*) tx_buf, SIM_TRANSFER_BUFF_SIZE, "%s", cmdInfo->cmd);

            //            LOG_DEBUG("%s - %s:\t Builed: %s", __TAG__, __func__, (char *) tx_buf);
            _isBuilded = true; /* Mark as built */
        }

        /* Execute sending to driver */
        if (_currentTxLen > 0) {
            if (SIMDriver_Execute((size_t) _currentTxLen, cmdInfo->timeoutMs)) {
                //                LOG_DEBUG("%s - %s:\t Sended", __TAG__, __func__);
                _isWaitingResp = true;
                _isBuilded = false; /* Clear flag so next cycle/state can rebuild */
            }
        }
        return;
    } else { /* 2. COMMAND SENT -> WAIT FOR RESPONSE */
        SIM_DRV_STATUS status = SIMDriver_GetStatus();

        if (status == SIM_DRV_STATUS_RECV_RESP) {
            uint8_t* rx_buf = SIMDriver_GetBuffer(SIM_DRV_RX_BUSY);
            if (rx_buf != NULL) {
                //                LOG_DEBUG("%s - %s:\t Receive %s", __TAG__, __func__, (char *) rx_buf);

                const char* expected_ok = _cmdTable[_currentState].respOk;
                const char* expected_fail = _cmdTable[_currentState].respFail;

                if (strstr((char*) rx_buf, expected_ok) != NULL) {
                    _isWaitingResp = false;
                    /* Success: move to next state and reset retry counter */
                    bool parsed = true;
                    if (_cmdTable[_currentState].parserFunc)
                        parsed = _cmdTable[_currentState].parserFunc(_currentState, rx_buf, SIM_TRANSFER_BUFF_SIZE);
                    if (parsed) {
                        _attemptCount = 0;
                        _currentState = _cmdTable[_currentState].nextStateOk;
                    } else
                        _handleErrorOrTimeout();
                } else if (strstr((char*) rx_buf, expected_fail) != NULL) {
                    _isWaitingResp = false;
                    _handleErrorOrTimeout();
                }
            }
        } else if (status == SIM_DRV_STATUS_TIMEOUT) {
            //            LOG_DEBUG("%s - %s:\t Timeout", __TAG__, __func__);
            _handleErrorOrTimeout();
        }
    }
}

SIM_BASIC_INFO * SIMBasic_GetInfo(void) {
    return &_simInfo;
}