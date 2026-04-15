#include "sim_main.h"
#include "core/sim_driver.h"
#include "core/sim_basic.h"

void SIMMain_Initialize(void) {
    SIMDriver_Initialize();
    SIMBasic_Initialize(0);
    SIM_SMS_Initialize();    // SMS
}

void SIMMain_Task(void) {
    SIMDriver_Task();
    SIMBasic_Process();
    SIM_SMS_Process();
}
