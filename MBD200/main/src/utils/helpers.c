#include "helpers.h"

uint8_t Helpers_Make8(uint16_t var, uint8_t offset) {
    uint8_t value = 0;
    value = (uint8_t) ((var >> (8 * offset))&0x00FF);
    return value;
}

uint16_t Helpers_Make16(uint8_t high, uint8_t low) {
    uint16_t value = 0;
    value = (((uint16_t) high << 8)&0xFF00)+(uint16_t) low;
    return value;
}

uint8_t Helpers_HexFromChars(char high, char low) {
    uint8_t hex_value = 0;

    if (isdigit(high)) {
        hex_value = (high - '0') << 4;
    } else if (high >= 'A' && high <= 'F') {
        hex_value = (high - 'A' + 10) << 4;
    } else if (high >= 'a' && high <= 'f') {
        hex_value = (high - 'a' + 10) << 4;
    } else {
        return 0;
    }

    if (isdigit(low)) {
        hex_value |= (low - '0');
    } else if (low >= 'A' && low <= 'F') {
        hex_value |= (low - 'A' + 10);
    } else if (low >= 'a' && low <= 'f') {
        hex_value |= (low - 'a' + 10);
    } else {
        return 0;
    }

    return hex_value;
}

uint32_t Helpers_CRC32Calculate(const uint8_t *data, size_t length) {
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xEDB88320UL;
            else
                crc >>= 1;
        }
    }
    return ~crc;
}