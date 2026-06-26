#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include "definitions.h"

bool mainForceReset = false;
static uint32_t resetTick = 0;
static bool resetFlag = false;

int main(void) {
    SYS_Initialize(NULL);

    while (true) {
        SYS_Tasks();

        if (mainForceReset) {
            mainForceReset = false;
            resetTick = TICK_NOW();
            resetFlag = true;
        }
        if (resetFlag && TIME_IS_EXPIRED(resetTick, 2000))
            SYS_RESET_SoftwareReset();

    }

    return ( EXIT_FAILURE);
}