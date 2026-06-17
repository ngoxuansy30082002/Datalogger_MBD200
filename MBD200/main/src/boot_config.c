#include "boot_config.h"

APP_CONFIG gAppCfg = {0};
ANALOG_CONFIG gAnalogCfg = {0};
MODBUSRTU_TAG_CONFIG gMbrtuCfg = {0};
INPUT_CAPTURE_CONFIG gInCaptureCfg = {0};
SENSOR_CONFIG gSensorCfg = {0};

static const char * __TAG__ = "BOOTCFG";
static const BOOT_CONFIG_PLIB _bootCfglib = {
    .button = GPIO_PIN_RB4,
    .led1 = GPIO_PIN_RG13,
    .led2 = GPIO_PIN_RE2,
    .led3 = GPIO_PIN_RE3,
    .led4 = GPIO_PIN_RE4,
};


static BOOT_CONFIG_STATES _states = 0;
static bool _saveFlag = false;
static int8_t _savePending = 0;
static int8_t _loadPending = 0;
static BOOT_CONFIG_FLAG _fsave = {.val = 0};
static BOOT_CONFIG_FLAG _fload = {.val = 0};

static uint32_t _lockSaveFlash = 0;
static uint8_t _firstSave = 7;
static int8_t _numFail = 3;

static bool isLockSaveFlash();
static bool _WaitForFlashTransfer(uint32_t timeoutMs);

static inline void _toggleLed() {
    GPIO_PinToggle(_bootCfglib.led1);
    GPIO_PinToggle(_bootCfglib.led2);
    GPIO_PinToggle(_bootCfglib.led3);
    GPIO_PinToggle(_bootCfglib.led4);
}

static inline void _setLed(bool level) {
    GPIO_PinWrite(_bootCfglib.led1, !level);
    GPIO_PinWrite(_bootCfglib.led2, !level);
    GPIO_PinWrite(_bootCfglib.led3, !level);
    GPIO_PinWrite(_bootCfglib.led4, !level);
}

static inline bool _isButtonPressed() {
    return (GPIO_PinRead(_bootCfglib.button) == 0);
}

static void _saveCallbackHandle(int type, int rlst) {
    _savePending -= (_savePending == 0) ? 0 : 1;
    if (rlst == EXTFL_SUCCESS) {
        switch ((EXTFL_DATA_TYPE) type) {
            case EXTFL_DATA_APP_CFG:
                _fsave.bits.appCfg = 0;
                break;
            case EXTFL_DATA_SENSOR_CFG:
                _fsave.bits.sensorCfg = 0;
                break;
            case EXTFL_DATA_ANALOG_CFG:
                _fsave.bits.analogCfg = 0;
                break;
            case EXTFL_DATA_MBRTU_CFG:
                _fsave.bits.mbRtuCfg = 0;
                break;
            case EXTFL_DATA_INCAPTURE_CFG:
                _fsave.bits.inCaptureCfg = 0;
                break;
            default: break;
        }
        SYS_CONSOLE_PRINT("%s - %s:\t Type %u Success \r\n", __TAG__, __func__, type);
    }
}

static void _loadCallbackHandle(int type, int rlst) {
    _loadPending -= (_loadPending == 0) ? 0 : 1;
    if (rlst == EXTFL_SUCCESS) {
        switch ((EXTFL_DATA_TYPE) type) {
            case EXTFL_DATA_APP_CFG:
                _fload.bits.appCfg = 0;
                break;
            case EXTFL_DATA_SENSOR_CFG:
                _fload.bits.sensorCfg = 0;
                break;
            case EXTFL_DATA_ANALOG_CFG:
                _fload.bits.analogCfg = 0;
                break;
            case EXTFL_DATA_MBRTU_CFG:
                _fload.bits.mbRtuCfg = 0;
                break;
            case EXTFL_DATA_INCAPTURE_CFG:
                _fload.bits.inCaptureCfg = 0;
                break;
            default: break;
        }
        SYS_CONSOLE_PRINT("%s - %s:\t Type %u Success \r\n", __TAG__, __func__, type);
    }
}

