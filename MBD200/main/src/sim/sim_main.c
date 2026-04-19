#include "sim_main.h"
#include "core/sim_driver.h"
#include "core/sim_basic.h"
#include "core/sim_net.h"

#include "service/sim_mqtt.h"
#include "service/sim_ntp.h"
#include "service/sim_ftp.h"
#include "service/sim_sms.h"
#include "service/sim_gps.h"

void SIMMain_Initialize(void) {
    SIMDriver_Initialize();
    SIMBasic_Initialize(0);
    SIM_SMS_Initialize(); // SMS
}

void SIMMain_Task(void) {
    SIMDriver_Task();
    SIMBasic_Process();
    SIMNet_Process();
    SIMNtp_Process();
    SIMMqtt_Process();
    SIMFtp_Process();
    SIM_SMS_Process();


    /* Test */
    //    static uint32_t testTick = 0;
    //    static bool active = false;
    //    static bool first = true;
    //    static uint32_t timeout = 0;
    //
    //    if (SIMBasic_IsReady()) {
    //        timeout = (first) ? 10000 : 60000;
    //        if (TIME_IS_EXPIRED(testTick, timeout)) {
    //            first = false;
    //            testTick = TICK_NOW();
    //
    //            if (!active) {
    //                SYS_CONSOLE_PRINT("connect \r\n");
    //                SIMNet_Start(false);
    //                //                SIMMqtt_Start();
    //
    //                snprintf(gAppCfg.ftpServer[0].username, USERNAME_LEN, "%s", "bk_developer");
    //                snprintf(gAppCfg.ftpServer[0].password, PASSWORD_LEN, "%s", "!@bklogy12");
    //                snprintf(gAppCfg.ftpServer[0].hostname, URL_LEN, "%s", "103.139.155.86");
    //                gAppCfg.ftpServer[0].port = 21;
    //                SIMFtp_Start(0, "bk1/bk2/bk3/test.txt", 21, "hello world", 11);
    //            } else {
    //                SYS_CONSOLE_PRINT("disconnect \r\n");
    //                SIMNet_Stop();
    //                //                SIMMqtt_Abort();
    //                SIMFtp_Abort();
    //            }
    //            active = !active;
    //        }
    //    }

    /* End test */
}
