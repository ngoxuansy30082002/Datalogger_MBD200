#include "sim_main.h"
#include "core/sim_driver.h"
#include "core/sim_basic.h"
#include "core/sim_net.h"

#include "service/sim_ftp.h"
#include "service/sim_mqtt.h"
#include "service/sim_ntp.h"
#include "service/sim_sms.h"
#include "service/sim_gps.h"

static const char * __TAG__ = "SIMMAIN";
static SIM_MAIN_NTP_CONTEXT _ntpCtx = {0};
static SIM_NET_OWNER _netOwner = NET_OWNER_NONE;
static SIM_MAIN_FTP_CONTEXT _ftpCtx = {0};
static SIM_FTP_RESULT _ftpResult = {0};

static bool _netAcquire(SIM_NET_OWNER who) {
    if (_netOwner == NET_OWNER_NONE) {
        _netOwner = who;
        return true;
    }
    return (_netOwner == who);
}

static void _netRelease(SIM_NET_OWNER who) {
    if (_netOwner == who)
        _netOwner = NET_OWNER_NONE;
}

/* 
 *
 *  NTP
 * 
 */
static void _ntpRetryOrFail(void) {
    SIMNtp_Abort();

    _ntpCtx.retryCount++;
    if (_ntpCtx.retryCount <= NTP_MAX_RETRY) {
        LOG_DEBUG("%s - %s\t Retry %d/%d", __TAG__, __func__, _ntpCtx.retryCount, NTP_MAX_RETRY);
        _ntpCtx.state = NTP_ST_NET_START;
    } else {
        LOG_DEBUG("%s - %s\t Failed after %d retries", __TAG__, __func__, NTP_MAX_RETRY);
        _netRelease(NET_OWNER_NTP);
        SIMNet_Stop();
        _ntpCtx.state = NTP_ST_ERROR;
    }
}

static void _ntpFsmProcess(void) {
    switch (_ntpCtx.state) {

        case NTP_ST_IDLE:
        case NTP_ST_DONE:
        case NTP_ST_ERROR:
            if (_ntpCtx.triggered) {
                if (!_netAcquire(NET_OWNER_NTP)) {
                    break;
                }
                _ntpCtx.timer = TICK_NOW();
                _ntpCtx.triggered = false;
                _ntpCtx.retryCount = 0;
                _ntpCtx.state = NTP_ST_NET_START;
                LOG_DEBUG("%s - %s\t Triggered", __TAG__, __func__);
            }
            break;

        case NTP_ST_NET_START:
            if (SIMNet_IsReady()) {
                _ntpCtx.state = NTP_ST_NTP_START;
            } else {
                if (!SIMNet_Start(false)) {
                    if (TIME_IS_EXPIRED(_ntpCtx.timer, NTP_NET_TIMEOUT_MS)) {
                        LOG_DEBUG("%s - %s\t Net timeout", __TAG__, __func__);
                        _ntpRetryOrFail();
                    }
                    break;
                } else {
                    _ntpCtx.timer = TICK_NOW();
                    _ntpCtx.state = NTP_ST_NET_WAIT;
                }
            }
            break;

        case NTP_ST_NET_WAIT:
            if (SIMNet_IsReady()) {
                LOG_DEBUG("%s - %s\t Net Ready", __TAG__, __func__);
                _ntpCtx.state = NTP_ST_NTP_START;
            } else if (SIMNet_HasError()) {
                LOG_DEBUG("%s - %s\t Net error", __TAG__, __func__);
                _ntpRetryOrFail();
            } else if (TIME_IS_EXPIRED(_ntpCtx.timer, NTP_NET_TIMEOUT_MS)) {
                LOG_DEBUG("%s - %s\t Net timeout", __TAG__, __func__);
                _ntpRetryOrFail();
            }
            break;

        case NTP_ST_NTP_START:
            if (!SIMNtp_Start()) {
                _ntpRetryOrFail();
            } else {
                LOG_DEBUG("%s - %s\t NTP start", __TAG__, __func__);
                _ntpCtx.timer = TICK_NOW();
                _ntpCtx.state = NTP_ST_NTP_WAIT;
            }
            break;

        case NTP_ST_NTP_WAIT:
            if (SIMNtp_IsReady()) {
                LOG_DEBUG("%s - %s\t Sync OK\n");
                _netRelease(NET_OWNER_NTP);
                SIMNtp_Abort();
                SIMNet_Stop();
                _ntpCtx.state = NTP_ST_DONE;
            } else if (SIMNtp_HasError()) {
                _ntpRetryOrFail();
            } else if (TIME_IS_EXPIRED(_ntpCtx.timer, NTP_NTP_TIMEOUT_MS)) {
                _ntpRetryOrFail();
            }
            break;
    }
}

/* 
 *
 *  FTP
 * 
 */
static void _ftpRetryOrFail(void) {
    SIMFtp_Abort();

    _ftpCtx.retryCount++;
    if (_ftpCtx.retryCount <= FTP_MAX_RETRY) {
        LOG_DEBUG("%s - %s\t Retry %d/%d", __TAG__, __func__, _ftpCtx.retryCount, FTP_MAX_RETRY);
        _ftpCtx.state = FTP_ST_NET_START;
    } else {
        LOG_DEBUG("%s - %s\t Failed after %d retries", __TAG__, __func__, FTP_MAX_RETRY);
        _netRelease(NET_OWNER_FTP);
        SIMNet_Stop();
        _ftpCtx.state = FTP_ST_ERROR;
    }
}

