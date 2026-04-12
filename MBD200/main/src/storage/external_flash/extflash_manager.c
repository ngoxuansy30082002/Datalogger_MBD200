#include "extflash_manager.h"

static const char * __TAG__ = "EXTFL";

static EXTFL_WRITE_STATE _writeState = EXTFL_WRITE_IDLE;
static EXTFL_READ_STATE _readState = EXTFL_READ_IDLE;

static iqueue_t _writeQueue;
static iqueue_t _readQueue;
static uint8_t _writeQueueStorage[EXTFL_QUEUE_SIZE * sizeof (EXTFL_QUEUE_ENTRY)];
static uint8_t _readQueueStorage[EXTFL_QUEUE_SIZE * sizeof (EXTFL_QUEUE_ENTRY)];

static DRV_HANDLE _driverHandle;
static bool _openStatus = 0;
static uint32_t _currentAddrFlash = 0;
static CACHE_ALIGN uint8_t _transferBuffer[EXTFL_PAGE_SIZE * 8];


static const EXTFL_PARTITION _partition[EXTFL_NUM_PARTITION] = {
    {.type = EXTFL_DATA_APP_CFG, .startAddress = 0x000000},
    {.type = EXTFL_DATA_SENSOR_CFG, .startAddress = 0x001000},
    {.type = EXTFL_DATA_ANALOG_CFG, .startAddress = 0x002000},
    {.type = EXTFL_DATA_MBRTU_CFG, .startAddress = 0x003000},
    {.type = EXTFL_DATA_INCAPTURE_CFG, .startAddress = 0x004000},
};

static uint32_t _maxPowerOfTwo(uint32_t num) {
    uint32_t ret_val = 0;
    uint32_t number = 0;

    for (number = num; number >= 1U; number--) {
        // If number is a power of 2
        if ((number & (number - 1U)) == 0U) {
            ret_val = number;
            break;
        }
    }
    return ret_val;
}

static bool _eraseSectorFlash(uint32_t address) {
    return DRV_SST26_SectorErase(_driverHandle, address);
}

static bool _transferIsBusy() {
    return (DRV_SST26_TransferStatusGet(_driverHandle) != DRV_SST26_TRANSFER_COMPLETED);
}

static bool _readFlash(uint32_t address, uint32_t len) {
    return DRV_SST26_Read(_driverHandle, _transferBuffer, len, address);
}

static uint16_t _calculateDataSize(EXTFL_DATA_TYPE dataType) {
    switch (dataType) {
        case EXTFL_DATA_APP_CFG: return sizeof (APP_PACKED);
        case EXTFL_DATA_SENSOR_CFG: return sizeof (SENSOR_PACKED);
        case EXTFL_DATA_ANALOG_CFG: return sizeof (ANALOG_PACKED);
        case EXTFL_DATA_MBRTU_CFG: return sizeof (MODBUSRTU_TAG_PACKED);
        case EXTFL_DATA_INCAPTURE_CFG: return sizeof (INPUT_CAPTURE_PACKED);
        default: return 0;
    }
}

static bool _writeStreamByteToFlash(uint8_t *pData, uint32_t wLen, bool *finish) {

    enum {
        WRITE_STATE_IDLE = 0,
        WRITE_STATE_FIRST_PART,
        WRITE_STATE_FIRST_PART_WAIT,
        WRITE_STATE_BLOCK,
        WRITE_STATE_BLOCK_WAIT,
        WRITE_STATE_LAST_PART,
        WRITE_STATE_LAST_PART_WAIT,
    };

    static int writeStreamState = WRITE_STATE_IDLE;
    static uint16_t remainingLen = 0;
    static uint8_t *pWriteData = NULL;
    static uint16_t numByteFirst = 0;
    static uint16_t numBlocks = 0;
    static uint32_t chunkSize = 0;
    static bool syncLock = false;
    bool rtn = true;

    if (wLen == 0u || !pData) return false;

    if (wLen > 0 && !syncLock) {
        pWriteData = pData;
        remainingLen = wLen;

        numByteFirst = EXTFL_PAGE_SIZE - (_currentAddrFlash % EXTFL_PAGE_SIZE);
        if (numByteFirst > remainingLen) numByteFirst = remainingLen;
        remainingLen -= numByteFirst;

        numBlocks = remainingLen / EXTFL_PAGE_SIZE;

        writeStreamState = (numByteFirst > 0) ? WRITE_STATE_FIRST_PART :
                (numBlocks > 0) ? WRITE_STATE_BLOCK :
                WRITE_STATE_LAST_PART;

        syncLock = true;
    }

    switch (writeStreamState) {
        case WRITE_STATE_IDLE:
            syncLock = false;
            *finish = true;
            return rtn;
            break;

        case WRITE_STATE_FIRST_PART:
            chunkSize = _maxPowerOfTwo(numByteFirst);
            rtn = DRV_SST26_ByteWrite(_driverHandle, pWriteData,
                    (uint32_t) _currentAddrFlash, chunkSize);
            writeStreamState = WRITE_STATE_FIRST_PART_WAIT;
            break;

        case WRITE_STATE_FIRST_PART_WAIT:
            if (_transferIsBusy()) break;

            _currentAddrFlash += chunkSize;
            pWriteData += chunkSize;
            numByteFirst -= chunkSize;
            if (numByteFirst > 0) {
                writeStreamState = WRITE_STATE_FIRST_PART;
            } else if (numBlocks > 0) {
                writeStreamState = WRITE_STATE_BLOCK;
            } else if (remainingLen > 0) {
                writeStreamState = WRITE_STATE_LAST_PART;
            } else {
                writeStreamState = WRITE_STATE_IDLE;
            }
            break;

        case WRITE_STATE_BLOCK:
            rtn = DRV_SST26_PageWrite(_driverHandle, pWriteData,
                    (uint32_t) _currentAddrFlash);
            writeStreamState = WRITE_STATE_BLOCK_WAIT;
            break;

        case WRITE_STATE_BLOCK_WAIT:
            if (_transferIsBusy()) break;

            _currentAddrFlash += EXTFL_PAGE_SIZE;
            pWriteData += EXTFL_PAGE_SIZE;
            remainingLen -= EXTFL_PAGE_SIZE;
            numBlocks--;
            if (numBlocks > 0) {
                writeStreamState = WRITE_STATE_BLOCK;
            } else if (remainingLen > 0) {
                writeStreamState = WRITE_STATE_LAST_PART;
            } else {
                writeStreamState = WRITE_STATE_IDLE;
            }
            break;

        case WRITE_STATE_LAST_PART:
            chunkSize = _maxPowerOfTwo(remainingLen);
            rtn = DRV_SST26_ByteWrite(_driverHandle, pWriteData,
                    (uint32_t) _currentAddrFlash, chunkSize);
            writeStreamState = WRITE_STATE_LAST_PART_WAIT;
            break;

        case WRITE_STATE_LAST_PART_WAIT:
            if (_transferIsBusy()) break;

            _currentAddrFlash += chunkSize;
            pWriteData += chunkSize;
            remainingLen -= chunkSize;
            if (remainingLen > 0) {
                writeStreamState = WRITE_STATE_LAST_PART;
            } else {
                writeStreamState = WRITE_STATE_IDLE;
            }
            break;
    }

    *finish = false;
    return rtn;
}

