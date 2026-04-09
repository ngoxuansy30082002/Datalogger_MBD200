//#include "sim/sim_general.h"
//#include "sim/core/sim_driver.h"
//#include "sim/core/sim_basic.h"
//#include "sim_ntp.h"
//
//static int _cmdBuilder(int state, char* buffer, size_t maxLen, const char* format);
//static bool _respParser(int state, char* buffer, size_t maxLen);
//
//static const SIM_CMD_SEQ _cmdTable[] = {
//    /* { cmd, builderFunc, respOk, respFail, timeoutMs, attempts, parserFunc, nextStateOk, nextStateFail } */
//
//    [SIM_NTP_IDLE] =
//    { NULL, NULL, NULL, NULL, 0, 0, NULL, SIM_NTP_IDLE, SIM_NTP_IDLE},
//
//    [SIM_NTP_SYNC] =
//    { NULL, _cmdBuilder, "OK", "ERROR", 1000, 3, NULL, SIM_NTP_WAIT_URC, SIM_NTP_ERROR},
//
//    [SIM_NTP_WAIT_URC] =
//    { NULL, NULL, NULL, NULL, 120000, 1, _respParser, SIM_NTP_READY, SIM_NTP_ERROR},
//
//    [SIM_NTP_READY] =
//    { NULL, NULL, NULL, NULL, 0, 0, NULL, SIM_NTP_IDLE, SIM_NTP_IDLE},
//
//    [SIM_NTP_ERROR] =
//    { NULL, NULL, NULL, NULL, 0, 0, NULL, SIM_NTP_IDLE, SIM_NTP_IDLE}
//};
//
//static int _cmdBuilder(int state, char* buffer, size_t maxLen, const char* format) {
//    if (buffer == NULL || maxLen == 0) return 0;
//
//    switch (state) {
//        case SIM_NTP_SYNC:
//            // C?u trúc: AT+QNTP=<contextID>,"<server>",<port>
//            return snprintf(buffer, maxLen, "AT+QNTP=%u,\"%s\",123\r\n",
//                    SIM_CONTEXT_ID,
//                    glbAppCfg.GSM.ntpServer);
//        default:
//            if (format != NULL) return snprintf(buffer, maxLen, "%s", format);
//            return 0;
//    }
//}
//
//static bool _respParser(int state, char* buffer, size_t maxLen) {
//    if (buffer == NULL) return false;
//
//    switch (state) {
//        case SIM_NTP_WAIT_URC:
//            // Ki?m tra URC báo thành công: +QNTP: 0
//            if (strstr(buffer, "+QNTP: 0") != NULL) {
//                gsmNTP_Dt.flag.Flags.isSynced = 1;
//                // Có th? c?p nh?t th?i gian h? th?ng t?i ?ây n?u c?n
//                return true;
//            }
//            // N?u nh?n ???c +QNTP v?i mã l?i khác 0
//            if (strstr(buffer, "+QNTP:") != NULL) {
//                return false;
//            }
//            return false; // Ti?p t?c ??i n?u ch?a th?y URC
//
//        default:
//            return true;
//    }
//}