#include "sim/sim_general.h"
#include "sim/service/sim_gps.h"
#include "sim/core/sim_driver.h"
#include "sim/core/sim_basic.h"

static const char * __TAG__ = "SIMGPS";

static bool _respParser(int state, char* buffer, size_t maxLen);

static const SIM_CMD_SEQ _cmdTable[SIM_GPS_COUNT] = {
    [SIM_GPS_IDLE] =
    { NULL, NULL, NULL, NULL, 0, 0, NULL, SIM_GPS_IDLE, SIM_GPS_IDLE},

    [SIM_GPS_CHECK_ON] =
    { "AT+QGPS?\r\n", NULL, "OK", "ERROR", 1000, 3, _respParser, SIM_GPS_TURN_ON, SIM_GPS_ERROR},

    [SIM_GPS_TURN_ON] =
    { "AT+QGPS=1\r\n", NULL, "OK", "ERROR", 5000, 3, _respParser, SIM_GPS_READY, SIM_GPS_ERROR},

    [SIM_GPS_CHECK_SAT] =
    { "AT+QGPSGNMEA=\"GSV\"\r\n", NULL, "OK", "ERROR", 2000, 1, _respParser, SIM_GPS_READ_LOC, SIM_GPS_READ_LOC},

    [SIM_GPS_READ_LOC] =
    { "AT+QGPSLOC?\r\n", NULL, "OK", "ERROR", 2000, 1, _respParser, SIM_GPS_READY, SIM_GPS_READY},
    
    [SIM_GPS_READY] =
    { NULL, NULL, NULL, NULL, 0, 0, NULL, SIM_GPS_READY, SIM_GPS_READY},

    [SIM_GPS_ERROR] =
    { NULL, NULL, NULL, NULL, 0, 0, NULL, SIM_GPS_ERROR, SIM_GPS_ERROR}
};

static SIM_GPS_STATE _currentState = SIM_GPS_IDLE;
static SIM_GPS_INFO _gpsInfo = {0};

static bool _isWaitingResp = false;
static bool _isBuilded = false;
static int _currentTxLen = 0;
static uint8_t _attemptCount = 0;

static bool _respParser(int state, char* buffer, size_t maxLen) {
    switch (state) {
        case SIM_GPS_CHECK_ON:
            if (strstr(buffer, "+QGPS: 1") != NULL) {
                SYS_CONSOLE_PRINT("\r\nAnten ready\r\n");
                _currentState = SIM_GPS_READY; 
            } else {
                SYS_CONSOLE_PRINT("\r\nAnten off\r\n");
            }
            return true;

        case SIM_GPS_TURN_ON:
            SYS_CONSOLE_PRINT("\r\nGPS ON\r\n");
            return true;

        case SIM_GPS_CHECK_SAT:
            SYS_CONSOLE_PRINT("\r\n[GSV] %s", buffer);
            return true;

        case SIM_GPS_READ_LOC:
            SYS_CONSOLE_PRINT("\r\n[Location] %s", buffer);
            
            if (strstr(buffer, "+QGPSLOC:") != NULL) {
                _gpsInfo.hasFix = true;
            } else {
                _gpsInfo.hasFix = false;
            }
            return true;

        default:
            return true;
    }
}

static void _handleErrorOrTimeout(void) {
    const SIM_CMD_SEQ * cmdInfo = &_cmdTable[_currentState];

    if (_attemptCount < cmdInfo->attempts - 1) {
        _attemptCount++;
    } else {
        _currentState = cmdInfo->nextStateFail;
        _attemptCount = 0; 
    }
    _isWaitingResp = false;
    _isBuilded = false;
}

void SIMGps_Initialize(void) {
    _currentState = SIM_GPS_CHECK_ON;
    memset(&_gpsInfo, 0, sizeof(SIM_GPS_INFO));
    _isWaitingResp = false;
    _isBuilded = false;
    _currentTxLen = 0;
    _attemptCount = 0;
}

bool SIMGps_IsReady(void) {
    return (_currentState == SIM_GPS_READY);
}

bool SIMGps_HasError(void) {
    return (_currentState == SIM_GPS_ERROR);
}

