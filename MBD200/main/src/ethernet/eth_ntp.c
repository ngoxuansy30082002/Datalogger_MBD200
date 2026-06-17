#include "eth_ntp.h"

static ETH_NTP_STATE _states = ETH_NTP_IDLE;
static bool _isChangeSetting = false;
static TCPIP_SNTP_EVENT_TIME_DATA _ntpData;
static TCPIP_NET_HANDLE netH;
static TCPIP_SNTP_RESULT _ntpRes;
static uint32_t _lastSyncTick = 0;

static void _SNTPCallbackHandler(TCPIP_SNTP_EVENT evType, const void* param) {
    if (evType == TCPIP_SNTP_EVENT_TSTAMP_OK) {
        _ntpData = *(TCPIP_SNTP_EVENT_TIME_DATA *) param;
        _states = ETH_NTP_PARSE_TIME;
    }
}

void EthNtp_Initialize(void) {
    TCPIP_SNTP_HandlerRegister(_SNTPCallbackHandler);
}

void EthNtp_Task(void) {
    switch (_states) {
        case ETH_NTP_IDLE:
            if (!gAppCfg.time.syncNtpEnable || (gAppCfg.network.uplink != UPLINK_ALL && gAppCfg.network.uplink != UPLINK_ETH)) {
                if (TCPIP_SNTP_IsEnabled())
                    TCPIP_SNTP_Disable();

                return;
            }

            if (_isChangeSetting) {
                _isChangeSetting = false;
                _lastSyncTick = TICK_NOW();
                _states = ETH_NTP_INIT;
            } else if (gAppCfg.time.syncInterval > 0) {
                uint32_t intervalMs = gAppCfg.time.syncInterval * 1000;

                if (TIME_IS_EXPIRED(_lastSyncTick, intervalMs)) {
                    _lastSyncTick = TICK_NOW();
                    _states = ETH_NTP_INIT;
                }
            }
            break;

        case ETH_NTP_INIT:
            if (!TCPIP_SNTP_IsEnabled())
                TCPIP_SNTP_Enable();

            netH = TCPIP_STACK_IndexToNet(0);

            if (strlen(gAppCfg.time.ntpServerPrimary) > 0)
                _ntpRes = TCPIP_SNTP_ConnectionParamSet(netH, IP_ADDRESS_TYPE_IPV4, gAppCfg.time.ntpServerPrimary);
            else if (strlen(gAppCfg.time.ntpServerBackup) > 0)
                _ntpRes = TCPIP_SNTP_ConnectionParamSet(netH, IP_ADDRESS_TYPE_IPV4, gAppCfg.time.ntpServerBackup);

            _ntpRes = TCPIP_SNTP_ConnectionInitiate();
            _states = ETH_NTP_IDLE;
            break;

        case ETH_NTP_PARSE_TIME:
            Rtc_updateFromEthNtp((time_t) _ntpData.tUnixSeconds);
            _states = ETH_NTP_IDLE;
            break;
    }
}

void EthNtp_TriggerUpdate(void) {
    _isChangeSetting = true;
}