static void _initFlagSave(bool firstSave) {
    _fsave.val = 0;
    _fsave.bits.appCfg = (firstSave) ? 1 : _fload.bits.appCfg;
    _fsave.bits.sensorCfg = (firstSave) ? 1 : _fload.bits.sensorCfg;
    _fsave.bits.analogCfg = (firstSave) ? 1 : _fload.bits.analogCfg;
    _fsave.bits.mbRtuCfg = (firstSave) ? 1 : _fload.bits.mbRtuCfg;
    _fsave.bits.inCaptureCfg = (firstSave) ? 1 : _fload.bits.inCaptureCfg;
}

static void _initFlagLoad() {
    _fload.val = 0;
    _fload.bits.appCfg = 1;
    _fload.bits.sensorCfg = 1;
    _fload.bits.analogCfg = 1;
    _fload.bits.mbRtuCfg = 1;
    _fload.bits.inCaptureCfg = 1;
}

void BootConfig_Initialize() {
    _initFlagSave(true);
    _initFlagLoad();
    _setLed(true);


    memset((void *) &gSensorCfg, 0x00, sizeof (gSensorCfg));
    memset((void *) &gAppCfg, 0x00, sizeof (gAppCfg));
    memset((void *) &gMbrtuCfg, 0x00, sizeof (gMbrtuCfg));
    memset((void *) &gAnalogCfg, 0x00, sizeof (gAnalogCfg));
    memset((void *) &gInCaptureCfg, 0x00, sizeof (gInCaptureCfg));


    gAppCfg.network.isDHCPEn = true;
    gAppCfg.network.uplink = UPLINK_ETH;
    TCPIP_Helper_StringToIPAddress(TCPIP_NETWORK_DEFAULT_IP_ADDRESS_IDX0, &gAppCfg.network.ipAddr);
    TCPIP_Helper_StringToIPAddress(TCPIP_NETWORK_DEFAULT_IP_MASK_IDX0, &gAppCfg.network.ipMask);
    TCPIP_Helper_StringToIPAddress(TCPIP_NETWORK_DEFAULT_GATEWAY_IDX0, &gAppCfg.network.gateway);
    TCPIP_Helper_StringToIPAddress(TCPIP_NETWORK_DEFAULT_DNS_IDX0, &gAppCfg.network.primaryDNS);
    TCPIP_Helper_StringToIPAddress(TCPIP_NETWORK_DEFAULT_SECOND_DNS_IDX0, &gAppCfg.network.secondDNS);
    snprintf(gAppCfg.network.netBIOSName, BIOS_NAME_LEN, "%s", SERIAL);
    snprintf(gAppCfg.network.deviceUsername, USERNAME_LEN, "%s", DEFAULT_USERNAME_DEVICE);
    snprintf(gAppCfg.network.devicePassword, PASSWORD_LEN, "%s", DEFAULT_PASSWORD_DEVICE);

    gAppCfg.modbusRtu.baudRate = MBRTU_BAUD_RATE; // baud modbus
    gAppCfg.modbusRtu.timeout = MBRTU_TIMEOUT; // time timeout
    gAppCfg.modbusRtu.retries = MBRTU_RETRIES; // retries
    gAppCfg.modbusRtu.pollInterval = MBRTU_POLL_INTERVAL; // poll interval
    gAppCfg.modbusRtu.stopbits = MBRTU_STOP_BITS;
    gAppCfg.modbusRtu.parity = MBRTU_PARITY;

    for (uint8_t i = 0; i < MAX_FTP_SERVER; i++) {
        snprintf(gAppCfg.ftpServer[i].username, USERNAME_LEN, "%s", FTP_USER);
        snprintf(gAppCfg.ftpServer[i].password, PASSWORD_LEN, "%s", FTP_PASS);
        snprintf(gAppCfg.ftpServer[i].dirPath, DIR_PATH_LEN, "%s", FTP_PATH);
        snprintf(gAppCfg.ftpServer[i].hostname, URL_LEN, "%s", FTP_HOST);
        gAppCfg.ftpServer[i].port = FTP_PORT;
        gAppCfg.ftpServer[i].makeFolder = MAKE_FOLDER_NONE;
        gAppCfg.ftpServer[i].enable = false;
    }

    snprintf(gAppCfg.gsm.APN, APN_LEN, "%s", MY_APN);
    snprintf(gAppCfg.gsm.usernameAPN, USERNAME_LEN, "%s", USERNAME_APN);
    snprintf(gAppCfg.gsm.passwordAPN, PASSWORD_LEN, "%s", PASSWORD_APN);

    memset(gAppCfg.position, 1, sizeof (gAppCfg.position));

    for (uint8_t i = 0; i < MAX_DIGITAL_OUTPUT; i++) {
        snprintf(gAppCfg.io.out[i].name, SENSOR_NAME_LEN, "-");
        snprintf(gAppCfg.io.out[i].describe, SENSOR_NAME_LEN, "-");
    }

    gAppCfg.sdCard.retentionMonths = SDCARD_TIME_REMOVE;

    gAppCfg.time.yearNumber = 20;
    gAppCfg.time.timeZone = 7;
    gAppCfg.time.ntpPort = 123;
    gAppCfg.time.syncInterval = 3600;
    snprintf(gAppCfg.time.ntpServerPrimary, URL_LEN, "%s", NTP_SERVER_PRIMARY);
    snprintf(gAppCfg.time.ntpServerBackup, URL_LEN, "%s", NTP_SERVER_BACKUP);

    gAppCfg.hmi.numEntry = 0;
    memset(gAppCfg.hmi.sensorIdx, 0, MAX_HMI_PARA * sizeof (uint8_t));



    gMbrtuCfg.numTag = 0;
    memset(gMbrtuCfg.entry, 0, sizeof (gMbrtuCfg.entry));
    for (uint8_t i = 0; i < MAX_MODBUS_TAG; i++) {
        snprintf(gMbrtuCfg.entry[i].name, SENSOR_NAME_LEN, "-");
        snprintf(gMbrtuCfg.entry[i].unit, SENSOR_UNIT_LEN, "-");
    }



    memset(gAnalogCfg.entry, 0, sizeof (gAnalogCfg.entry));
    for (uint8_t i = 0; i < MAX_ANALOG_CHANNEL; i++) {
        if (i < 4) gAnalogCfg.entry[i].adcType = ADC_4_20mA;
        else gAnalogCfg.entry[i].adcType = ADC_0_10V;
        snprintf(gAnalogCfg.entry[i].name, SENSOR_NAME_LEN, "-");
        snprintf(gAnalogCfg.entry[i].unit, SENSOR_UNIT_LEN, "-");
    }



    for (uint8_t i = 0; i < MAX_INPUT_CAPTURE; i++) {
        snprintf(gInCaptureCfg.entry[i].name, SENSOR_NAME_LEN, "-");
        snprintf(gInCaptureCfg.entry[i].unit, SENSOR_UNIT_LEN, "-");
    }



    gSensorCfg.numSensor = 0;
    memset(gSensorCfg.entry, 0, sizeof (gSensorCfg.entry));
    gSensorCfg.typefile = FILE_TYPE_TXT;
    gSensorCfg.formatFile = FORMAT_FILE_TT24;
    gSensorCfg.logInterval = 2;
    snprintf(gSensorCfg.filenameTemplate, FILE_NAME_LEN, "datalogger_{YYYY}{MM}{DD}{hh}{mm}{ss}");
    gSensorCfg.uploadFtp = true;
    gSensorCfg.saveSdcard = true;
}

