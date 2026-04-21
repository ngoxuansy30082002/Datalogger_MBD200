#include "sim/sim_general.h"
#include "sim/core/sim_driver.h"
#include "sim/core/sim_basic.h"
#include "sim/core/sim_net.h"
#include "sim_mqtt.h"

typedef struct {
    uint8_t client_idx;
    const char* host;
    uint16_t port;
    const char* client_id;
    const char* username;
    const char* password;
} MQTT_CONFIG_T;

static MQTT_CONFIG_T _mqttCfg = {
    .client_idx = 0,
    .host = "broker.emqx.io",
    .port = 1883,
    .client_id = "clientExample",
    .username = "user123",
    .password = "pass123"
};

static char _pubTopic[64];
static char _pubPayload[256];
static int _pubLen = 0;
static uint16_t _pubMsgId = 1;

static const char * __TAG__ = "SIMMQTT";

static int _cmdBuilder(int state, char* buffer, size_t maxLen, const char* format);
static bool _respParser(int state, char* buffer, size_t maxLen);

static const SIM_CMD_SEQ _cmdTable[SIM_MQTT_COUNT] = {
    /* { cmd, builderFunc, respOk, respFail, timeoutMs, attempts, parserFunc, nextStateOk, nextStateFail } */

    [SIM_MQTT_IDLE] =
    { NULL, NULL, NULL, NULL, 0, 0, NULL, SIM_MQTT_IDLE, SIM_MQTT_IDLE},

    [SIM_MQTT_CFG_RECV] =
    { "AT+QMTCFG=\"recv/mode\",%u,0,1\r\n", _cmdBuilder, "OK", "ERROR", 2000, 3, NULL, SIM_MQTT_OPEN, SIM_MQTT_ERROR},

    [SIM_MQTT_OPEN] =
    { "AT+QMTOPEN=%u,\"%s\",%d\r\n", _cmdBuilder, "+QMTOPEN: ", "ERROR", 120000, 1, _respParser, SIM_MQTT_CONN, SIM_MQTT_ERROR},

    [SIM_MQTT_CONN] =
    { "AT+QMTCONN=%u,\"%s\",\"%s\",\"%s\"\r\n", _cmdBuilder, "+QMTCONN: ", "ERROR", 15000, 1, _respParser, SIM_MQTT_SUB, SIM_MQTT_ERROR},

    [SIM_MQTT_SUB] =
    { "AT+QMTSUB=%u,1,\"/test\",1\r\n", _cmdBuilder, "+QMTSUB: ", "ERROR", 15000, 3, _respParser, SIM_MQTT_READY, SIM_MQTT_ERROR},

    [SIM_MQTT_PUB_CMD] =
    { "AT+QMTPUBEX=%u,%d,1,0,\"%s\",%d\r\n", _cmdBuilder, ">", "ERROR", 5000, 1, NULL, SIM_MQTT_PUB_DATA, SIM_MQTT_READY},

    [SIM_MQTT_PUB_DATA] =
    { NULL, _cmdBuilder, "+QMTPUBEX: ", "ERROR", 15000, 1, _respParser, SIM_MQTT_READY, SIM_MQTT_READY},
};

static SIM_MQTT_STATE _currentState = 0;
static bool _isWaitingResp = false;
static bool _isBuilded = false;
static int _currentTxLen = 0;
static uint8_t _attemptCount = 0;

static int _cmdBuilder(int state, char* buffer, size_t maxLen, const char* format) {
    if (buffer == NULL || maxLen == 0) return 0;

    switch (state) {
        case SIM_MQTT_CFG_RECV:
            return snprintf(buffer, maxLen, format,
                    _mqttCfg.client_idx);

        case SIM_MQTT_OPEN:
            return snprintf(buffer, maxLen, format,
                    _mqttCfg.client_idx, _mqttCfg.host, _mqttCfg.port);

        case SIM_MQTT_CONN:
            return snprintf(buffer, maxLen, format,
                    _mqttCfg.client_idx, _mqttCfg.client_id, _mqttCfg.username, _mqttCfg.password);

        case SIM_MQTT_SUB:
            return snprintf(buffer, maxLen, format,
                    _mqttCfg.client_idx);

        case SIM_MQTT_PUB_CMD:
            return snprintf(buffer, maxLen, format,
                    _mqttCfg.client_idx, ++_pubMsgId, _pubTopic, _pubLen);

        case SIM_MQTT_PUB_DATA:
            return snprintf(buffer, maxLen, "%s",
                    _pubPayload);

        default:
            if (format) return snprintf(buffer, maxLen, "%s", format);
            return 0;
    }
}