static bool _unpackBufferLoaded(void * buffer, uint16_t bufferSize, EXTFL_DATA_TYPE dataType) {
    switch (dataType) {
        case EXTFL_DATA_APP_CFG:
        {
            if (bufferSize < sizeof (APP_PACKED))
                return false;

            APP_PACKED *packed = (APP_PACKED*) buffer;
            uint8_t type = (uint8_t) EXTFL_DATA_APP_CFG;
            uint16_t length = sizeof (APP_PACKED) - sizeof (EXTFL_METADATA);
            uint32_t crc = Helpers_CRC32Calculate((uint8_t *) & packed->network, length);

            if (packed->metadata.magic != EXTFL_MAGIC || type != packed->metadata.type ||
                    length != packed->metadata.length || crc != packed->metadata.crc)
                return false;

            memcpy(&gAppCfg.network.ipAddr, &packed->network.ipAddr, sizeof (IP_ADDR));
            memcpy(&gAppCfg.network.ipMask, &packed->network.ipMask, sizeof (IP_ADDR));
            memcpy(&gAppCfg.network.gateway, &packed->network.gateway, sizeof (IP_ADDR));
            memcpy(&gAppCfg.network.primaryDNS, &packed->network.primaryDNS, sizeof (IP_ADDR));
            memcpy(&gAppCfg.network.secondDNS, &packed->network.secondDNS, sizeof (IP_ADDR));
            memcpy(gAppCfg.network.netBIOSName, packed->network.netBIOSName, BIOS_NAME_LEN);
            gAppCfg.network.isDHCPEn = packed->network.isDHCPEn;
            memcpy(gAppCfg.network.deviceUsername, packed->network.deviceUsername, USERNAME_LEN);
            memcpy(gAppCfg.network.devicePassword, packed->network.devicePassword, PASSWORD_LEN);

            for (int i = 0; i < MAX_FTP_SERVER; i++) {
                memcpy(gAppCfg.ftpServer[i].username, packed->ftpServer[i].username, USERNAME_LEN);
                memcpy(gAppCfg.ftpServer[i].password, packed->ftpServer[i].password, PASSWORD_LEN);
                memcpy(gAppCfg.ftpServer[i].dirPath, packed->ftpServer[i].dirPath, DIR_PATH_LEN);
                memcpy(gAppCfg.ftpServer[i].hostname, packed->ftpServer[i].hostname, URL_LEN);
                gAppCfg.ftpServer[i].port = packed->ftpServer[i].port;
                memcpy(gAppCfg.ftpServer[i].namePrefix, packed->ftpServer[i].namePrefix, FILE_NAME_PREFIX_LEN);
                gAppCfg.ftpServer[i].makeFolder = packed->ftpServer[i].makeFolder;
                gAppCfg.ftpServer[i].enable = packed->ftpServer[i].enable;
            }

            gAppCfg.logFile.uplink = packed->logFile.uplink;
            gAppCfg.logFile.formatFile = packed->logFile.formatFile;
            gAppCfg.logFile.typefile = packed->logFile.typefile;
            gAppCfg.logFile.timeMode = packed->logFile.timeMode;
            gAppCfg.logFile.sendInterval = packed->logFile.sendInterval;

            gAppCfg.modbusRtu.timeout = packed->modbusRtu.timeout;
            gAppCfg.modbusRtu.retries = packed->modbusRtu.retries;
            gAppCfg.modbusRtu.baudRate = packed->modbusRtu.baudRate;
            gAppCfg.modbusRtu.pollInterval = packed->modbusRtu.pollInterval;
            gAppCfg.modbusRtu.parity = packed->modbusRtu.parity;
            gAppCfg.modbusRtu.stopbits = packed->modbusRtu.stopbits;
            gAppCfg.modbusRtu.latency = packed->modbusRtu.latency;

            memcpy(gAppCfg.gsm.usernameAPN, packed->gsm.usernameAPN, USERNAME_LEN);
            memcpy(gAppCfg.gsm.passwordAPN, packed->gsm.passwordAPN, PASSWORD_LEN);
            memcpy(gAppCfg.gsm.APN, packed->gsm.APN, APN_LEN);

            gAppCfg.time.indexNTP = packed->time.indexNTP;
            gAppCfg.time.timeAuto = packed->time.timeAuto;
            gAppCfg.time.yearNumber = packed->time.yearNumber;
            gAppCfg.time.timeZone = packed->time.timeZone;

            for (int i = 0; i < MAX_DIGITAL_OUTPUT; i++) {
                memcpy(gAppCfg.io.out[i].describe, packed->io.out[i].describe, SENSOR_NAME_LEN);
                gAppCfg.io.out[i].time = packed->io.out[i].time;
                gAppCfg.io.out[i].type = packed->io.out[i].type;
            }

            gAppCfg.sdCard.retentionMonths = packed->sdCard.retentionMonths;
            gAppCfg.sdCard.lastMonth = packed->sdCard.lastMonth;

            memcpy(gAppCfg.hmi, packed->hmi, MAX_HMI_PARA);
            memcpy(gAppCfg.position, packed->position, MAX_POSITION_SIZE * sizeof (uint16_t));
            break;
        }

        case EXTFL_DATA_SENSOR_CFG:
        {
            if (bufferSize < sizeof (SENSOR_PACKED))
                return false;

            SENSOR_PACKED *packed = (SENSOR_PACKED *) buffer;
            uint8_t type = (uint8_t) EXTFL_DATA_SENSOR_CFG;
            uint16_t length = sizeof (SENSOR_PACKED) - sizeof (EXTFL_METADATA);
            uint32_t crc = Helpers_CRC32Calculate((uint8_t *) & packed->entry, length);

            if (packed->metadata.magic != EXTFL_MAGIC || type != packed->metadata.type ||
                    length != packed->metadata.length || crc != packed->metadata.crc)
                return false;

            for (int i = 0; i < MAX_SENSOR; i++) {
                gSensorCfg.entry[i].enable = packed->entry[i].enable;
                gSensorCfg.entry[i].type = packed->entry[i].type;
                gSensorCfg.entry[i].indexOfType = packed->entry[i].indexOfType;

                gSensorCfg.entry[i].calibrated = packed->entry[i].calibrated;
                gSensorCfg.entry[i].typeStatus = packed->entry[i].typeStatus;

                gSensorCfg.entry[i].typeGood = packed->entry[i].typeGood;
                gSensorCfg.entry[i].indexOfTypeGood = packed->entry[i].indexOfTypeGood;
                gSensorCfg.entry[i].typeCalib = packed->entry[i].typeCalib;
                gSensorCfg.entry[i].indexOfTypeCalib = packed->entry[i].indexOfTypeCalib;
                gSensorCfg.entry[i].typeErr = packed->entry[i].typeErr;
                gSensorCfg.entry[i].indexOfTypeErr = packed->entry[i].indexOfTypeErr;

                gSensorCfg.entry[i].goodValueAND = packed->entry[i].goodValueAND;
                gSensorCfg.entry[i].goodValueCompare = packed->entry[i].goodValueCompare;
                gSensorCfg.entry[i].calibValueAND = packed->entry[i].calibValueAND;
                gSensorCfg.entry[i].calibValueCompare = packed->entry[i].calibValueCompare;
                gSensorCfg.entry[i].errorValueAND = packed->entry[i].errorValueAND;
                gSensorCfg.entry[i].errorValueCompare = packed->entry[i].errorValueCompare;
            }

            gSensorCfg.numSensor = packed->numSensor;
            break;
        }

        case EXTFL_DATA_ANALOG_CFG:
        {
            if (bufferSize < sizeof (ANALOG_PACKED))
                return false;

            ANALOG_PACKED *packed = (ANALOG_PACKED *) buffer;
            uint8_t type = (uint8_t) EXTFL_DATA_ANALOG_CFG;
            uint16_t length = sizeof (ANALOG_PACKED) - sizeof (EXTFL_METADATA);
            uint32_t crc = Helpers_CRC32Calculate((uint8_t *) & packed->entry, length);

            if (packed->metadata.magic != EXTFL_MAGIC || type != packed->metadata.type ||
                    length != packed->metadata.length || crc != packed->metadata.crc)
                return false;

            for (int i = 0; i < MAX_ANALOG_CHANNEL; i++) {
                gAnalogCfg.entry[i].enable = packed->entry[i].enable;
                memcpy(gAnalogCfg.entry[i].name, packed->entry[i].name, SENSOR_NAME_LEN);
                memcpy(gAnalogCfg.entry[i].unit, packed->entry[i].unit, SENSOR_UNIT_LEN);

                gAnalogCfg.entry[i].scaleType = packed->entry[i].scaleType;
                gAnalogCfg.entry[i].scaleDataType = packed->entry[i].scaleDataType;
                gAnalogCfg.entry[i].scaleValue = packed->entry[i].scaleValue;

                gAnalogCfg.entry[i].adcType = packed->entry[i].adcType;
                gAnalogCfg.entry[i].adcLow = packed->entry[i].adcLow;
                gAnalogCfg.entry[i].adcHigh = packed->entry[i].adcHigh;
                gAnalogCfg.entry[i].offsetPreVal = packed->entry[i].offsetPreVal;
                gAnalogCfg.entry[i].offsetSubVal = packed->entry[i].offsetSubVal;
                gAnalogCfg.entry[i].offSetPreOperator = packed->entry[i].offSetPreOperator;
                gAnalogCfg.entry[i].offsetSubOperator = packed->entry[i].offsetSubOperator;
            }
            break;
        }

        case EXTFL_DATA_MBRTU_CFG:
        {
            if (bufferSize < sizeof (MODBUSRTU_TAG_PACKED))
                return false;

            MODBUSRTU_TAG_PACKED *packed = (MODBUSRTU_TAG_PACKED *) buffer;
            uint8_t type = (uint8_t) EXTFL_DATA_MBRTU_CFG;
            uint16_t length = sizeof (MODBUSRTU_TAG_PACKED) - sizeof (EXTFL_METADATA);
            uint32_t crc = Helpers_CRC32Calculate((uint8_t *) & packed->entry, length);

            if (packed->metadata.magic != EXTFL_MAGIC || type != packed->metadata.type ||
                    length != packed->metadata.length || crc != packed->metadata.crc)
                return false;

            for (int i = 0; i < MAX_MODBUS_TAG; i++) {
                gMbrtuCfg.entry[i].enable = packed->entry[i].enable;
                memcpy(gMbrtuCfg.entry[i].name, packed->entry[i].name, SENSOR_NAME_LEN);
                memcpy(gMbrtuCfg.entry[i].unit, packed->entry[i].unit, SENSOR_UNIT_LEN);

                gMbrtuCfg.entry[i].slaveAddress = packed->entry[i].slaveAddress;
                gMbrtuCfg.entry[i].function = packed->entry[i].function;
                gMbrtuCfg.entry[i].regAddress = packed->entry[i].regAddress;
                gMbrtuCfg.entry[i].quantity = packed->entry[i].quantity;
                gMbrtuCfg.entry[i].rawDataType = packed->entry[i].rawDataType;
                gMbrtuCfg.entry[i].bigEndian = packed->entry[i].bigEndian;

                gMbrtuCfg.entry[i].scaleType = packed->entry[i].scaleType;
                gMbrtuCfg.entry[i].scaleDataType = packed->entry[i].scaleDataType;
                gMbrtuCfg.entry[i].scaleValue = packed->entry[i].scaleValue;

                gMbrtuCfg.entry[i].adcType = packed->entry[i].adcType;
                gMbrtuCfg.entry[i].adcLow = packed->entry[i].adcLow;
                gMbrtuCfg.entry[i].adcHigh = packed->entry[i].adcHigh;
                gMbrtuCfg.entry[i].offsetPreVal = packed->entry[i].offsetPreVal;
                gMbrtuCfg.entry[i].offsetSubVal = packed->entry[i].offsetSubVal;
                gMbrtuCfg.entry[i].offSetPreOperator = packed->entry[i].offSetPreOperator;
                gMbrtuCfg.entry[i].offsetSubOperator = packed->entry[i].offsetSubOperator;
            }

            gMbrtuCfg.numTag = packed->numTag;
            break;
        }

        case EXTFL_DATA_INCAPTURE_CFG:
        {
            if (bufferSize < sizeof (INPUT_CAPTURE_PACKED))
                return false;

            INPUT_CAPTURE_PACKED *packed = (INPUT_CAPTURE_PACKED *) buffer;
            uint8_t type = (uint8_t) EXTFL_DATA_INCAPTURE_CFG;
            uint16_t length = sizeof (INPUT_CAPTURE_PACKED) - sizeof (EXTFL_METADATA);
            uint32_t crc = Helpers_CRC32Calculate((uint8_t *) & packed->entry, length);

            if (packed->metadata.magic != EXTFL_MAGIC || type != packed->metadata.type ||
                    length != packed->metadata.length || crc != packed->metadata.crc)
                return false;

            for (int i = 0; i < MAX_INPUT_CAPTURE; i++) {
                gInCaptureCfg.entry[i].enable = packed->entry[i].enable;
                memcpy(gInCaptureCfg.entry[i].name, packed->entry[i].name, SENSOR_NAME_LEN);
                memcpy(gInCaptureCfg.entry[i].unit, packed->entry[i].unit, SENSOR_UNIT_LEN);

                gInCaptureCfg.entry[i].valPerPulse = packed->entry[i].valPerPulse;
                gInCaptureCfg.entry[i].minFreq = packed->entry[i].minFreq;
                gInCaptureCfg.entry[i].scale = packed->entry[i].scale;
            }
            break;
        }

        default: false;
    }

    return true;
}