bool SIMGps_UpdateLocation(void) {
    if (_currentState == SIM_GPS_READY) {
        SYS_CONSOLE_PRINT("\r\n--- UPDATE LOCATION ---\r\n");
        _currentState = SIM_GPS_CHECK_SAT; 
        _isWaitingResp = false;
        _isBuilded = false;
        return true;
    }
    return false;
}

SIM_GPS_INFO* SIMGps_GetInfo(void) {
    return &_gpsInfo;
}

void SIMGps_Process(void) {
    if (!SIMBasic_IsReady() || 
        _currentState == SIM_GPS_IDLE || 
        _currentState == SIM_GPS_READY || 
        _currentState == SIM_GPS_ERROR) {
        return;
    }

    if (!_isWaitingResp) {
        const SIM_CMD_SEQ * cmdInfo = &_cmdTable[_currentState];

        if (!_isBuilded) {
            uint8_t* tx_buf = SIMDriver_GetBuffer(SIM_DRV_TX_BUSY);
            if (tx_buf == NULL) return; 

            if (cmdInfo->builderFunc != NULL)
                _currentTxLen = cmdInfo->builderFunc((int)_currentState, (char*)tx_buf, SIM_TRANSFER_BUFF_SIZE, cmdInfo->cmd);
            else
                _currentTxLen = snprintf((char*)tx_buf, SIM_TRANSFER_BUFF_SIZE, "%s", cmdInfo->cmd);

            _isBuilded = true; 
        }

        if (_currentTxLen > 0) {
            if (SIMDriver_Execute((size_t)_currentTxLen, cmdInfo->timeoutMs)) {
                _isWaitingResp = true;
                _isBuilded = false; 
            }
        }
        return;
    } else { 
        SIM_DRV_STATUS status = SIMDriver_GetStatus();

        if (status == SIM_DRV_STATUS_RECV_RESP) {
            uint8_t* rx_buf = SIMDriver_GetBuffer(SIM_DRV_RX_BUSY);
            if (rx_buf != NULL) {
                _isWaitingResp = false;

                const char* expected_ok = _cmdTable[_currentState].respOk;
                const char* expected_fail = _cmdTable[_currentState].respFail;

                if (strstr((char*)rx_buf, expected_ok) != NULL) {
                    bool parsed = true;
                    if (_cmdTable[_currentState].parserFunc)
                        parsed = _cmdTable[_currentState].parserFunc(_currentState, (char*)rx_buf, SIM_TRANSFER_BUFF_SIZE);
                    
                    if (parsed) {
                        _attemptCount = 0;
                        if (_currentState != SIM_GPS_READY) {
                            _currentState = _cmdTable[_currentState].nextStateOk;
                        }
                    } else {
                        _handleErrorOrTimeout();
                    }
                } 
                else if (expected_fail != NULL && strstr((char*)rx_buf, expected_fail) != NULL) {
                    if (_cmdTable[_currentState].parserFunc) {
                        _cmdTable[_currentState].parserFunc(_currentState, (char*)rx_buf, SIM_TRANSFER_BUFF_SIZE);
                    }
                    _handleErrorOrTimeout();
                } 
                else {
                    _handleErrorOrTimeout();
                }
            }
        } else if (status == SIM_DRV_STATUS_TIMEOUT) {
            _handleErrorOrTimeout();
        }
    }
}

//    SIMBasic_Initialize(0); 
//    SIMGps_Initialize();   

//while (true) {
//        SYS_Tasks();
//        HMIDwin_Tasks();
//
//        SIMBasic_Process();
//        SIMGps_Process();
//
//        if (SIMBasic_IsReady()) {
//            uint32_t curTick = SYS_TMR_TickCountGet();
//            uint32_t tickPerSec = SYS_TMR_TickCounterFrequencyGet();
//
//            if (SIMGps_IsReady()) {
//                if (curTick - gpsTimer >= (tickPerSec * 10)) { 
//                    gpsTimer = curTick;
//                    SIMGps_UpdateLocation(); 
//                }
//            }
//        }
//    }