static bool _respParser(int state, char* buffer, size_t maxLen) {
    if (buffer == NULL) return false;

    int clientIdx = -1, result = -1, retCode = -1, msgId = -1, val = -1;
    int parsedItems = 0;
    char * pMatch = NULL;

    switch (state) {
        case SIM_MQTT_OPEN:
            /* +QMTOPEN: <client_idx>,<result> */
            pMatch = strstr(buffer, "+QMTOPEN:");
            if (pMatch != NULL) {
                parsedItems = sscanf(pMatch, "+QMTOPEN: %d,%d", &clientIdx, &result);
                SYS_CONSOLE_PRINT("%s - %s:\t SIM_MQTT_OPEN: result=%d, client=%d\r\n", __TAG__, __func__, result, clientIdx);
                if (parsedItems == 2 && clientIdx == _mqttCfg.client_idx && result == 0)
                    return true;
            }
            return false;

        case SIM_MQTT_CONN:
            /* +QMTCONN: <client_idx>,<result>[,<ret_code>] */
            pMatch = strstr(buffer, "+QMTCONN:");
            if (pMatch != NULL) {
                parsedItems = sscanf(pMatch, "+QMTCONN: %d,%d,%d", &clientIdx, &result, &retCode);
                SYS_CONSOLE_PRINT("%s - %s:\t SIM_MQTT_CONN: result=%d, retCode=%d\r\n", __TAG__, __func__, result, retCode);
                if (parsedItems == 3 && clientIdx == _mqttCfg.client_idx && result == 0 && retCode == 0)
                    return true;
            }
            return false;

        case SIM_MQTT_SUB:
            pMatch = strstr(buffer, "+QMTSUB:");
            if (pMatch != NULL) {
                if (sscanf(pMatch, "+QMTSUB: %d,%d,%d,%d", &clientIdx, &msgId, &result, &val) >= 3) {
                    if (clientIdx == _mqttCfg.client_idx && result == 0)
                        return true;
                }
            }
            return false;

        case SIM_MQTT_PUB_DATA:
            pMatch = strstr(buffer, "+QMTPUBEX:");
            if (pMatch != NULL) {
                if (sscanf(pMatch, "+QMTPUBEX: %d,%d,%d", &clientIdx, &msgId, &result) >= 3) {
                    if (clientIdx == _mqttCfg.client_idx && result == 0)
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
    }

    _isWaitingResp = false;
    _isBuilded = false;
}

static void _handleIncomingMessage(char* rxbuf) {
    char* pMatch = strstr(rxbuf, "+QMTRECV:");
    if (pMatch == NULL) return;

    // C?u trúc mong ??i: +QMTRECV: <client_idx>,<msgid>,"<topic>",<payload_len>,"<payload>"
    // Do ??c thù chu?i này, ta dùng strchr ?? tách chu?i an toàn h?n sscanf
    //    SYS_CONSOLE_PRINT("%s - INCOMING MSG:\r\n%s\r\n", __TAG__, pMatch);

    // (Tùy ch?n) B?n có th? vi?t thêm logic parse trích xu?t Topic và Payload ? ?ây 
    // ?? nhét vào Queue ??y lên cho t?ng Application x? lý.
}

bool SIMMqtt_Start(void) {
    if (_currentState > SIM_MQTT_IDLE)
        return false;

    _currentState = SIM_MQTT_CFG_RECV;
    _isWaitingResp = false;
    _isBuilded = false;
    _currentTxLen = 0;
    _attemptCount = 0;

    return true;
}

void SIMMqtt_Abort(void) {
    _currentState = SIM_MQTT_IDLE;
}

bool SIMMqtt_IsReady(void) {
    return (_currentState == SIM_MQTT_READY);
}

bool SIMMqtt_HasError(void) {
    return (_currentState == SIM_MQTT_ERROR);
}

void SIMMqtt_Process(void) {
    if (!SIMBasic_IsReady() || !SIMNet_IsReady() ||
            _currentState == SIM_MQTT_IDLE ||
            _currentState == SIM_MQTT_ERROR) {
        return;
    }


    if (_currentState == SIM_MQTT_READY) {
        SIM_DRV_STATUS status = SIMDriver_GetStatus();

        if (status == SIM_DRV_STATUS_RECV_RESP) {
            uint8_t* rxbuf = SIMDriver_GetBuffer(SIM_DRV_RX_BUSY);
            if (rxbuf != NULL) {
                SYS_CONSOLE_PRINT("%s - %s:\t Receive\r\n", __TAG__, __func__, (char *) rxbuf);

                if (strstr((char*) rxbuf, "+QMTRECV:") != NULL) {
                    _handleIncomingMessage((char*) rxbuf);
                } else if (strstr((char*) rxbuf, "+QMTSTAT:") != NULL) {
                    SYS_CONSOLE_PRINT("%s - MQTT DISCONNECTED BY SERVER!\r\n", __TAG__);
                    _currentState = SIM_MQTT_ERROR;
                }
            }
        }
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

bool SIMMqtt_Publish(const char* topic, const char* payload) {
    if (_currentState != SIM_MQTT_READY) {
        SYS_CONSOLE_PRINT("%s - Error: MQTT not ready for publish!\r\n", __TAG__);
        return false;
    }

    strncpy(_pubTopic, topic, sizeof (_pubTopic) - 1);
    strncpy(_pubPayload, payload, sizeof (_pubPayload) - 1);
    _pubLen = strlen(_pubPayload);

    _currentState = SIM_MQTT_PUB_CMD;
    _isWaitingResp = false;
    _isBuilded = false;
    _currentTxLen = 0;
    _attemptCount = 0;

    return true;
}