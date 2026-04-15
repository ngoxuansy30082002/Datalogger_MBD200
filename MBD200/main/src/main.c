#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include "definitions.h"


int main(void) {
    SYS_Initialize(NULL);
    SIM_SMS_Initialize();   // Khoi tao SMS
    static bool smsReadyPrinted = false;

    while (true) {
    SYS_Tasks();
    HMIDwin_Tasks();
    
//    test sms
    if (SIMBasic_IsReady()&& SIM_SMS_IsReady()&& !smsReadyPrinted) {
    SYS_CONSOLE_PRINT("\r\n SMS Init \r\n");
    //    SIM_SMS_Send("+84898171844", "AnhSondeptrai");
   
    smsReadyPrinted = true;
}
    }
    return (EXIT_FAILURE);

}