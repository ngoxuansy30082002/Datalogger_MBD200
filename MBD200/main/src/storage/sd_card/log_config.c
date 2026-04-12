#include "log_config.h"

static char _buffer[LOG_CONFIG_BUFFER_SIZE] = {0};
static uint16_t _bufferOffset = 0;
static uint32_t commitId = 0;
static uint32_t parrentId = 0;
static char _filepath[128] = {0};
static bool _setedHidden = false;

static const char* _getAuthorString(LOG_CONFIG_AUTHOR author) {
    switch (author) {
        case LOG_CONFIG_POWER_UP: return "Power-Up";
        case LOG_CONFIG_WEB: return "Web config";
        case LOG_CONFIG_SDCARD_IMPORT: return "SDcard Import";
        default: return "Unknown";
    }
}

static void _flush() {
    if (_bufferOffset > 0) {
        SYS_CONSOLE_PRINT("Logger flush %s\r\n", _filepath);
        SDcard_WriteLog(_filepath, _buffer);
        _bufferOffset = 0;
        _buffer[0] = '\0';
    }
}

static void _print(const char* format, ...) {
    static char temp[256];
    va_list args;
    va_start(args, format);
    vsnprintf(temp, sizeof (temp), format, args);
    va_end(args);

    uint16_t len = strlen(temp);

    if (_bufferOffset + len >= LOG_CONFIG_BUFFER_SIZE) {
        _flush();
    }

    strcat(_buffer, temp);
    _bufferOffset += len;
}