static bool _packBufferSave(void * buffer, uint16_t maxBufferSize, EXTFL_DATA_TYPE dataType) {
    switch (dataType) {
        case EXTFL_DATA_APP_CFG:
        {
            if (maxBufferSize < sizeof (APP_PACKED))
                return false;

            APP_PACKED * dst = (APP_PACKED *) buffer;

            memcpy(&dst->network.ipAddr, &gAppCfg.network.ipAddr, sizeof (IP_ADDR));
            memcpy(&dst->network.ipMask, &gAppCfg.network.ipMask, sizeof (IP_ADDR));
            memcpy(&dst->network.gateway, &gAppCfg.network.gateway, sizeof (IP_ADDR));
            memcpy(&dst->network.primaryDNS, &gAppCfg.network.primaryDNS, sizeof (IP_ADDR));
            memcpy(&dst->network.secondDNS, &gAppCfg.network.secondDNS, sizeof (IP_ADDR));
            memcpy(dst->network.netBIOSName, &gAppCfg.network.netBIOSName, BIOS_NAME_LEN);
            dst->network.isDHCPEn = gAppCfg.network.isDHCPEn;
            memcpy(dst->network.deviceUsername, gAppCfg.network.deviceUsername, USERNAME_LEN);
            memcpy(dst->network.devicePassword, gAppCfg.network.devicePassword, PASSWORD_LEN);

            for (int i = 0; i < MAX_FTP_SERVER; i++) {
                memcpy(dst->ftpServer[i].username, gAppCfg.ftpServer[i].username, USERNAME_LEN);
                memcpy(dst->ftpServer[i].password, gAppCfg.ftpServer[i].password, PASSWORD_LEN);
                memcpy(dst->ftpServer[i].dirPath, gAppCfg.ftpServer[i].dirPath, DIR_PATH_LEN);
                memcpy(dst->ftpServer[i].hostname, gAppCfg.ftpServer[i].hostname, URL_LEN);
                dst->ftpServer[i].port = gAppCfg.ftpServer[i].port;
                memcpy(dst->ftpServer[i].namePrefix, gAppCfg.ftpServer[i].namePrefix, FILE_NAME_PREFIX_LEN);
                dst->ftpServer[i].makeFolder = gAppCfg.ftpServer[i].makeFolder;
                dst->ftpServer[i].enable = gAppCfg.ftpServer[i].enable;
            }

            dst->logFile.uplink = gAppCfg.logFile.uplink;
            dst->logFile.formatFile = gAppCfg.logFile.formatFile;
            dst->logFile.typefile = gAppCfg.logFile.typefile;
            dst->logFile.timeMode = gAppCfg.logFile.timeMode;
            dst->logFile.sendInterval = gAppCfg.logFile.sendInterval;

            dst->modbusRtu.timeout = gAppCfg.modbusRtu.timeout;
            dst->modbusRtu.retries = gAppCfg.modbusRtu.retries;
            dst->modbusRtu.baudRate = gAppCfg.modbusRtu.baudRate;
            dst->modbusRtu.pollInterval = gAppCfg.modbusRtu.pollInterval;
            dst->modbusRtu.parity = gAppCfg.modbusRtu.parity;
            dst->modbusRtu.stopbits = gAppCfg.modbusRtu.stopbits;
            dst->modbusRtu.latency = gAppCfg.modbusRtu.latency;

            memcpy(dst->gsm.usernameAPN, gAppCfg.gsm.usernameAPN, USERNAME_LEN);
            memcpy(dst->gsm.passwordAPN, gAppCfg.gsm.passwordAPN, PASSWORD_LEN);
            memcpy(dst->gsm.APN, gAppCfg.gsm.APN, APN_LEN);

            dst->time.indexNTP = gAppCfg.time.indexNTP;
            dst->time.timeAuto = gAppCfg.time.timeAuto;
            dst->time.yearNumber = gAppCfg.time.yearNumber;
            dst->time.timeZone = gAppCfg.time.timeZone;

            for (int i = 0; i < MAX_DIGITAL_OUTPUT; i++) {
                memcpy(dst->io.out[i].describe, gAppCfg.io.out[i].describe, SENSOR_NAME_LEN);
                dst->io.out[i].time = gAppCfg.io.out[i].time;
                dst->io.out[i].type = gAppCfg.io.out[i].type;
            }

            dst->sdCard.retentionMonths = gAppCfg.sdCard.retentionMonths;
            dst->sdCard.lastMonth = gAppCfg.sdCard.lastMonth;

            memcpy(dst->hmi, gAppCfg.hmi, MAX_HMI_PARA);
            memcpy(dst->position, gAppCfg.position, MAX_POSITION_SIZE * sizeof (uint16_t));

            uint16_t datalen = sizeof (APP_PACKED) - sizeof (EXTFL_METADATA);
            dst->metadata.magic = EXTFL_MAGIC;
            dst->metadata.length = datalen;
            dst->metadata.type = (uint8_t) EXTFL_DATA_APP_CFG;
            dst->metadata.crc = Helpers_CRC32Calculate((uint8_t *) & dst->network, datalen);
            break;
        }

        case EXTFL_DATA_SENSOR_CFG:
        {
            if (maxBufferSize < sizeof (SENSOR_PACKED))
                return false;

            SENSOR_PACKED *dst = (SENSOR_PACKED *) buffer;

            for (int i = 0; i < MAX_SENSOR; i++) {
                dst->entry[i].enable = gSensorCfg.entry[i].enable;
                dst->entry[i].type = gSensorCfg.entry[i].type;
                dst->entry[i].indexOfType = gSensorCfg.entry[i].indexOfType;
                dst->entry[i].calibrated = gSensorCfg.entry[i].calibrated;
                dst->entry[i].typeStatus = gSensorCfg.entry[i].typeStatus;
                dst->entry[i].typeGood = gSensorCfg.entry[i].typeGood;
                dst->entry[i].indexOfTypeGood = gSensorCfg.entry[i].indexOfTypeGood;
                dst->entry[i].typeCalib = gSensorCfg.entry[i].typeCalib;
                dst->entry[i].indexOfTypeCalib = gSensorCfg.entry[i].indexOfTypeCalib;
                dst->entry[i].typeErr = gSensorCfg.entry[i].typeErr;
                dst->entry[i].indexOfTypeErr = gSensorCfg.entry[i].indexOfTypeErr;
                dst->entry[i].goodValueAND = gSensorCfg.entry[i].goodValueAND;
                dst->entry[i].goodValueCompare = gSensorCfg.entry[i].goodValueCompare;
                dst->entry[i].calibValueAND = gSensorCfg.entry[i].calibValueAND;
                dst->entry[i].calibValueCompare = gSensorCfg.entry[i].calibValueCompare;
                dst->entry[i].errorValueAND = gSensorCfg.entry[i].errorValueAND;
                dst->entry[i].errorValueCompare = gSensorCfg.entry[i].errorValueCompare;
            }
            dst->numSensor = gSensorCfg.numSensor;

            uint16_t datalen = sizeof (SENSOR_PACKED) - sizeof (EXTFL_METADATA);
            dst->metadata.magic = EXTFL_MAGIC;
            dst->metadata.length = datalen;
            dst->metadata.type = (uint8_t) EXTFL_DATA_SENSOR_CFG;
            dst->metadata.crc = Helpers_CRC32Calculate((uint8_t *) & dst->entry, datalen);
            break;
        }

        case EXTFL_DATA_ANALOG_CFG:
        {
            if (maxBufferSize < sizeof (ANALOG_PACKED))
                return false;

            ANALOG_PACKED *dst = (ANALOG_PACKED *) buffer;

            for (int i = 0; i < MAX_ANALOG_CHANNEL; i++) {
                dst->entry[i].enable = gAnalogCfg.entry[i].enable;
                memcpy(dst->entry[i].name, gAnalogCfg.entry[i].name, SENSOR_NAME_LEN);
                memcpy(dst->entry[i].unit, gAnalogCfg.entry[i].unit, SENSOR_UNIT_LEN);

                dst->entry[i].scaleType = gAnalogCfg.entry[i].scaleType;
                dst->entry[i].scaleDataType = gAnalogCfg.entry[i].scaleDataType;
                dst->entry[i].scaleValue = gAnalogCfg.entry[i].scaleValue;

                dst->entry[i].adcType = gAnalogCfg.entry[i].adcType;
                dst->entry[i].adcLow = gAnalogCfg.entry[i].adcLow;
                dst->entry[i].adcHigh = gAnalogCfg.entry[i].adcHigh;
                dst->entry[i].offsetPreVal = gAnalogCfg.entry[i].offsetPreVal;
                dst->entry[i].offsetSubVal = gAnalogCfg.entry[i].offsetSubVal;
                dst->entry[i].offSetPreOperator = gAnalogCfg.entry[i].offSetPreOperator;
                dst->entry[i].offsetSubOperator = gAnalogCfg.entry[i].offsetSubOperator;
            }

            uint16_t datalen = sizeof (ANALOG_PACKED) - sizeof (EXTFL_METADATA);
            dst->metadata.magic = EXTFL_MAGIC;
            dst->metadata.length = datalen;
            dst->metadata.type = (uint8_t) EXTFL_DATA_ANALOG_CFG;
            dst->metadata.crc = Helpers_CRC32Calculate((uint8_t *) & dst->entry, datalen);
            break;
        }

        case EXTFL_DATA_MBRTU_CFG:
        {
            if (maxBufferSize < sizeof (MODBUSRTU_TAG_PACKED))
                return false;

            MODBUSRTU_TAG_PACKED *dst = (MODBUSRTU_TAG_PACKED *) buffer;

            for (int i = 0; i < MAX_MODBUS_TAG; i++) {
                dst->entry[i].enable = gMbrtuCfg.entry[i].enable;
                memcpy(dst->entry[i].name, gMbrtuCfg.entry[i].name, SENSOR_NAME_LEN);
                memcpy(dst->entry[i].unit, gMbrtuCfg.entry[i].unit, SENSOR_UNIT_LEN);

                dst->entry[i].slaveAddress = gMbrtuCfg.entry[i].slaveAddress;
                dst->entry[i].function = gMbrtuCfg.entry[i].function;
                dst->entry[i].regAddress = gMbrtuCfg.entry[i].regAddress;
                dst->entry[i].quantity = gMbrtuCfg.entry[i].quantity;
                dst->entry[i].rawDataType = gMbrtuCfg.entry[i].rawDataType;
                dst->entry[i].bigEndian = gMbrtuCfg.entry[i].bigEndian;

                dst->entry[i].scaleType = gMbrtuCfg.entry[i].scaleType;
                dst->entry[i].scaleDataType = gMbrtuCfg.entry[i].scaleDataType;
                dst->entry[i].scaleValue = gMbrtuCfg.entry[i].scaleValue;

                dst->entry[i].adcType = gMbrtuCfg.entry[i].adcType;
                dst->entry[i].adcLow = gMbrtuCfg.entry[i].adcLow;
                dst->entry[i].adcHigh = gMbrtuCfg.entry[i].adcHigh;
                dst->entry[i].offsetPreVal = gMbrtuCfg.entry[i].offsetPreVal;
                dst->entry[i].offsetSubVal = gMbrtuCfg.entry[i].offsetSubVal;
                dst->entry[i].offSetPreOperator = gMbrtuCfg.entry[i].offSetPreOperator;
                dst->entry[i].offsetSubOperator = gMbrtuCfg.entry[i].offsetSubOperator;
            }
            dst->numTag = gMbrtuCfg.numTag;

            uint16_t datalen = sizeof (MODBUSRTU_TAG_PACKED) - sizeof (EXTFL_METADATA);
            dst->metadata.magic = EXTFL_MAGIC;
            dst->metadata.length = datalen;
            dst->metadata.type = (uint8_t) EXTFL_DATA_MBRTU_CFG;
            dst->metadata.crc = Helpers_CRC32Calculate((uint8_t *) & dst->entry, datalen);
            break;
        }

        case EXTFL_DATA_INCAPTURE_CFG:
        {
            if (maxBufferSize < sizeof (INPUT_CAPTURE_PACKED))
                return false;

            INPUT_CAPTURE_PACKED *dst = (INPUT_CAPTURE_PACKED *) buffer;

            for (int i = 0; i < MAX_INPUT_CAPTURE; i++) {
                dst->entry[i].enable = gInCaptureCfg.entry[i].enable;
                memcpy(dst->entry[i].name, gInCaptureCfg.entry[i].name, SENSOR_NAME_LEN);
                memcpy(dst->entry[i].unit, gInCaptureCfg.entry[i].unit, SENSOR_UNIT_LEN);

                dst->entry[i].valPerPulse = gInCaptureCfg.entry[i].valPerPulse;
                dst->entry[i].minFreq = gInCaptureCfg.entry[i].minFreq;
                dst->entry[i].scale = gInCaptureCfg.entry[i].scale;
            }

            uint16_t datalen = sizeof (INPUT_CAPTURE_PACKED) - sizeof (EXTFL_METADATA);
            dst->metadata.magic = EXTFL_MAGIC;
            dst->metadata.length = datalen;
            dst->metadata.type = (uint8_t) EXTFL_DATA_INCAPTURE_CFG;
            dst->metadata.crc = Helpers_CRC32Calculate((uint8_t *) & dst->entry, datalen);
            break;
        }

        default:
            return false;
    }
    return true;
}

