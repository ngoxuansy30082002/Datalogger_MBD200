#include "sim/sim_general.h"
#include "sim/core/sim_driver.h"
#include "sim/core/sim_basic.h"
#include "sim/core/sim_net.h"
#include "sim_ntp.h"

static const char * __TAG__ = "SIMNTP";
static const char * _ntpServer[7] = {
    "pool.ntp.org",
    "europe.pool.ntp.org",
    "asia.pool.ntp.org",
    "oceania.pool.ntp.org",
    "north-america.pool.ntp.org",
    "south-america.pool.ntp.org",
    "africa.pool.ntp.org"
};

static int _cmdBuilder(int state, char* buffer, size_t maxLen, const char* format);
static bool _respParser(int state, char* buffer, size_t maxLen);

static const SIM_CMD_SEQ _cmdTable[SIM_NTP_COUNT] = {
    /* { cmd, builderFunc, respOk, respFail, timeoutMs, attempts, parserFunc, nextStateOk, nextStateFail } */

    [SIM_NTP_IDLE] =
    { NULL, NULL, NULL, NULL, 0, 0, NULL, SIM_NTP_IDLE, SIM_NTP_IDLE},

    [SIM_NTP_SETUP_TIMEZONE] =
    { "AT+CCLK=\"04/01/01,00:00:02+00\"\r\n", NULL, "OK", "ERROR", 1000, 6, NULL, SIM_NTP_SYNC_TIME, SIM_NTP_ERROR},

    [SIM_NTP_SYNC_TIME] =
    {"AT+QNTP=%u,\"%s\",123\r\n", _cmdBuilder, "+QNTP: 0", "ERROR", 90000, 3, NULL, SIM_NTP_QUERY_TIME, SIM_NTP_ERROR},

    [SIM_NTP_QUERY_TIME] =
    { "AT+CCLK?\r\n", NULL, "+00\"", "ERROR", 1000, 3, _respParser, SIM_NTP_READY, SIM_NTP_ERROR},

};

static SIM_NTP_STATE _currentState = 0;
static bool _isWaitingResp = false;
static bool _isBuilded = false;
static int _currentTxLen = 0;
static uint8_t _attemptCount = 0;

static int _cmdBuilder(int state, char* buffer, size_t maxLen, const char* format) {
    if (buffer == NULL || maxLen == 0) return 0;

    switch (state) {
        case SIM_NTP_SYNC_TIME:
            return snprintf(buffer, maxLen, format,
                    SIM_CONTEXT_ID, _ntpServer[gAppCfg.time.indexNTP]);
        default: return snprintf(buffer, maxLen, "%s", format);
            return 0;
    }
}

static bool _respParser(int state, char* buffer, size_t maxLen) {
    if (buffer == NULL) return false;

    switch (state) {
        case SIM_NTP_QUERY_TIME:
        {
            /* +QNTP: 0,"yy/MM/dd,hh:mm:ss+zz" */
            char *startPtr = strchr(buffer, '\"');
            if (startPtr != NULL) {
                startPtr++;
                char *endPtr = strchr(startPtr, '\"');
                if (endPtr != NULL) {
                    *endPtr = '\0';

                    Rtc_updateFromGsmNtp(startPtr);
                    SYS_CONSOLE_PRINT("NTP Parsed Time: %s\r\n", startPtr);
                    return true;
                }
            }
            return false;
        }

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

bool SIMNtp_Start(void) {
    if (_currentState > SIM_NTP_IDLE)
        return false;

    _currentState = SIM_NTP_SETUP_TIMEZONE;
    _isWaitingResp = false;
    _isBuilded = false;
    _currentTxLen = 0;
    _attemptCount = 0;

    return true;
}

void SIMNtp_Abort(void) {
    _currentState = SIM_NTP_IDLE;
}

bool SIMNtp_IsReady(void) {
    return (_currentState == SIM_NTP_READY);
}

bool SIMNtp_HasError(void) {
    return (_currentState == SIM_NTP_ERROR);
}

void SIMNtp_Process(void) {
    if (!SIMBasic_IsReady() || !SIMNet_IsReady() ||
            _currentState == SIM_NTP_IDLE ||
            _currentState == SIM_NTP_READY ||
            _currentState == SIM_NTP_ERROR) {
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

            //            SYS_CONSOLE_PRINT("%s - %s:\t Builed: %s\r\n", __TAG__, __func__, (char *) txbuf);
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
                //                SYS_CONSOLE_PRINT("%s - %s:\t Receive %s\r\n", __TAG__, __func__, (char *) rxbuf);

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