#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include "definitions.h"
#include "app_mqtt_task.h"

int main(void) {
    SYS_Initialize(NULL);

    while (true) {
        SYS_Tasks();
    }

    return ( EXIT_FAILURE);
}