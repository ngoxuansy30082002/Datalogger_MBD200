#include "sim/sim_general.h"
#include "sim/core/sim_driver.h"
#include "sim/core/sim_basic.h"
#include "sim_ntp.h"

static char * _ntpServer[7] = {
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

static const SIM_CMD_SEQ _cmdTable[] = {
    /* { cmd, builderFunc, respOk, respFail, timeoutMs, attempts, parserFunc, nextStateOk, nextStateFail } */

    [SIM_NTP_IDLE] =
    { NULL, NULL, NULL, NULL, 0, 0, NULL, SIM_NTP_IDLE, SIM_NTP_IDLE},

    [SIM_NTP_SETUP_TIMEZONE] =
    { "AT+CCLK=\"04/01/01,00:00:02+00\"\r\n", NULL, "OK", "ERROR", 1000, 6, NULL, SIM_NTP_SYNC_TIME, SIM_NTP_ERROR},

    [SIM_NTP_SYNC_TIME] =
    {"AT+QNTP=%u,\"%s\",123\r\n", _cmdBuilder, "+00\"", "ERROR", 90000, 3, _respParser, SIM_NTP_READY, SIM_NTP_ERROR},

};

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
        case SIM_NTP_SYNC_TIME:
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