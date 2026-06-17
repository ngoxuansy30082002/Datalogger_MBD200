#include "sim/sim_general.h"
#include "sim_driver.h"
#include "sim_basic.h"
#include "sim_net.h"

static const char * __TAG__ = "SIMNET";

static int _cmdBuilder(int state, char* buffer, size_t maxLen, const char* format);
static bool _respParser(int state, char* buffer, size_t maxLen);

static const SIM_CMD_SEQ _cmdTable[SIM_NET_COUNT] = {
    /* { cmd, builderFunc, respOk, respFail, timeoutMs, attempts, parserFunc, nextStateOk, nextStateFail } */
    [SIM_NET_IDLE] =
    { NULL, NULL, NULL, NULL, 0, 0, NULL, SIM_NET_IDLE, SIM_NET_IDLE},

    [SIM_NET_DEFINE_PDP] =
    { "AT+QICSGP=%u,1,\"%s\",\"%s\",\"%s\",1\r\n", _cmdBuilder, "OK", "ERROR", 300, 5, NULL, SIM_NET_ACTIVE_PDP, SIM_NET_DEACTIVE_PDP},

    [SIM_NET_DEACTIVE_PDP] =
    { "AT+QIDEACT=%u\r\n", _cmdBuilder, "OK", "ERROR", 40000, 3, _respParser, SIM_NET_DEFINE_PDP, SIM_NET_ERROR},

    [SIM_NET_ACTIVE_PDP] =
    { "AT+QIACT=%u\r\n", _cmdBuilder, "OK", "ERROR", 150000, 3, NULL, SIM_NET_CHECK_ACTIVE, SIM_NET_DEACTIVE_PDP},

    [SIM_NET_CHECK_ACTIVE] =
    { "AT+QIACT?\r\n", NULL, "OK", "ERROR", 1000, 3, _respParser, SIM_NET_READY, SIM_NET_DEFINE_PDP},

    [SIM_NET_DEACTIVE_PDP_STOP] =
    { "AT+QIDEACT=%u\r\n", _cmdBuilder, "OK", "ERROR", 40000, 3, _respParser, SIM_NET_IDLE, SIM_NET_IDLE},
};


static SIM_NET_STATE _currentState = 0;
static bool _isWaitingResp = false;
static bool _isBuilded = false;
static int _currentTxLen = 0;
static uint8_t _attemptCount = 0;
static uint8_t _activeFailCount = 0;

static int _cmdBuilder(int state, char* buffer, size_t maxLen, const char* format) {
    if (buffer == NULL || maxLen == 0)
        return 0;

    switch (state) {
        case SIM_NET_DEFINE_PDP:
            return snprintf(buffer, maxLen, format,
                    SIM_CONTEXT_ID, gAppCfg.gsm.APN, gAppCfg.gsm.usernameAPN, gAppCfg.gsm.passwordAPN);
        case SIM_NET_DEACTIVE_PDP:
        case SIM_NET_DEACTIVE_PDP_STOP:
        case SIM_NET_ACTIVE_PDP:
            return snprintf(buffer, maxLen, format, SIM_CONTEXT_ID);

        default: return snprintf(buffer, maxLen, "%s", format);
    }
}

static bool _respParser(int state, char* buffer, size_t maxLen) {
    if (buffer == NULL)
        return false;

    switch (state) {
        case SIM_NET_CHECK_ACTIVE:
        {
            char pattern[20];
            snprintf(pattern, sizeof (pattern), "+QIACT: %d,1", SIM_CONTEXT_ID);
            if (strstr(buffer, pattern) != NULL) {
                SYS_CONSOLE_PRINT("%s - %s:\t Net active OKK: %s\r\n", __TAG__, __func__, (char *) buffer);
                return true;
            }
            return false;
        }

        case SIM_NET_DEACTIVE_PDP:
            if (++_activeFailCount > 5) return false;
            return true;

        default: return true;
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

static void _initialize(void) {
    _isWaitingResp = false;
    _isBuilded = false;
    _currentTxLen = 0;
    _attemptCount = 0;
    _activeFailCount = 0;
}

bool SIMNet_Start(bool restart) {
    if (!restart && _currentState > SIM_NET_IDLE)
        return false;

    _initialize();
    _currentState = SIM_NET_DEFINE_PDP;

    return true;
}

void SIMNet_Stop(void) {
    if (_currentState == SIM_NET_IDLE)
        return;

    _initialize();
    _currentState = SIM_NET_DEACTIVE_PDP_STOP;
}

bool SIMNet_IsReady(void) {
    return (_currentState == SIM_NET_READY);
}

bool SIMNet_HasError(void) {
    return (_currentState == SIM_NET_ERROR);
}

void SIMNet_Process(void) {
    if (!SIMBasic_IsReady() ||
            _currentState == SIM_NET_IDLE ||
            _currentState == SIM_NET_READY ||
            _currentState == SIM_NET_ERROR) {
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
            //            SYS_CONSOLE_PRINT("%s - %s:\t Timeout\r\n", __TAG__, __func__);
            _handleErrorOrTimeout();
        }
    }
}