static void _ftpFsmProcess(void) {
    switch (_ftpCtx.state) {

        case FTP_ST_IDLE:
        case FTP_ST_DONE:
        case FTP_ST_ERROR:
            if (_ftpCtx.triggered) {
                if (!_netAcquire(NET_OWNER_FTP)) {
                    break;
                }
                _ftpCtx.timer = TICK_NOW();
                _ftpCtx.triggered = false;
                _ftpCtx.retryCount = 0;
                _ftpCtx.state = FTP_ST_NET_START;
                LOG_DEBUG("%s - %s\t Triggered (ftp1=%d, ftp2=%d)",
                        __TAG__, __func__,
                        _ftpCtx.useFtp1, _ftpCtx.useFtp2);
            }
            break;

        case FTP_ST_NET_START:
            if (SIMNet_IsReady()) {
                _ftpCtx.state = FTP_ST_FTP_START;
            } else {
                if (!SIMNet_Start(false)) {
                    if (TIME_IS_EXPIRED(_ftpCtx.timer, FTP_NET_TIMEOUT_MS)) {
                        _ftpRetryOrFail();
                    }
                    break;
                } else {
                    _ftpCtx.timer = TICK_NOW();
                    _ftpCtx.state = FTP_ST_NET_WAIT;
                }
            }
            break;

        case FTP_ST_NET_WAIT:
            if (SIMNet_IsReady()) {
                _ftpCtx.state = FTP_ST_FTP_START;
            } else if (SIMNet_HasError()) {
                _ftpRetryOrFail();
            } else if (TIME_IS_EXPIRED(_ftpCtx.timer, FTP_NET_TIMEOUT_MS)) {
                _ftpRetryOrFail();
            }
            break;

        case FTP_ST_FTP_START:
            if (!SIMFtp_Start(_ftpCtx.useFtp1, _ftpCtx.useFtp2)) {
                _ftpRetryOrFail();
            } else {
                _ftpCtx.timer = TICK_NOW();
                _ftpCtx.state = FTP_ST_FTP_WAIT;
            }
            break;

        case FTP_ST_FTP_WAIT:
        {
            _ftpResult = SIMFtp_GetStatus();
            if (!_ftpResult.isUploading) {
                bool allOk = true;
                if (_ftpCtx.useFtp1 && _ftpResult.server1 != SIM_FTP_SERVER_SUCCESS) allOk = false;
                if (_ftpCtx.useFtp2 && _ftpResult.server2 != SIM_FTP_SERVER_SUCCESS) allOk = false;

                if (allOk) {
                    LOG_DEBUG("%s - %s\t Upload OK", __TAG__, __func__);
                    _netRelease(NET_OWNER_FTP);
                    SIMFtp_Abort();
                    SIMNet_Stop();
                    _ftpCtx.state = FTP_ST_DONE;
                } else {
                    LOG_DEBUG("%s - %s\t Upload fail (s1=%d, s2=%d)",
                            __TAG__, __func__,
                            _ftpResult.server1, _ftpResult.server2);
                    _ftpRetryOrFail();
                }
            } else if (TIME_IS_EXPIRED(_ftpCtx.timer, FTP_UPLOAD_TIMEOUT_MS)) {
                _ftpRetryOrFail();
            }
            break;
        }

    }
}

void SIMMain_Initialize(void) {
    SIMDriver_Initialize();
    SIMBasic_Initialize(0);
    SIM_SMS_Initialize();
}

void SIMMain_Task(void) {
    /* ?? Core ?? */
    SIMDriver_Task();
    SIMBasic_Process();
    SIMNet_Process();

    /* ?? Services ?? */
    SIMNtp_Process();
    SIMMqtt_Process();
    SIMFtp_Process();
    SIM_SMS_Process();

    /* ?? Orchestrator FSMs ?? */
    _ntpFsmProcess();
    _ftpFsmProcess();
}

/* NTP Public API */
void SIMMain_NTPTrigger(void) {
    if (_ntpCtx.state == NTP_ST_IDLE ||
            _ntpCtx.state == NTP_ST_DONE ||
            _ntpCtx.state == NTP_ST_ERROR) {
        _ntpCtx.triggered = true;
    }
}

bool SIMMain_NTPIsBusy(void) {
    return (_ntpCtx.state != NTP_ST_IDLE &&
            _ntpCtx.state != NTP_ST_DONE &&
            _ntpCtx.state != NTP_ST_ERROR);
}

bool SIMMain_NTPIsSuccess(void) {
    return (_ntpCtx.state == NTP_ST_DONE);
}

bool SIMMain_NTPHasError(void) {
    return (_ntpCtx.state == NTP_ST_ERROR);
}

/* FTP Public API */
void SIMMain_FTPTrigger(bool ftp1, bool ftp2) {
    if (_ftpCtx.state == FTP_ST_IDLE ||
            _ftpCtx.state == FTP_ST_DONE ||
            _ftpCtx.state == FTP_ST_ERROR) {
        _ftpCtx.useFtp1 = ftp1;
        _ftpCtx.useFtp2 = ftp2;
        _ftpCtx.triggered = true;
    }
}

bool SIMMain_FTPIsBusy(void) {
    return (_ftpCtx.state != FTP_ST_IDLE &&
            _ftpCtx.state != FTP_ST_DONE &&
            _ftpCtx.state != FTP_ST_ERROR);
}

bool SIMMain_FTPIsSuccess(void) {
    return (_ftpCtx.state == FTP_ST_DONE);
}

bool SIMMain_FTPHasError(void) {
    return (_ftpCtx.state == FTP_ST_ERROR);
}

bool SIMMain_FTPGetResult(bool * ftp1Success, bool * ftp2Success) {
    (*ftp1Success) = (_ftpResult.server1 == SIM_FTP_SERVER_SUCCESS);
    (*ftp2Success) = (_ftpResult.server2 == SIM_FTP_SERVER_SUCCESS);
    return (_ftpCtx.state == FTP_ST_DONE);
}
