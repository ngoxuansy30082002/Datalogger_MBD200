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

static SENSOR_PACKED _sensorPacked = {0};
static APP_PACKED _appPacked = {0};
static MODBUSRTU_TAG_PACKED _mbTagPacked = {0};
static ANALOG_PACKED _analogPacked = {0};
static INPUT_CAPTURE_PACKED _inCapturePacked = {0};
static DEVICE_INFO_PACKED _deviceInfoPacked = {0};

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
    GPIO_PinWrite(_bootCfglib.led1, level);
    GPIO_PinWrite(_bootCfglib.led2, level);
    GPIO_PinWrite(_bootCfglib.led3, level);
    GPIO_PinWrite(_bootCfglib.led4, level);
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


    memset((void *) &_sensorPacked, 0x00, sizeof (_sensorPacked));
    memset((void *) &_appPacked, 0x00, sizeof (_appPacked));
    memset((void *) &_mbTagPacked, 0x00, sizeof (_mbTagPacked));
    memset((void *) &_analogPacked, 0x00, sizeof (_analogPacked));
    memset((void *) &_inCapturePacked, 0x00, sizeof (_inCapturePacked));


    _appPacked.network.isDHCPEn = true;
    TCPIP_Helper_StringToIPAddress(TCPIP_NETWORK_DEFAULT_IP_ADDRESS_IDX0, &_appPacked.network.ipAddr);
    TCPIP_Helper_StringToIPAddress(TCPIP_NETWORK_DEFAULT_IP_MASK_IDX0, &_appPacked.network.ipMask);
    TCPIP_Helper_StringToIPAddress(TCPIP_NETWORK_DEFAULT_GATEWAY_IDX0, &_appPacked.network.gateway);
    TCPIP_Helper_StringToIPAddress(TCPIP_NETWORK_DEFAULT_DNS_IDX0, &_appPacked.network.primaryDNS);
    TCPIP_Helper_StringToIPAddress(TCPIP_NETWORK_DEFAULT_SECOND_DNS_IDX0, &_appPacked.network.secondDNS);
    snprintf(_appPacked.network.netBIOSName, BIOS_NAME_LEN, "%s", SERIAL);
    snprintf(_appPacked.network.deviceUsername, USERNAME_LEN, "%s", DEFAULT_USERNAME_DEVICE);
    snprintf(_appPacked.network.devicePassword, PASSWORD_LEN, "%s", DEFAULT_PASSWORD_DEVICE);

    _appPacked.modbusRtu.baudRate = MBRTU_BAUD_RATE; // baud modbus
    _appPacked.modbusRtu.timeout = MBRTU_TIMEOUT; // time timeout
    _appPacked.modbusRtu.retries = MBRTU_RETRIES; // retries
    _appPacked.modbusRtu.pollInterval = MBRTU_POLL_INTERVAL; // poll interval
    _appPacked.modbusRtu.stopbits = MBRTU_STOP_BITS;
    _appPacked.modbusRtu.parity = MBRTU_PARITY;

    for (uint8_t i = 0; i < MAX_FTP_SERVER; i++) {
        snprintf(_appPacked.ftpServer[i].username, USERNAME_LEN, "%s", FTP_USER);
        snprintf(_appPacked.ftpServer[i].password, PASSWORD_LEN, "%s", FTP_PASS);
        snprintf(_appPacked.ftpServer[i].dirPath, DIR_PATH_LEN, "%s", FTP_PATH);
        snprintf(_appPacked.ftpServer[i].hostname, URL_LEN, "%s", FTP_HOST);
        _appPacked.ftpServer[i].port = FTP_PORT;
        _appPacked.ftpServer[i].makeFolder = MAKE_FOLDER_NONE;
        snprintf(_appPacked.ftpServer[i].namePrefix, FILE_NAME_PREFIX_LEN, "%s", FTP_NAME_PREFIX);
        _appPacked.ftpServer[i].enable = false;
    }

    _appPacked.logFile.uplink = UPLINK_ETH;
    _appPacked.logFile.typefile = FILE_TYPE_TXT;
    _appPacked.logFile.formatFile = FORMAT_FILE_TT24;
    _appPacked.logFile.timeMode = TIME_MODE_OCLOCK;
    _appPacked.logFile.sendInterval = 2;

    snprintf(_appPacked.gsm.APN, APN_LEN, "%s", MY_APN);
    snprintf(_appPacked.gsm.usernameAPN, USERNAME_LEN, "%s", USERNAME_APN);
    snprintf(_appPacked.gsm.passwordAPN, PASSWORD_LEN, "%s", PASSWORD_APN);

    memset(_appPacked.position, 1, sizeof (_appPacked.position));

    for (uint8_t i = 0; i < MAX_DIGITAL_OUTPUT; i++) {
        snprintf(_appPacked.io.out[i].describe, SENSOR_NAME_LEN, "Description");
        _appPacked.io.out[i].time = 0;
        _appPacked.io.out[i].type = OUT_HOLD;
    }

    _appPacked.sdCard.retentionMonths = SDCARD_TIME_REMOVE;

    _appPacked.time.yearNumber = 20;
    _appPacked.time.timeAuto = 0;
    _appPacked.time.indexNTP = 0;
    _appPacked.time.timeZone = 7;

    memset(_appPacked.hmi, 30, sizeof (_appPacked.hmi));



    _mbTagPacked.numTag = 0;
    memset(_mbTagPacked.entry, 0, sizeof (_mbTagPacked.entry));
    for (uint8_t i = 0; i < MAX_MODBUS_TAG; i++) {
        snprintf(_mbTagPacked.entry[i].name, SENSOR_NAME_LEN, "Unused");
        snprintf(_mbTagPacked.entry[i].unit, SENSOR_UNIT_LEN, "Unused");
    }



    memset(_analogPacked.entry, 0, sizeof (_analogPacked.entry));
    for (uint8_t i = 0; i < MAX_ANALOG_CHANNEL; i++) {
        if (i < 4) _analogPacked.entry[i].adcType = ADC_4_20mA;
        else _analogPacked.entry[i].adcType = ADC_0_10V;
        snprintf(_analogPacked.entry[i].name, SENSOR_NAME_LEN, "Unused");
        snprintf(_analogPacked.entry[i].unit, SENSOR_UNIT_LEN, "Unused");
    }



    for (uint8_t i = 0; i < MAX_INPUT_CAPTURE; i++) {
        if (i & 1) {
            snprintf(_inCapturePacked.entry[i].name, SENSOR_NAME_LEN, "TotalFlow%u", i);
            snprintf(_inCapturePacked.entry[i].unit, SENSOR_UNIT_LEN, "%s", "m3");
        } else {
            snprintf(_inCapturePacked.entry[i].name, SENSOR_NAME_LEN, "Flowrate%u", i);
            snprintf(_inCapturePacked.entry[i].unit, SENSOR_UNIT_LEN, "%s", "m3/h");
        }
        _inCapturePacked.entry[i].enable = false;
        _inCapturePacked.entry[i].scale = 3600;
        _inCapturePacked.entry[i].valPerPulse = 1;
        _inCapturePacked.entry[i].minFreq = 1;
    }



    _sensorPacked.numSensor = 0;
    memset(_sensorPacked.entry, 0, sizeof (_sensorPacked.entry));
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
            _setLed(false);
            return true;
    }
    return false;
}