void Logger_SaveFullConfig(LOG_CONFIG_AUTHOR author) {
    if (author == LOG_CONFIG_POWER_UP) {
        commitId = 0;
        parrentId = 0;
    } else {
        parrentId = commitId;
        commitId++;
    }

    snprintf(_filepath, sizeof (_filepath), "%s/%04u%02u/%04u%02u%02u/configurations_%02u%02u%02u.cfg",
            LOG_CONFIG_DIRECTORY,
            rtcDt.sysTime.year, rtcDt.sysTime.month,
            rtcDt.sysTime.year, rtcDt.sysTime.month, rtcDt.sysTime.day,
            rtcDt.sysTime.hour, rtcDt.sysTime.minute, rtcDt.sysTime.second);
    _bufferOffset = 0;
    _buffer[0] = '\0';

    _print("[Metadata]\n");
    _print("CommitID=%lu\n", commitId);
    _print("ParentID=%lu\n", parrentId);
    _print("Timestamp=%04u-%02u-%02u %02u:%02u:%02u\n",
            rtcDt.sysTime.year, rtcDt.sysTime.month, rtcDt.sysTime.day, rtcDt.sysTime.hour, rtcDt.sysTime.minute, rtcDt.sysTime.second);
    _print("Author=%s\n", _getAuthorString(author));
    _print("\n");

    _print("[APP - Network]\n");
    _print("DHCPEnable=%d\n", gAppCfg.network.isDHCPEn);
    _print("NetBIOSName=%s\n", gAppCfg.network.netBIOSName);
    _print("AppUserDevice=%s\n", gAppCfg.network.deviceUsername);
    _print("AppPassDevice=%s\n", gAppCfg.network.devicePassword);
    _print("IPAddr=%u.%u.%u.%u\n", gAppCfg.network.ipAddr.v[0], gAppCfg.network.ipAddr.v[1], gAppCfg.network.ipAddr.v[2], gAppCfg.network.ipAddr.v[3]);
    _print("\n");

    _print("[APP - Log file]\n");
    _print("UpLink=%d\n", gAppCfg.logFile.uplink);
    _print("FormatData=%d\n", gAppCfg.logFile.formatFile);
    _print("FileType=%d\n", gAppCfg.logFile.typefile);
    _print("TimeMode=%d\n", gAppCfg.logFile.timeMode);
    _print("SendInterval=%u\n", gAppCfg.logFile.sendInterval);
    _print("\n");

    for (int i = 0; i < MAX_FTP_SERVER; i++) {
        _print("[APP - FTPServer%d]\n", i);
        _print("Enable=%d\n", gAppCfg.ftpServer[i].enable);
        _print("Hostname=%s\n", gAppCfg.ftpServer[i].hostname);
        _print("Port=%u\n", gAppCfg.ftpServer[i].port);
        _print("Path=%s\n", gAppCfg.ftpServer[i].dirPath);
        _print("Username=%s\n", gAppCfg.ftpServer[i].username);
        _print("Password=%s\n", gAppCfg.ftpServer[i].password);
        _print("NamePrefix=%s\n", gAppCfg.ftpServer[i].namePrefix);
        _print("MakeFolderType=%d\n", gAppCfg.ftpServer[i].makeFolder);
        _print("\n");
    }

    _print("[APP - ModbusRTU Phy]\n");
    _print("BaudRate=%lu\n", gAppCfg.modbusRtu.baudRate);
    _print("Parity=%u\n", gAppCfg.modbusRtu.parity);
    _print("Stopbit=%u\n", gAppCfg.modbusRtu.stopbits);
    _print("Timeout=%u\n", gAppCfg.modbusRtu.timeout);
    _print("PollInterval=%u\n", gAppCfg.modbusRtu.pollInterval);
    _print("Retries=%u\n", gAppCfg.modbusRtu.retries);
    _print("Latency=%u\n", gAppCfg.modbusRtu.latency);
    _print("\n");

    _print("[APP - GSM]\n");
    _print("APN=%s\n", gAppCfg.gsm.APN);
    _print("UserAPN=%s\n", gAppCfg.gsm.usernameAPN);
    _print("PassAPN=%s\n", gAppCfg.gsm.passwordAPN);
    _print("\n");

    _print("[APP - Time]\n");
    _print("NTPIndex=%u\n", gAppCfg.time.indexNTP);
    _print("TimeAuto=%u\n", gAppCfg.time.timeAuto);
    _print("\n");

    _print("[APP - Output]\n");
    _print("OUT1Describe=%s\n", gAppCfg.io.out[0].describe);
    _print("OUT1Type=%d\n", gAppCfg.io.out[0].type);
    _print("OUT1Time=%u\n", gAppCfg.io.out[0].time);
    _print("OUT2Describe=%s\n", gAppCfg.io.out[0].describe);
    _print("OUT2Type=%d\n", gAppCfg.io.out[0].type);
    _print("OUT2Time=%u\n", gAppCfg.io.out[0].time);
    _print("\n");

    _print("[APP - SDcard]\n");
    _print("TimeRemove=%u\n", gAppCfg.sdCard.retentionMonths);
    _print("\n");

    _print("[APP - HMI Tag]\n");
    for (int i = 0; i < MAX_HMI_PARA; i++) {
        _print("TagHMI%d=%u\n", i, gAppCfg.hmi[i]);
    }
    _print("\n");

    _print("[APP - ModbusTCP position]\n");
    for (int i = 0; i < MAX_POSITION_SIZE; i++) {
        _print("Position%d=%u\n", i, gAppCfg.position[i]);
    }
    _print("\n");

    _print("[SENSOR]\n");
    _print("TotalSensor=%u\n", gSensorCfg.numSensor);
    for (int i = 0; i < gSensorCfg.numSensor; i++) {
        _print("\t[Entry%d]\n", i);
        _print("\tEnable=%d\n", gSensorCfg.entry[i].enable);
        _print("\tType=%d\n", gSensorCfg.entry[i].type);
        _print("\tIndexOfType=%u\n", gSensorCfg.entry[i].indexOfType);
        _print("\tCalibrated=%d\n", gSensorCfg.entry[i].calibrated);
        _print("\tTypeSuccess=%d\n", gSensorCfg.entry[i].typeGood);
        _print("\tIndexOfTypeSuccess=%u\n", gSensorCfg.entry[i].indexOfTypeGood);
        _print("\tTypeCalib=%d\n", gSensorCfg.entry[i].typeCalib);
        _print("\tIndexOfTypeCalib=%u\n", gSensorCfg.entry[i].indexOfTypeCalib);
        _print("\tTypeErr=%d\n", gSensorCfg.entry[i].typeErr);
        _print("\tIndexOfTypeError=%u\n", gSensorCfg.entry[i].indexOfTypeErr);
        _print("\tTypeStatus=%d\n", gSensorCfg.entry[i].typeStatus);
        _print("\tSuccessValueAnd=%u\n", gSensorCfg.entry[i].goodValueAND);
        _print("\tSuccessValueCompare=%u\n", gSensorCfg.entry[i].goodValueCompare);
        _print("\tCalibValueAnd=%u\n", gSensorCfg.entry[i].calibValueAND);
        _print("\tCalibValueCompare=%u\n", gSensorCfg.entry[i].calibValueCompare);
        _print("\tErrorValueAnd=%u\n", gSensorCfg.entry[i].errorValueAND);
        _print("\tErrorValueCompare=%u\n", gSensorCfg.entry[i].errorValueCompare);
    }
    _print("\n");

    _print("[MODBUSRTU TAG]\n");
    _print("TotalTag=%u\n", gMbrtuCfg.numTag);
    for (int i = 0; i < gMbrtuCfg.numTag; i++) {
        _print("\t[Tag%d]\n", i);
        _print("\tSlaveAddress=%u\n", gMbrtuCfg.entry[i].slaveAddress);
        _print("\tFunction=%u\n", gMbrtuCfg.entry[i].function);
        _print("\tRegister=%u\n", gMbrtuCfg.entry[i].regAddress);
        _print("\tQuantity=%u\n", gMbrtuCfg.entry[i].quantity);
        _print("\tDataType=%u\n", gMbrtuCfg.entry[i].rawDataType);
        _print("\tBigEndian=%d\n", gMbrtuCfg.entry[i].bigEndian);
        _print("\tEnable=%d\n", gMbrtuCfg.entry[i].enable);
        _print("\tName=%s\n", gMbrtuCfg.entry[i].name);
        _print("\tUnit=%s\n", gMbrtuCfg.entry[i].unit);
        _print("\tScaleType=%u\n", gMbrtuCfg.entry[i].scaleType);
        _print("\tScaledDataType=%u\n", gMbrtuCfg.entry[i].scaleDataType);
        _print("\tScaleValue=%.3f\n", gMbrtuCfg.entry[i].scaleValue);
        _print("\tADCType=%u\n", gMbrtuCfg.entry[i].adcType);
        _print("\tADCLow=%.3f\n", gMbrtuCfg.entry[i].adcLow);
        _print("\tADCHigh=%.3f\n", gMbrtuCfg.entry[i].adcHigh);
        _print("\tOffsetPreValue=%.3f\n", gMbrtuCfg.entry[i].offsetPreVal);
        _print("\tOffsetSubValue=%.3f\n", gMbrtuCfg.entry[i].offsetSubVal);
        _print("\tOffsetPreOperator=%u\n", gMbrtuCfg.entry[i].offSetPreOperator);
        _print("\tOffsetSubOperator=%u\n", gMbrtuCfg.entry[i].offsetSubOperator);
    }
    _print("\n");

    _print("[ANALOG]\n");
    for (int i = 0; i < MAX_ANALOG_CHANNEL; i++) {
        _print("\t[Channel%d]\n", i);
        _print("\tName=%s\n", gAnalogCfg.entry[i].name);
        _print("\tUnit=%s\n", gAnalogCfg.entry[i].unit);
        _print("\tEnable=%d\n", gAnalogCfg.entry[i].enable);
        _print("\tScaleType=%u\n", gAnalogCfg.entry[i].scaleType);
        _print("\tScaledDataType=%u\n", gAnalogCfg.entry[i].scaleDataType);
        _print("\tScaleValue=%.3f\n", gAnalogCfg.entry[i].scaleValue);
        _print("\tADCType=%u\n", gAnalogCfg.entry[i].adcType);
        _print("\tADCLow=%.3f\n", gAnalogCfg.entry[i].adcLow);
        _print("\tADCHigh=%.3f\n", gAnalogCfg.entry[i].adcHigh);
        _print("\tOffsetPreValue=%.3f\n", gAnalogCfg.entry[i].offsetPreVal);
        _print("\tOffsetSubValue=%.3f\n", gAnalogCfg.entry[i].offsetSubVal);
        _print("\tOffsetPreOperator=%u\n", gAnalogCfg.entry[i].offSetPreOperator);
        _print("\tOffsetSubOperator=%u\n", gAnalogCfg.entry[i].offsetSubOperator);
    }
    _print("\n");

    _print("[INPUT_CAPTURE]\n");
    for (int i = 0; i < MAX_INPUT_CAPTURE; i++) {
        _print("\t[Channel%d]\n", i);
        _print("\tName=%s\n", gInCaptureCfg.entry[i].name);
        _print("\tUnit=%s\n", gInCaptureCfg.entry[i].unit);
        _print("\tEnable=%d\n", gInCaptureCfg.entry[i].enable);
        _print("\tValueOfPulse=%.4f\n", (double) gInCaptureCfg.entry[i].valPerPulse);
        _print("\tScale=%.4f\n", (double) gInCaptureCfg.entry[i].scale);
        _print("\tMinFreq=%.2f\n", gInCaptureCfg.entry[i].minFreq);
    }
    _print("\n");


    _flush();

    if (!_setedHidden) {
        _setedHidden = true;
        SDcard_SetHidden(LOG_CONFIG_DIRECTORY);
    }
}