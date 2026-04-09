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

    //    _print("[Metadata]\n");
    //    _print("CommitID=%lu\n", commitId);
    //    _print("ParentID=%lu\n", parrentId);
    //    _print("Timestamp=%04u-%02u-%02u %02u:%02u:%02u\n",
    //            RTC_Dt.sysTime.year, RTC_Dt.sysTime.month, RTC_Dt.sysTime.day, RTC_Dt.sysTime.hour, RTC_Dt.sysTime.minute, RTC_Dt.sysTime.second);
    //    _print("Author=%s\n", _getAuthorString(author));
    //    _print("\n");
    //
    //    _print("[APP - Network]\n");
    //    _print("DHCPEnable=%d\n", glbAppCfg.network.isDHCPEn);
    //    _print("NetBIOSName=%s\n", glbAppCfg.network.NetBIOSName);
    //    _print("AppUserDevice=%s\n", glbAppCfg.network.app_username_device);
    //    _print("AppPassDevice=%s\n", glbAppCfg.network.app_password_device);
    //    _print("IPAddr=%u.%u.%u.%u\n", glbAppCfg.network.ipAddr.v[0], glbAppCfg.network.ipAddr.v[1], glbAppCfg.network.ipAddr.v[2], glbAppCfg.network.ipAddr.v[3]);
    //    _print("\n");
    //
    //    _print("[APP - Log file]\n");
    //    _print("UpLink=%d\n", glbAppCfg.ftpGeneral.uplink);
    //    _print("FormatData=%d\n", glbAppCfg.ftpGeneral.formatData);
    //    _print("FileType=%d\n", glbAppCfg.ftpGeneral.typefile);
    //    _print("TimeMode=%d\n", glbAppCfg.ftpGeneral.timeMode);
    //    _print("SendInterval=%u\n", glbAppCfg.ftpGeneral.SendInterval);
    //    _print("\n");
    //
    //    for (int i = 0; i < NUM_FTP_SERVER; i++) {
    //        _print("[APP - FTPServer%d]\n", i);
    //        _print("Enable=%d\n", glbAppCfg.ftpServer[i].enable);
    //        _print("Hostname=%s\n", glbAppCfg.ftpServer[i].hostname);
    //        _print("Port=%u\n", glbAppCfg.ftpServer[i].port);
    //        _print("Path=%s\n", glbAppCfg.ftpServer[i].path);
    //        _print("Username=%s\n", glbAppCfg.ftpServer[i].username);
    //        _print("Password=%s\n", glbAppCfg.ftpServer[i].password);
    //        _print("NamePrefix=%s\n", glbAppCfg.ftpServer[i].namePrefix);
    //        _print("MakeFolderType=%d\n", glbAppCfg.ftpServer[i].makeFolder);
    //        _print("\n");
    //    }
    //
    //    _print("[APP - ModbusRTU Phy]\n");
    //    _print("BaudRate=%lu\n", glbAppCfg.modbusRTU.baudRate);
    //    _print("Parity=%u\n", glbAppCfg.modbusRTU.parity);
    //    _print("Stopbit=%u\n", glbAppCfg.modbusRTU.stopbits);
    //    _print("Timeout=%u\n", glbAppCfg.modbusRTU.timeout);
    //    _print("PollInterval=%u\n", glbAppCfg.modbusRTU.pollInterval);
    //    _print("Retries=%u\n", glbAppCfg.modbusRTU.retries);
    //    _print("Latency=%u\n", glbAppCfg.modbusRTU.latency);
    //    _print("\n");
    //
    //    _print("[APP - GSM]\n");
    //    _print("APN=%s\n", glbAppCfg.GSM.APN);
    //    _print("UserAPN=%s\n", glbAppCfg.GSM.usernameAPN);
    //    _print("PassAPN=%s\n", glbAppCfg.GSM.passAPN);
    //    _print("Mode=%d\n", glbAppCfg.GSM.mode);
    //    _print("\n");
    //
    //    _print("[APP - Time]\n");
    //    _print("Timezone=%s\n", glbAppCfg.time.Timezone);
    //    _print("NTPIndex=%u\n", glbAppCfg.time.indexNTP);
    //    _print("TimeAuto=%u\n", glbAppCfg.time.Time_auto);
    //    _print("\n");
    //
    //    _print("[APP - Input]\n");
    //    _print("IN1Describe=%s\n", glbAppCfg.io.describeIN1);
    //    _print("IN2Describe=%s\n", glbAppCfg.io.describeIN2);
    //    _print("IN3Describe=%s\n", glbAppCfg.io.describeIN3);
    //    _print("IN4Describe=%s\n", glbAppCfg.io.describeIN4);
    //    _print("IN5Describe=%s\n", glbAppCfg.io.describeIN5);
    //    _print("IN6Describe=%s\n", glbAppCfg.io.describeIN6);
    //    _print("IN7Describe=%s\n", glbAppCfg.io.describeIN7);
    //    _print("IN8Describe=%s\n", glbAppCfg.io.describeIN8);
    //    _print("IN9Describe=%s\n", glbAppCfg.io.describeIN9);
    //    _print("IN10Describe=%s\n", glbAppCfg.io.describeIN10);
    //    _print("IN11Describe=%s\n", glbAppCfg.io.describeIN11);
    //    _print("IN12Describe=%s\n", glbAppCfg.io.describeIN12);
    //    _print("IN13Describe=%s\n", glbAppCfg.io.describeIN13);
    //    _print("IN14Describe=%s\n", glbAppCfg.io.describeIN14);
    //    _print("IN15Describe=%s\n", glbAppCfg.io.describeIN15);
    //    _print("IN16Describe=%s\n", glbAppCfg.io.describeIN16);
    //    _print("\n");
    //
    //    _print("[APP - Output]\n");
    //    _print("OUT1Describe=%s\n", glbAppCfg.io.describeOUT1);
    //    _print("OUT1Type=%d\n", glbAppCfg.io.typeCtrlOut1);
    //    _print("OUT1Time=%u\n", glbAppCfg.io.timeCtrlOut1);
    //
    //    _print("OUT2Describe=%s\n", glbAppCfg.io.describeOUT2);
    //    _print("OUT2Type=%d\n", glbAppCfg.io.typeCtrlOut2);
    //    _print("OUT2Time=%u\n", glbAppCfg.io.timeCtrlOut2);
    //    _print("\n");
    //
    //    _print("[APP - Get sample API]\n");
    //    _print("WhichOut=%u\n", glbAppCfg.sampleApi.whichOut);
    //    _print("MaxBottle=%u\n", glbAppCfg.sampleApi.maxBottle);
    //    _print("BottleIdx=%u\n", glbAppCfg.sampleApi.bottleIdx);
    //    _print("SampleTime=%u\n", glbAppCfg.sampleApi.sampleTime);
    //    _print("RemoteHost=%s\n", glbAppCfg.sampleApi.remoteHost);
    //    _print("RemotePort=%u\n", glbAppCfg.sampleApi.remotePort);
    //    _print("RemotePath=%s\n", glbAppCfg.sampleApi.remotePath);
    //    _print("LocalPort=%u\n", glbAppCfg.sampleApi.localPort);
    //    _print("LocalPath=%s\n", glbAppCfg.sampleApi.localPath);
    //    _print("\n");
    //
    //    _print("[APP - SDcard]\n");
    //    _print("TimeRemove=%u\n", glbAppCfg.sdCard.timeRemove);
    //    _print("\n");
    //
    //    _print("[APP - HMI Tag]\n");
    //    //    _print("Backlight_Strength=%u\n", glbAppCfg.strengthBackLight);
    //    for (int i = 0; i < MAX_HMI_PARA; i++) {
    //        _print("TagHMI%d=%u\n", i, glbAppCfg.tag_hmi[i]);
    //    }
    //    _print("\n");
    //
    //    _print("[APP - ModbusTCP position]\n");
    //    for (int i = 0; i < MAX_POSITION_SIZE; i++) {
    //        _print("Position%d=%u\n", i, glbAppCfg.position[i]);
    //    }
    //    _print("\n");
    //
    //    //    _print("[APP - User management]\n");
    //    //    _print("TotalUser=%u\n", glbAppCfg.user.num);
    //    //    for (int i = 0; i < glbAppCfg.user.num; i++) {
    //    //        _print("Username%u=%s\n", i, glbAppCfg.user.entry[i].username);
    //    //        _print("Permission%u=%d\n", i, glbAppCfg.user.entry[i].permission);
    //    //    }
    //    //    _print("\n");
    //
    //    _print("[SENSOR]\n");
    //    _print("TotalSensor=%u\n", glbAppCfg.sensor.total_sensor);
    //    for (int i = 0; i < glbAppCfg.sensor.total_sensor; i++) {
    //        _print("\t[Entry%d]\n", i);
    //        _print("\tEnable=%d\n", glbAppCfg.sensor.entry[i].enable);
    //        _print("\tType=%d\n", glbAppCfg.sensor.entry[i].type);
    //        _print("\tIndexOfType=%u\n", glbAppCfg.sensor.entry[i].idxInType);
    //        _print("\tCalibrated=%d\n", glbAppCfg.sensor.entry[i].calibrated);
    //        _print("\tTypeSuccess=%d\n", glbAppCfg.sensor.entry[i].typeRun);
    //        _print("\tIndexOfTypeSuccess=%u\n", glbAppCfg.sensor.entry[i].idxInTypeRun);
    //        _print("\tTypeCalib=%d\n", glbAppCfg.sensor.entry[i].typeCalib);
    //        _print("\tIndexOfTypeCalib=%u\n", glbAppCfg.sensor.entry[i].idxInTypeCalib);
    //        _print("\tTypeErr=%d\n", glbAppCfg.sensor.entry[i].typeErr);
    //        _print("\tIndexOfTypeError=%u\n", glbAppCfg.sensor.entry[i].idxInTypeErr);
    //        _print("\tTypeStatus=%d\n", glbAppCfg.sensor.entry[i].typeStatus);
    //        _print("\tSuccessValueAnd=%u\n", glbAppCfg.sensor.entry[i].runvalueAND);
    //        _print("\tSuccessValueCompare=%u\n", glbAppCfg.sensor.entry[i].runvalueCompare);
    //        _print("\tCalibValueAnd=%u\n", glbAppCfg.sensor.entry[i].calibvalueAND);
    //        _print("\tCalibValueCompare=%u\n", glbAppCfg.sensor.entry[i].calibvalueCompare);
    //        _print("\tErrorValueAnd=%u\n", glbAppCfg.sensor.entry[i].errorvalueAND);
    //        _print("\tErrorValueCompare=%u\n", glbAppCfg.sensor.entry[i].errorvalueCompare);
    //    }
    //    _print("\n");
    //
    //    _print("[MODBUSRTU TAG]\n");
    //    _print("TotalTag=%u\n", glbAppRtu.total_row);
    //    for (int i = 0; i < glbAppRtu.total_row; i++) {
    //        _print("\t[Tag%d]\n", i);
    //        _print("\tSlaveAddress=%u\n", glbAppRtu.app_rtu_table[i].addr);
    //        _print("\tFunction=%u\n", glbAppRtu.app_rtu_table[i].func);
    //        _print("\tRegister=%u\n", glbAppRtu.app_rtu_table[i].addr_reg);
    //        _print("\tQuantity=%u\n", glbAppRtu.app_rtu_table[i].bytes);
    //        _print("\tDataType=%u\n", glbAppRtu.app_rtu_table[i].type);
    //        _print("\tBigEndian=%d\n", glbAppRtu.app_rtu_table[i].big_endian);
    //        _print("\tEnable=%d\n", glbAppRtu.app_rtu_table[i].enable);
    //        _print("\tName=%s\n", glbAppRtu.analog_modbus[i].des);
    //        _print("\tUnit=%s\n", glbAppRtu.analog_modbus[i].unit);
    //        _print("\tScaleType=%u\n", glbAppRtu.analog_modbus[i].scale_type);
    //        _print("\tScaledDataType=%u\n", glbAppRtu.analog_modbus[i].scaled_data_type);
    //        _print("\tScaleValue=%.3f\n", glbAppRtu.analog_modbus[i].scale_value);
    //        _print("\tADCType=%u\n", glbAppRtu.analog_modbus[i].ADCtype);
    //        _print("\tADCLow=%.3f\n", glbAppRtu.analog_modbus[i].ADClow);
    //        _print("\tADCHigh=%.3f\n", glbAppRtu.analog_modbus[i].ADChigh);
    //        _print("\tOffsetPreValue=%.3f\n", glbAppRtu.analog_modbus[i].ADCofset_pre);
    //        _print("\tOffsetSubValue=%.3f\n", glbAppRtu.analog_modbus[i].ADCofset_sub);
    //        _print("\tOffsetPreOperator=%u\n", glbAppRtu.analog_modbus[i].ADCtypepre);
    //        _print("\tOffsetSubOperator=%u\n", glbAppRtu.analog_modbus[i].ADCtypesub);
    //    }
    //    _print("\n");
    //
    //    _print("[ANALOG]\n");
    //    for (int i = 0; i < MAX_ANALOG_CHANNEL; i++) {
    //        _print("\t[Channel%d]\n", i);
    //        _print("\tName=%s\n", glbAppAnlg.entry[i].des);
    //        _print("\tUnit=%s\n", glbAppAnlg.entry[i].unit);
    //        _print("\tEnable=%d\n", glbAppAnlg.entry[i].enable);
    //        _print("\tScaleType=%u\n", glbAppAnlg.entry[i].scale_type);
    //        _print("\tScaledDataType=%u\n", glbAppAnlg.entry[i].scaled_data_type);
    //        _print("\tScaleValue=%.3f\n", glbAppAnlg.entry[i].scale_value);
    //        _print("\tADCType=%u\n", glbAppAnlg.entry[i].ADCtype);
    //        _print("\tADCLow=%.3f\n", glbAppAnlg.entry[i].ADClow);
    //        _print("\tADCHigh=%.3f\n", glbAppAnlg.entry[i].ADChigh);
    //        _print("\tOffsetPreValue=%.3f\n", glbAppAnlg.entry[i].ADCofset_pre);
    //        _print("\tOffsetSubValue=%.3f\n", glbAppAnlg.entry[i].ADCofset_sub);
    //        _print("\tOffsetPreOperator=%u\n", glbAppAnlg.entry[i].ADCtypepre);
    //        _print("\tOffsetSubOperator=%u\n", glbAppAnlg.entry[i].ADCtypesub);
    //    }
    //    _print("\n");
    //
    //    _print("[COUNTER]\n");
    //    for (int i = 0; i < (MAX_COUNTER * 2); i++) {
    //        _print("\t[Counter%d]\n", i);
    //        _print("\tName=%s\n", glbAppCnter.counter[i].name);
    //        _print("\tUnit=%s\n", glbAppCnter.counter[i].unit);
    //        _print("\tEnable=%d\n", glbAppCnter.counter[i].enable);
    //        _print("\tValueOfPulse=%.4f\n", (double) glbAppCnter.counter[i].pulse);
    //        _print("\tScale=%.4f\n", (double) glbAppCnter.counter[i].scale);
    //        _print("\tMinFreq=%.2f\n", glbAppCnter.counter[i].minFreq);
    //    }
    //    _print("\n");
    //
    //    _print("[EXTEND]\n");
    //    _print("Enable=%d\n", glbExtend.enable);
    //    _print("Address=%u\n", glbExtend.address);
    //    for (int i = 0; i < EXTEND_MAX_POSITION_SIZE; i++) {
    //        _print("Position%d=%u\n", i, glbExtend.position[i]);
    //    }
    //    for (int i = 0; i < EXTEND_MAX_INPUT_OUTPUT; i++) {
    //        _print("\t[IO%d]\n", i);
    //        _print("\tName=%s\n", glbExtend.inputOutput[i].des);
    //        _print("\tMode=%d\n", glbExtend.inputOutput[i].mode);
    //    }
    //    for (int i = 0; i < EXTEND_MAX_INPUT_CAPTURE; i++) {
    //        _print("\t[CAPTURE%d]\n", i);
    //        _print("\tName=%s\n", glbExtend.inputCapture[i].des);
    //        _print("\tMode=%d\n", glbExtend.inputCapture[i].mode);
    //
    //        _print("\tPulseRateName=%s\n", glbExtend.inputCapture[i].pulseRate.name);
    //        _print("\tPulseRateUnit=%s\n", glbExtend.inputCapture[i].pulseRate.unit);
    //        _print("\tPulseRateValuePerPulse=%.4f\n", glbExtend.inputCapture[i].pulseRate.pulse);
    //        _print("\tPulseRateMinFreq=%.2f\n", glbExtend.inputCapture[i].pulseRate.minFreq);
    //        _print("\tPulseRateEnable=%d\n", glbExtend.inputCapture[i].pulseRate.enable);
    //        _print("\tPulseRateScale=%.4f\n", glbExtend.inputCapture[i].pulseRate.scale);
    //
    //        _print("\tCounterName=%s\n", glbExtend.inputCapture[i].counter.name);
    //        _print("\tCounterUnit=%s\n", glbExtend.inputCapture[i].counter.unit);
    //        _print("\tCounterValuePerPulse=%.4f\n", glbExtend.inputCapture[i].counter.pulse);
    //        _print("\tCounterEnable=%d\n", glbExtend.inputCapture[i].counter.enable);
    //    }
    //    _print("\n");

    _flush();

    if (!_setedHidden) {
        _setedHidden = true;
        SDcard_SetHidden(LOG_CONFIG_DIRECTORY);
    }
}