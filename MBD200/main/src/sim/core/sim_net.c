#include "sim/sim_general.h"
#include "sim_driver.h"
#include "sim_basic.h"
#include "sim_net.h"

static const char * __TAG__ = "SIMNET";

static int _cmdBuilder(int state, char* buffer, size_t maxLen, const char* format);
static bool _respParser(int state, char* buffer, size_t maxLen);

static const SIM_CMD_SEQ _cmdTable[] = {
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
            return snprintf(buffer, maxLen, format, SIM_CONTEXT_ID);
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
            if (strstr(buffer, pattern) != NULL) return true;
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

void SIMNet_Initialize() {
    _currentState = SIM_NET_DEFINE_PDP;

    _isWaitingResp = false;
    _isBuilded = false;
    _currentTxLen = 0;
    _attemptCount = 0;
    _activeFailCount = 0;
}

bool SIMNet_IsReady(void) {
    return (_currentState == SIM_NET_READY);
}

bool SIMNet_HasError(void) {
    return (_currentState == SIM_NET_ERROR);
}

void SIMNet_Process(void) {
    if (!SIMBasic_IsReady() ||
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

            //            SYS_CONSOLE_PRINT("%s - %s:\t Builed: %s\r\n", __TAG__, __func__, (char *) tx_buf);
            _isBuilded = true; /* Mark as built */
        }

        /* Execute sending to driver */
        if (_currentTxLen > 0) {
            if (SIMDriver_Execute((size_t) _currentTxLen, cmdInfo->timeoutMs)) {
                //                SYS_CONSOLE_PRINT("%s - %s:\t Sended\r\n", __TAG__, __func__);
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
                _isWaitingResp = false;
                //                SYS_CONSOLE_PRINT("%s - %s:\t Receive %s\r\n", __TAG__, __func__, (char *) rx_buf);

                const char* expected_ok = _cmdTable[_currentState].respOk;
                const char* expected_fail = _cmdTable[_currentState].respFail;

                if (strstr((char*) rx_buf, expected_ok) != NULL) {
                    /* Success: move to next state and reset retry counter */
                    bool parsed = true;
                    if (_cmdTable[_currentState].parserFunc)
                        parsed = _cmdTable[_currentState].parserFunc(_currentState, rx_buf, SIM_TRANSFER_BUFF_SIZE);
                    if (parsed) {
                        _attemptCount = 0;
                        _currentState = _cmdTable[_currentState].nextStateOk;
                    } else
                        _handleErrorOrTimeout();
                } else if (strstr((char*) rx_buf, expected_fail) != NULL)
                    _handleErrorOrTimeout();
                else
                    _handleErrorOrTimeout();
            }
        } else if (status == SIM_DRV_STATUS_TIMEOUT) {
            SYS_CONSOLE_PRINT("%s - %s:\t Timeout\r\n", __TAG__, __func__);
            _handleErrorOrTimeout();
        }
    }
}