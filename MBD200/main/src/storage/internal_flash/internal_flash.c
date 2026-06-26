#include "internal_flash.h"
#include "dee_emulation.h"

bool InFlash_Initialize(void) {
    uint32_t res = DataEEInit();
    SYS_CONSOLE_PRINT("res: %d\r\n", res);

    return res;
}

bool InFlash_SaveDeviceInfo(uint8_t *devInfo, uint16_t size) {
    uint32_t res = DataEEWriteArray(devInfo, VA_OF_DEVICE_INFO, size);
    return res;
}

bool InFlash_LoadDeviceInfo(uint8_t *devInfo, uint16_t size) {
    uint32_t res = DataEEReadArray(devInfo, VA_OF_DEVICE_INFO, size);
    return res;
}