void ExtFlash_Initialize() {
    _writeState = EXTFL_WRITE_IDLE;
    _readState = EXTFL_READ_IDLE;
    _currentAddrFlash = 0;
    iqueue_init(&_writeQueue, EXTFL_QUEUE_SIZE, sizeof (EXTFL_QUEUE_ENTRY), &_writeQueueStorage);
    iqueue_init(&_readQueue, EXTFL_QUEUE_SIZE, sizeof (EXTFL_QUEUE_ENTRY), &_readQueueStorage);
}

void ExtFlash_Task() {
    static EXTFL_QUEUE_ENTRY qEntry;
    static uint32_t timeoutTick = 0;
    static uint32_t openTick = 0;
    static EXTFL_RESULT rslt = 0;

    if (!_openStatus) {
        if (!TIME_IS_EXPIRED(openTick, 100))
            return;

        openTick = TICK_NOW();
        _openStatus = DRV_SST26_IsOpened();
        if (_openStatus)
            SYS_CONSOLE_PRINT("%s - %s\t Init SUCCESS\r\n", __TAG__, __func__);
        else
            SYS_CONSOLE_PRINT("%s - %s\t Init FAIL\r\n", __TAG__, __func__);

        return;
    }

    switch (_writeState) {
        case EXTFL_WRITE_IDLE:
        {
            size_t queueSize;
            iqueue_size(&_writeQueue, &queueSize);
            if (queueSize == 0) break;

            if (iqueue_dequeue(&_writeQueue, &qEntry) == I_OK) {
                timeoutTick = TICK_NOW();
                memset(_transferBuffer, 0, sizeof (_transferBuffer));
                if (_packBufferSave(_transferBuffer, sizeof (_transferBuffer), qEntry.type)) {
                    DCACHE_CLEAN_BY_ADDR((uint32_t) _transferBuffer, qEntry.size);
                    _writeState = EXTFL_WRITE_ERASE_SECTOR;
                }
            }
            break;
        }

        case EXTFL_WRITE_ERASE_SECTOR:
        {
            if (TIME_IS_EXPIRED(timeoutTick, EXTFL_TIMEOUT_TRANSFER)) {
                _writeState = EXTFL_WRITE_DONE;
                rslt = EXTFL_TRANSFER_TIMEOUT;
                break;
            }
            if (_transferIsBusy()) break;

            _currentAddrFlash = _partition[qEntry.type].startAddress;
            if (!_eraseSectorFlash(_currentAddrFlash)) {
                _writeState = EXTFL_WRITE_DONE;
                rslt = EXTFL_ERASE_FAIL;
                break;
            }

            timeoutTick = TICK_NOW();
            _writeState = EXTFL_WRITE_TRANSFER;
            break;
        }
        case EXTFL_WRITE_TRANSFER:
        {
            bool finish;
            if (TIME_IS_EXPIRED(timeoutTick, EXTFL_TIMEOUT_TRANSFER)) {
                _writeState = EXTFL_WRITE_DONE;
                rslt = EXTFL_TRANSFER_TIMEOUT;
                break;
            }
            if (_transferIsBusy()) break;

            if (!_writeStreamByteToFlash(_transferBuffer, qEntry.size, &finish)) {
                _writeState = EXTFL_WRITE_DONE;
                rslt = EXTFL_WRITE_FAIL;
                break;
            }
            if (!finish) break;

            rslt = EXTFL_SUCCESS;
            _writeState = EXTFL_WRITE_DONE;
            break;
        }
        case EXTFL_WRITE_DONE:
        {
            SYS_CONSOLE_PRINT("%s - %s:\t write done, result=%u\r\n", __TAG__, __func__, rslt);
            if (qEntry.callback) qEntry.callback(qEntry.type, rslt);
            _writeState = EXTFL_WRITE_IDLE;
            break;
        }
    }


    size_t wQueueSize;
    iqueue_size(&_writeQueue, &wQueueSize);
    if (_writeState != EXTFL_WRITE_IDLE || wQueueSize != 0) return;

    switch (_readState) {
        case EXTFL_READ_IDLE:
        {
            size_t queueSize;
            iqueue_size(&_readQueue, &queueSize);
            if (queueSize == 0) break;

            if (iqueue_dequeue(&_readQueue, &qEntry) == I_OK) {
                timeoutTick = TICK_NOW();
                _readState = EXTFL_READ_TRANSFER;
            }
            break;
        }
        case EXTFL_READ_TRANSFER:
        {
            uint32_t address = _partition[qEntry.type].startAddress;
            if (!_readFlash(address, qEntry.size)) {
                _readState = EXTFL_READ_DONE;
                rslt = EXTFL_READ_FAIL;
                break;
            }

            timeoutTick = TICK_NOW();
            _readState = EXTFL_READ_WAIT_TRANSFER;
            break;
        }
        case EXTFL_READ_WAIT_TRANSFER:
        {
            if (TIME_IS_EXPIRED(timeoutTick, EXTFL_TIMEOUT_TRANSFER)) {
                _readState = EXTFL_READ_DONE;
                rslt = EXTFL_TRANSFER_TIMEOUT;
                break;
            }
            if (_transferIsBusy()) break;

            _readState = EXTFL_READ_VERIFY_DATA;
            break;
        }
        case EXTFL_READ_VERIFY_DATA:
        {
            bool res = false;
            res = _unpackBufferLoaded(_transferBuffer, qEntry.size, qEntry.type);

            if (!res) rslt = EXTFL_VERIFY_FAIL;
            else rslt = EXTFL_SUCCESS;
            _readState = EXTFL_READ_DONE;
            break;
        }
        case EXTFL_READ_DONE:
        {
            SYS_CONSOLE_PRINT("%s - %s:\t read done, result=%u\r\n", __TAG__, __func__, rslt);
            if (qEntry.callback) qEntry.callback(qEntry.type, rslt);
            _readState = EXTFL_WRITE_IDLE;
            break;
        }
    }
}

bool ExtFlash_SaveConfig(EXTFL_DATA_TYPE dataType, void (*clb)(int type, int rlst)) {
    EXTFL_QUEUE_ENTRY entry = {
        .type = dataType,
        .callback = clb,
        .size = _calculateDataSize(dataType)
    };

    i_status ret = iqueue_enqueue(&_writeQueue, &entry);
    //    SYS_CONSOLE_PRINT("%s - %s:\t read done, result=%u\r\n", __TAG__, __func__, ret);
    return (ret == I_OK);
}

bool ExtFlash_LoadConfig(EXTFL_DATA_TYPE dataType, void (*clb)(int type, int rlst)) {
    EXTFL_QUEUE_ENTRY entry = {
        .type = dataType,
        .callback = clb,
        .size = _calculateDataSize(dataType)
    };

    i_status ret = iqueue_enqueue(&_readQueue, &entry);
    //    SYS_CONSOLE_PRINT("%s - %s:\t read done, result=%u\r\n", __TAG__, __func__, ret);
    return (ret == I_OK);
}