bool BootConfig_Task(void) {
    static const uint8_t retryNum = 10;
    static uint8_t loadRetryCnt = 0;
    static uint8_t saveRetryCnt = 0;
    static uint32_t _buttonTick = 0;
    static uint32_t _ledBlinkTick = 0;

    switch (_states) {
        case BOOT_CONFIG_INIT:
            if (_isButtonPressed()) _states = BOOT_CONFIG_BTN_HOLD;
            else _states = BOOT_CONFIG_LOAD;
            _ledBlinkTick = TICK_NOW();
            _buttonTick = TICK_NOW();
            break;

        case BOOT_CONFIG_BTN_HOLD:
            if (TIME_IS_EXPIRED(_ledBlinkTick, 500)) {
                _ledBlinkTick = TICK_NOW();
                _toggleLed();
            }

            if (TIME_IS_EXPIRED(_buttonTick, 10000)) {
                _buttonTick = TICK_NOW();
                _states = BOOT_CONFIG_WAIT;
            }
            if (!_isButtonPressed()) _states = BOOT_CONFIG_LOAD;
            break;

        case BOOT_CONFIG_WAIT:
            if (TIME_IS_EXPIRED(_ledBlinkTick, 100)) {
                _ledBlinkTick = TICK_NOW();
                _toggleLed();
            }

            if (TIME_IS_EXPIRED(_buttonTick, 10000)) {
                _buttonTick = TICK_NOW();
                _states = BOOT_CONFIG_LOAD;
            }
            if (!_isButtonPressed()) _states = BOOT_CONFIG_WAIT_COMFIRM;
            break;

        case BOOT_CONFIG_WAIT_COMFIRM:
            if (TIME_IS_EXPIRED(_ledBlinkTick, 100)) {
                _ledBlinkTick = TICK_NOW();
                _toggleLed();
            }

            if (_isButtonPressed()) {
                _saveFlag = true;
                _states = BOOT_CONFIG_SAVE;
            }
            break;

        case BOOT_CONFIG_SAVE:
            if (_fsave.bits.appCfg) {
                if (ExtFlash_SaveConfig(EXTFL_DATA_APP_CFG, _saveCallbackHandle))
                    _savePending++;
            }
            if (_fsave.bits.sensorCfg) {
                if (ExtFlash_SaveConfig(EXTFL_DATA_SENSOR_CFG, _saveCallbackHandle))
                    _savePending++;
            }
            if (_fsave.bits.analogCfg) {
                if (ExtFlash_SaveConfig(EXTFL_DATA_ANALOG_CFG, _saveCallbackHandle))
                    _savePending++;
            }
            if (_fsave.bits.inCaptureCfg) {
                if (ExtFlash_SaveConfig(EXTFL_DATA_INCAPTURE_CFG, _saveCallbackHandle))
                    _savePending++;
            }
            if (_fsave.bits.mbRtuCfg) {
                if (ExtFlash_SaveConfig(EXTFL_DATA_MBRTU_CFG, _saveCallbackHandle))
                    _savePending++;
            }
            SYS_CONSOLE_PRINT("%s - %s:\t Save config pending %u\r\n", __TAG__, __func__, _savePending);
            _states = BOOT_CONFIG_WAIT_SAVE;
            break;

        case BOOT_CONFIG_WAIT_SAVE:
            if (_savePending > 0) break;

            if (_fsave.val == 0) _states = BOOT_CONFIG_LOAD;
            else {
                if (saveRetryCnt++ > retryNum) {
                    saveRetryCnt = 0;
                    _states = BOOT_CONFIG_FAULT;
                    break;
                }
                _states = BOOT_CONFIG_SAVE;
            }
            break;

        case BOOT_CONFIG_LOAD:
            if (_fload.bits.appCfg) {
                if (ExtFlash_LoadConfig(EXTFL_DATA_APP_CFG, _loadCallbackHandle))
                    _loadPending++;
            }
            if (_fload.bits.sensorCfg) {
                if (ExtFlash_LoadConfig(EXTFL_DATA_SENSOR_CFG, _loadCallbackHandle))
                    _loadPending++;
            }
            if (_fload.bits.analogCfg) {
                if (ExtFlash_LoadConfig(EXTFL_DATA_ANALOG_CFG, _loadCallbackHandle))
                    _loadPending++;
            }
            if (_fload.bits.inCaptureCfg) {
                if (ExtFlash_LoadConfig(EXTFL_DATA_INCAPTURE_CFG, _loadCallbackHandle))
                    _loadPending++;
            }
            if (_fload.bits.mbRtuCfg) {
                if (ExtFlash_LoadConfig(EXTFL_DATA_MBRTU_CFG, _loadCallbackHandle))
                    _loadPending++;
            }
            SYS_CONSOLE_PRINT("%s - %s:\t Load config pending %u\r\n", __TAG__, __func__, _loadPending);
            _states = BOOT_CONFIG_WAIT_LOAD;
            break;

        case BOOT_CONFIG_WAIT_LOAD:
            if (_loadPending > 0) break;

            if (_fload.val == 0) {
                loadRetryCnt = 0;
                _states = BOOT_CONFIG_COMPLETE;
            } else {
                if (loadRetryCnt++ > retryNum) {
                    loadRetryCnt = 0;
                    if (saveRetryCnt++ < retryNum) {
                        _initFlagSave(false);
                        _states = BOOT_CONFIG_SAVE;
                    } else _states = BOOT_CONFIG_FAULT;
                    break;
                }
                _states = BOOT_CONFIG_LOAD;
            }
            break;

        case BOOT_CONFIG_COMPLETE:
            SYS_CONSOLE_PRINT("%s - %s:\t Complete boot config \r\n", __TAG__, __func__);
            return true;
    }
    return false;
}