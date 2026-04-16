#include "sim/sim_general.h"
#include "sim_sms.h"
#include "sim/core/sim_driver.h"
#include "sim/core/sim_basic.h"

static const char * __TAG__ = "SIM_SMS";

typedef enum {
    SMS_STEP_CMGF = 0,
    SMS_STEP_CMGS_CMD,
    SMS_STEP_WAIT_PROMPT,
    SMS_STEP_TEXT_DATA,
    SMS_STEP_DONE
} SIM_SMS_SEND_STEP;

static SIM_SMS_STATE _smsState = SIM_SMS_IDLE;
static SIM_SMS_SEND_STEP _smsStep = SMS_STEP_CMGF;

static SIM_SMS_MSG _outMsg;
static bool _isWaitingResp = false;
static uint8_t _retryCount = 0;

void SIM_SMS_Initialize(void) {
    _smsState = SIM_SMS_IDLE;
    _isWaitingResp = false;
    memset(&_outMsg, 0, sizeof(SIM_SMS_MSG));
}

bool SIM_SMS_IsReady(void) {
    return (_smsState == SIM_SMS_IDLE);
}
    //    SIM_SMS_Send("+84898171844", "AnhSondeptrai");
bool SIM_SMS_Send(const char* phoneNumber, const char* message) {
    if (_smsState != SIM_SMS_IDLE || !SIMBasic_IsReady()) {
        return false;
    }

    snprintf(_outMsg.phoneNumber, sizeof(_outMsg.phoneNumber), "%s", phoneNumber);
    snprintf(_outMsg.message, sizeof(_outMsg.message), "%s", message);

    _smsState = SIM_SMS_SENDING;
    _smsStep = SMS_STEP_CMGF; 
    _isWaitingResp = false;
    _retryCount = 0;

    SYS_CONSOLE_PRINT("%s - %s:\t Request SEND to %s\r\n", __TAG__, __func__, _outMsg.phoneNumber);
    return true;
}

void SIM_SMS_Process(void) {
    if (!SIMBasic_IsReady() || _smsState == SIM_SMS_IDLE) {
        return; 
    }

    if (_smsState == SIM_SMS_SENDING) {
        if (!_isWaitingResp) {
            uint8_t* tx_buf = SIMDriver_GetBuffer(SIM_DRV_TX_BUSY);
            if (tx_buf == NULL) return;

            int len = 0;
            uint32_t timeoutMs = 2000;

            switch (_smsStep) {
                case SMS_STEP_CMGF:
                    len = snprintf((char*)tx_buf, SIM_TRANSFER_BUFF_SIZE, "AT+CMGF=1\r\n");
                    timeoutMs = 1000;
                    break;
                case SMS_STEP_CMGS_CMD:
                    len = snprintf((char*)tx_buf, SIM_TRANSFER_BUFF_SIZE, "AT+CMGS=\"%s\"\r\n", _outMsg.phoneNumber);
                    timeoutMs = 5000;
                    break;
                case SMS_STEP_TEXT_DATA:
                    len = snprintf((char*)tx_buf, SIM_TRANSFER_BUFF_SIZE, "%s\x1A", _outMsg.message);
                    timeoutMs = 15000;
                    break;
                default: break;
            }

            if (len > 0 && SIMDriver_Execute((size_t)len, timeoutMs)) {
                _isWaitingResp = true;
            }
        } 
        else {
            SIM_DRV_STATUS status = SIMDriver_GetStatus();

            if (status == SIM_DRV_STATUS_RECV_RESP) {
                uint8_t* rx_buf = SIMDriver_GetBuffer(SIM_DRV_RX_BUSY);
                if (rx_buf != NULL) {
                    _isWaitingResp = false;
                    
                    if (_smsStep == SMS_STEP_CMGF) {
                        if (strstr((char*)rx_buf, "OK") != NULL) {
                            _smsStep = SMS_STEP_CMGS_CMD;
                            _retryCount = 0;
                        } else goto SMS_FAIL_HANDLER;
                    } 
                    else if (_smsStep == SMS_STEP_CMGS_CMD) {
                        if (strstr((char*)rx_buf, ">") != NULL) {
                            _smsStep = SMS_STEP_TEXT_DATA;
                            _retryCount = 0;
                        } else goto SMS_FAIL_HANDLER;
                    } 
                    else if (_smsStep == SMS_STEP_TEXT_DATA) {
                        if (strstr((char*)rx_buf, "+CMGS:") != NULL || strstr((char*)rx_buf, "OK") != NULL) {
                            SYS_CONSOLE_PRINT("%s - %s:\t SMS Sent SUCCESS!\r\n", __TAG__, __func__);
                            _smsState = SIM_SMS_IDLE;
                        } else goto SMS_FAIL_HANDLER;
                    }
                }
            } 
            else if (status == SIM_DRV_STATUS_TIMEOUT) {
                goto SMS_FAIL_HANDLER;
            }
            return;

SMS_FAIL_HANDLER:
            _isWaitingResp = false;
            if (++_retryCount >= 3) {
                SYS_CONSOLE_PRINT("%s - %s:\t SMS SEND FAILED\r\n", __TAG__, __func__);
                _smsState = SIM_SMS_ERROR;
            }
        }
    } 
    else if (_smsState == SIM_SMS_ERROR) {
        _smsState = SIM_SMS_IDLE;
    }
}