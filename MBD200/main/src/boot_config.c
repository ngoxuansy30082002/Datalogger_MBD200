#include "boot_config.h"

APP_CONFIG gAppCfg;
ANALOG gAnalogCfg = {0};
MODBUS_RTU_TAG gMbrtuCfg = {0};
INPUT_CAPTURE gInCaptureCfg = {0};

//APP_ANALOG glbAppAnlg;
//APP_CONFIG_DEVICE glbAppDev;
////APP_CONFIG_ASSETS glbAppAsset;
//APP_RTU_TAG glbAppRtu;
//APP_COUNTER glbAppCnter;
//GET_SAMPLE glbGetSample;
//EXTEND_CONFIG glbExtend;
//
//static APP_CONFIG _appConfig;
//static APP_ANALOG _appAnalog;
//static APP_CONFIG_DEVICE _appConfigDevice;
////static APP_CONFIG_ASSETS _appConfigAssets;
//static APP_RTU_TAG _appRtuTags;
//static APP_COUNTER _appCounter;
//static GET_SAMPLE _getSample;
//static EXTEND_CONFIG _extend;
//
//static BOOT_CONFIG_DATA bootCfg_Dt;
//NVM_VALIDATION_STRUCT NVMValidationStruct;
//unsigned short wOriginalglbAppCfgChecksum; // Checksum of the ROM defaults for glbAppCfg
//static DRV_HANDLE driver;
//static uint32_t _lockSaveFlash = 0;
//static uint8_t _firstSave = 7;
//static int8_t _numFail = 3;
//
//static bool isLockSaveFlash();
//static bool _WaitForFlashTransfer(uint32_t timeoutMs);
//
//void BOOT_CONFIG_Initialize() {
//    RESET5_EP_OutputEnable();
//    SS5_EP_OutputEnable();
//
//    RESET5_EP_Set();
//    CONFIG_InputEnable();
//
//    bootCfg_Dt.state = BOOT_CONFIG_INIT;
//    bootCfg_Dt.driverHandle = driver;
//    bootCfg_Dt.proactiveSaveFlag = false;
//    bootCfg_Dt.currentAddrFlash = 0;
//
//    memset((void *) &_appConfig, 0x00, sizeof (_appConfig));
//    memset((void *) &_appConfigDevice, 0x00, sizeof (_appConfigDevice));
//    //    memset((void *) &_appConfigAssets, 0x00, sizeof (_appConfigAssets));
//    memset((void *) &_appRtuTags, 0x00, sizeof (_appRtuTags));
//    memset((void *) &_appCounter, 0x00, sizeof (_appCounter));
//    memset((void *) &_appAnalog, 0x00, sizeof (_appAnalog));
//    memset((void *) &_extend, 0x00, sizeof (_extend));
//
//    memset((void *) &glbAppCfg, 0x00, sizeof (glbAppCfg));
//    memset((void *) &glbAppAnlg, 0x00, sizeof (glbAppAnlg));
//    memset((void *) &glbAppDev, 0x00, sizeof (glbAppDev));
//    //    memset((void *) &glbAppAsset, 0x00, sizeof (glbAppAsset));
//    memset((void *) &glbAppRtu, 0x00, sizeof (glbAppRtu));
//    memset((void *) &glbAppCnter, 0x00, sizeof (glbAppCnter));
//    memset((void *) &glbExtend, 0x00, sizeof (glbExtend));
//
//
//    // _________ config network ___________
//    glbAppCfg.network.isDHCPEn = true;
//    TCPIP_Helper_StringToIPAddress(TCPIP_NETWORK_DEFAULT_IP_ADDRESS_IDX0, &glbAppCfg.network.ipAddr);
//    TCPIP_Helper_StringToIPAddress(TCPIP_NETWORK_DEFAULT_IP_ADDRESS_IDX0, &glbAppCfg.network.defaultIpAddr);
//    TCPIP_Helper_StringToIPAddress(TCPIP_NETWORK_DEFAULT_IP_MASK_IDX0, &glbAppCfg.network.ipMask);
//    TCPIP_Helper_StringToIPAddress(TCPIP_NETWORK_DEFAULT_IP_MASK_IDX0, &glbAppCfg.network.defaultIpMask);
//    TCPIP_Helper_StringToIPAddress(TCPIP_NETWORK_DEFAULT_GATEWAY_IDX0, &glbAppCfg.network.gateway);
//    TCPIP_Helper_StringToIPAddress(TCPIP_NETWORK_DEFAULT_DNS_IDX0, &glbAppCfg.network.priDNS);
//    TCPIP_Helper_StringToIPAddress(TCPIP_NETWORK_DEFAULT_SECOND_DNS_IDX0, &glbAppCfg.network.secondDNS);
//    snprintf(glbAppCfg.network.NetBIOSName, sizeof (glbAppCfg.network.NetBIOSName), "%s", SERIAL);
//
//    snprintf(glbAppCfg.network.app_username_device, sizeof (glbAppCfg.network.app_username_device), "%s", DEFAULT_USERNAME_DEVICE); // user name device
//    snprintf(glbAppCfg.network.app_password_device, sizeof (glbAppCfg.network.app_password_device), "%s", DEFAULT_PASSWORD_DEVICE); // password name device
//    // _________ config network ___________
//
//    // _________ config modbusRTU ___________
//    glbAppCfg.modbusRTU.baudRate = MBRTU_BAUD_RATE; // baud modbus
//    glbAppCfg.modbusRTU.timeout = MBRTU_TIMEOUT; // time timeout
//    glbAppCfg.modbusRTU.retries = MBRTU_RETRIES; // retries
//    glbAppCfg.modbusRTU.pollInterval = MBRTU_POLL_INTERVAL; // poll interval
//    glbAppCfg.modbusRTU.stopbits = MBRTU_STOP_BITS;
//    glbAppCfg.modbusRTU.parity = MBRTU_PARITY;
//    glbAppCfg.modbusRTU.latency = 1000;
//    glbAppRtu.total_row = 0; // total row of table that it maded by user
//
//    for (uint8_t i = 0; i < MAX_BUFFER_TAG; i++) {
//
//        glbAppRtu.app_rtu_table[i].addr = 0;
//        glbAppRtu.app_rtu_table[i].func = 0;
//        glbAppRtu.app_rtu_table[i].addr_reg = 0;
//        glbAppRtu.app_rtu_table[i].bytes = 0;
//        glbAppRtu.app_rtu_table[i].type = 1;
//        glbAppRtu.app_rtu_table[i].big_endian = 1;
//
//        // scale
//        glbAppRtu.analog_modbus[i].scaled_data_type = 0;
//        glbAppRtu.analog_modbus[i].scale_type = 0;
//        glbAppRtu.analog_modbus[i].scale_value = 1;
//
//        // ADC
//        glbAppRtu.analog_modbus[i].ADCtype = 0;
//        glbAppRtu.analog_modbus[i].ADChigh = 0;
//        glbAppRtu.analog_modbus[i].ADClow = 0;
//        glbAppRtu.analog_modbus[i].ADCofset_sub = 0;
//        glbAppRtu.analog_modbus[i].ADCofset_pre = 0;
//        glbAppRtu.analog_modbus[i].ADCtypesub = 0;
//        glbAppRtu.analog_modbus[i].ADCtypepre = 0;
//
//        snprintf(glbAppRtu.analog_modbus[i].unit, sizeof (glbAppRtu.analog_modbus[i].unit), "Unused");
//        snprintf(glbAppRtu.analog_modbus[i].des, sizeof (glbAppRtu.analog_modbus[i].des), "Unused");
//    }
//    // _________ config modbusRTU ___________
//
//    // _________ config device descript ___________
//    snprintf(glbAppDev.manufacturer, sizeof (glbAppDev.manufacturer), "%s", MANUFACTURER);
//    snprintf(glbAppDev.fw_code, sizeof (glbAppDev.fw_code), "%s", FW_CODE);
//    snprintf(glbAppDev.hw_code, sizeof (glbAppDev.hw_code), "%s", HW_CODE);
//    snprintf(glbAppDev.date, sizeof (glbAppDev.date), "%s", DATE);
//    snprintf(glbAppDev.model, sizeof (glbAppDev.model), "%s", MODEL);
//    snprintf(glbAppDev.serial, sizeof (glbAppDev.serial), "%s", SERIAL);
//    snprintf(glbAppDev.describe_device, sizeof (glbAppDev.describe_device), "%s", DESCRIBE_DEVICE);
//    // _________ config device descript ___________
//
//    // _________ config analog ___________
//    for (uint8_t i = 0; i < MAX_ANALOG_CHANNEL; i++) {
//        // scale
//        glbAppAnlg.entry[i].scaled_data_type = 1;
//        glbAppAnlg.entry[i].scale_type = 1;
//        glbAppAnlg.entry[i].scale_value = 1;
//        glbAppAnlg.entry[i].enable = 0;
//
//        // ADC
//        if (i < 4) {
//            glbAppAnlg.entry[i].ADCtype = 2;
//        } else {
//            glbAppAnlg.entry[i].ADCtype = 3;
//        }
//        glbAppAnlg.entry[i].ADChigh = 0;
//        glbAppAnlg.entry[i].ADClow = 0;
//        glbAppAnlg.entry[i].ADCofset_sub = 0;
//        glbAppAnlg.entry[i].ADCofset_pre = 0;
//        glbAppAnlg.entry[i].ADCtypesub = 0;
//        glbAppAnlg.entry[i].ADCtypepre = 0;
//
//        snprintf(glbAppAnlg.entry[i].unit, sizeof (glbAppAnlg.entry[i].unit), "Unused");
//        snprintf(glbAppAnlg.entry[i].des, sizeof (glbAppAnlg.entry[i].des), "Unused");
//    }
//    // _________ config analog ___________
//
//    // _________ config FTP ___________
//    for (uint8_t i = 0; i < NUM_FTP_SERVER; i++) {
//        snprintf(glbAppCfg.ftpServer[i].username, sizeof (glbAppCfg.ftpServer[i].username), "%s", FTP_USER);
//        snprintf(glbAppCfg.ftpServer[i].password, sizeof (glbAppCfg.ftpServer[i].password), "%s", FTP_PASS);
//        snprintf(glbAppCfg.ftpServer[i].path, sizeof (glbAppCfg.ftpServer[i].path), "%s", FTP_PATH);
//        snprintf(glbAppCfg.ftpServer[i].hostname, sizeof (glbAppCfg.ftpServer[i].hostname), "%s", FTP_HOST);
//        glbAppCfg.ftpServer[i].port = FTP_PORT;
//        glbAppCfg.ftpServer[i].makeFolder = NONE_FOLDER;
//        snprintf(glbAppCfg.ftpServer[i].namePrefix, sizeof (glbAppCfg.ftpServer[i].namePrefix), "%s", "");
//        glbAppCfg.ftpServer[i].enable = false;
//    }
//
//    glbAppCfg.ftpGeneral.uplink = GSM; // default FTP uplink is GSM
//    glbAppCfg.ftpGeneral.typefile = TXT; // default format file is txt
//    glbAppCfg.ftpGeneral.formatData = TT24;
//    glbAppCfg.ftpGeneral.timeMode = OCLOCK;
//    glbAppCfg.ftpGeneral.SendInterval = 2;
//    // _________ config FTP ___________
//
//
//    // _________ config GSM ___________
//    snprintf(glbAppCfg.GSM.APN, sizeof (glbAppCfg.GSM.APN), "%s", MY_APN);
//    snprintf(glbAppCfg.GSM.usernameAPN, sizeof (glbAppCfg.GSM.usernameAPN), "%s", USERNAME_APN);
//    snprintf(glbAppCfg.GSM.passAPN, sizeof (glbAppCfg.GSM.passAPN), "%s", PASSWORD_APN);
//    glbAppCfg.GSM.mode = GSM_SIM1;
//    // _________ config GSM ___________
//
//
//    // _________ config counter ___________
//    for (uint8_t i = 0; i < MAX_COUNTER * 2; i++) {
//        snprintf(glbAppCnter.counter[i].name, sizeof (glbAppCnter.counter[i].name), "TotalFlow%u", i);
//        snprintf(glbAppCnter.counter[i].unit, sizeof (glbAppCnter.counter[i].unit), "%s", "m3");
//        glbAppCnter.counter[i].enable = 0;
//        glbAppCnter.counter[i].scale = 3600;
//        glbAppCnter.counter[i].pulse = 1;
//        glbAppCnter.counter[i].minFreq = 1;
//    }
//    // _________ config counter ___________
//
//    // _________ config modbusTCP ___________
//    for (uint8_t i = 0; i < MAX_POSITION_SIZE; i++) {
//        glbAppCfg.position[i] = 1;
//    }
//    // _________ config modbusTCP ___________
//
//    // _________ config IO ___________
//    snprintf(glbAppCfg.io.describeIN1, sizeof (glbAppCfg.io.describeIN1), "Description");
//    snprintf(glbAppCfg.io.describeIN2, sizeof (glbAppCfg.io.describeIN2), "Description");
//    snprintf(glbAppCfg.io.describeIN3, sizeof (glbAppCfg.io.describeIN3), "Description");
//    snprintf(glbAppCfg.io.describeIN4, sizeof (glbAppCfg.io.describeIN4), "Description");
//    snprintf(glbAppCfg.io.describeIN5, sizeof (glbAppCfg.io.describeIN5), "Description");
//    snprintf(glbAppCfg.io.describeIN6, sizeof (glbAppCfg.io.describeIN6), "Description");
//    snprintf(glbAppCfg.io.describeIN7, sizeof (glbAppCfg.io.describeIN7), "Description");
//    snprintf(glbAppCfg.io.describeIN8, sizeof (glbAppCfg.io.describeIN8), "Description");
//    snprintf(glbAppCfg.io.describeIN9, sizeof (glbAppCfg.io.describeIN9), "Description");
//    snprintf(glbAppCfg.io.describeIN10, sizeof (glbAppCfg.io.describeIN10), "Description");
//    snprintf(glbAppCfg.io.describeIN11, sizeof (glbAppCfg.io.describeIN11), "Description");
//    snprintf(glbAppCfg.io.describeIN12, sizeof (glbAppCfg.io.describeIN12), "Description");
//    snprintf(glbAppCfg.io.describeIN13, sizeof (glbAppCfg.io.describeIN13), "Description");
//    snprintf(glbAppCfg.io.describeIN14, sizeof (glbAppCfg.io.describeIN14), "Description");
//    snprintf(glbAppCfg.io.describeIN15, sizeof (glbAppCfg.io.describeIN15), "Description");
//    snprintf(glbAppCfg.io.describeIN16, sizeof (glbAppCfg.io.describeIN16), "Description");
//
//    snprintf(glbAppCfg.io.describeOUT1, sizeof (glbAppCfg.io.describeOUT1), "Description");
//    snprintf(glbAppCfg.io.describeOUT2, sizeof (glbAppCfg.io.describeOUT2), "Description");
//
//    glbAppCfg.io.timeCtrlOut1 = 0;
//    glbAppCfg.io.timeCtrlOut2 = 0;
//    glbAppCfg.io.typeCtrlOut1 = HOLD;
//    glbAppCfg.io.typeCtrlOut2 = HOLD;
//    // _________ config IO ___________
//
//    // _________ config get sample ___________
//    glbAppCfg.sampleApi.whichOut = 1;
//    glbAppCfg.sampleApi.maxBottle = 2;
//    snprintf(glbAppCfg.sampleApi.remotePath, sizeof (glbAppCfg.sampleApi.remotePath), "/");
//    snprintf(glbAppCfg.sampleApi.remoteHost, sizeof (glbAppCfg.sampleApi.remoteHost), "/");
//    glbAppCfg.sampleApi.remotePort = 80;
//    snprintf(glbAppCfg.sampleApi.localPath, sizeof (glbAppCfg.sampleApi.localPath), "/");
//    glbAppCfg.sampleApi.localPort = 400;
//    glbAppCfg.sampleApi.sampleTime = 1;
//    glbAppCfg.sampleApi.bottleIdx = 1;
//
//    // _________ config IO ___________
//
//    glbAppCfg.sdCard.timeRemove = SDCARD_TIME_REMOVE;
//
//    // _________ config Sensor ___________
//    glbAppCfg.sensor.total_sensor = 0;
//    for (uint8_t i = 0; i < MAX_SENSOR; i++) {
//        glbAppCfg.sensor.entry[i].type = SENSOR_NONE;
//        glbAppCfg.sensor.entry[i].calibrated = false;
//        glbAppCfg.sensor.entry[i].idxInType = 0;
//        glbAppCfg.sensor.entry[i].enable = false;
//        glbAppCfg.sensor.entry[i].typeStatus = FROM_NONE;
//        glbAppCfg.sensor.entry[i].typeRun = FROM_NONE;
//        glbAppCfg.sensor.entry[i].idxInTypeRun = 0;
//        glbAppCfg.sensor.entry[i].typeCalib = FROM_NONE;
//        glbAppCfg.sensor.entry[i].idxInTypeCalib = 0;
//        glbAppCfg.sensor.entry[i].typeErr = FROM_NONE;
//        glbAppCfg.sensor.entry[i].idxInTypeErr = 0;
//        glbAppCfg.sensor.entry[i].runvalueAND = 0;
//        glbAppCfg.sensor.entry[i].runvalueCompare = 0;
//        glbAppCfg.sensor.entry[i].calibvalueAND = 0;
//        glbAppCfg.sensor.entry[i].calibvalueCompare = 0;
//        glbAppCfg.sensor.entry[i].errorvalueAND = 0;
//        glbAppCfg.sensor.entry[i].errorvalueCompare = 0;
//    }
//    // _________ config Sensor ___________
//
//
//    glbAppCfg.time.Year_number = 20;
//    glbAppCfg.time.Time_auto = 0;
//    glbAppCfg.time.indexNTP = 0;
//    snprintf(glbAppCfg.time.Timezone, sizeof (glbAppCfg.time.Timezone), "%s", "7");
//
//    glbAppCfg.strengthBackLight = STRENGTH_BACK_LIGHT;
//    memset(glbAppCfg.tag_hmi, 30, MAX_HMI_PARA * sizeof (uint8_t));
//
//    /* app config extend module */
//    for (uint8_t i = 0; i < EXTEND_MAX_INPUT_OUTPUT; i++) {
//        snprintf(glbExtend.inputOutput[i].des, sizeof (glbExtend.inputOutput[i].des), "Description");
//        glbExtend.inputOutput[i].mode = EXTEND_IO_DISABLE;
//    }
//
//    for (uint8_t i = 0; i < EXTEND_MAX_INPUT_CAPTURE; i++) {
//        snprintf(glbExtend.inputCapture[i].des, sizeof (glbExtend.inputCapture[i].des), "Description");
//        glbExtend.inputCapture[i].mode = EXTEND_IO_DISABLE;
//
//        snprintf(glbExtend.inputCapture[i].pulseRate.name, sizeof (glbExtend.inputCapture[i].pulseRate.name), "Pulse rate %u", i + 1);
//        snprintf(glbExtend.inputCapture[i].pulseRate.unit, sizeof (glbExtend.inputCapture[i].pulseRate.unit), "Hz");
//        glbExtend.inputCapture[i].pulseRate.enable = false;
//        glbExtend.inputCapture[i].pulseRate.minFreq = 1.0;
//        glbExtend.inputCapture[i].pulseRate.pulse = 1.0;
//        glbExtend.inputCapture[i].pulseRate.scale = 1.0;
//
//        snprintf(glbExtend.inputCapture[i].counter.name, sizeof (glbExtend.inputCapture[i].counter.name), "Total counter %u", i + 1);
//        snprintf(glbExtend.inputCapture[i].counter.unit, sizeof (glbExtend.inputCapture[i].counter.unit), "Count");
//        glbExtend.inputCapture[i].counter.enable = false;
//        glbExtend.inputCapture[i].counter.pulse = 1.0;
//    }
//
//    for (uint8_t i = 0; i < EXTEND_MAX_POSITION_SIZE; i++) {
//        glbExtend.position[i] = 1;
//    }
//
//    wOriginalglbAppCfgChecksum = TCPIP_Helper_CalcIPChecksum((uint8_t *) & glbAppCfg, (uint16_t)sizeof (APP_CONFIG), 0);
//
//    //    SYS_CONSOLE_PRINT("BOOT-CONFIG: %d \n\r",sizeof (APP_CONFIG));
//}
//
//bool BOOT_CONFIG_Tasks(void) {
//    LEDDISP_Tasks();
//    static uint32_t _bootTick = 0;
//    static uint32_t _timeBlink = 0;
//
//    switch (bootCfg_Dt.state) {
//        case BOOT_CONFIG_INIT:
//            if (!CONFIG_Get()) bootCfg_Dt.state = BOOT_CONFIG_BTN_HOLD;
//            else bootCfg_Dt.state = BOOT_CONFIG_LOAD;
//            break;
//        case BOOT_CONFIG_BTN_HOLD:
//            if (SYS_TMR_TickCountGet() - _timeBlink >= SYS_TMR_TickCounterFrequencyGet() / 2ul) {
//                _timeBlink = SYS_TMR_TickCountGet();
//                ledDisp_Dt.buff[0] = 0;
//                ledDisp_Dt.buff[1] = 0;
//                ledDisp_Dt.buff[2] = ~ledDisp_Dt.buff[2];
//            }
//
//            if (SYS_TMR_TickCountGet() - _bootTick >= SYS_TMR_TickCounterFrequencyGet()*10) {
//                _bootTick = SYS_TMR_TickCountGet();
//                bootCfg_Dt.state = BOOT_CONFIG_WAIT;
//            }
//            if (CONFIG_Get()) bootCfg_Dt.state = BOOT_CONFIG_LOAD;
//            break;
//        case BOOT_CONFIG_WAIT:
//            if (SYS_TMR_TickCountGet() - _timeBlink >= SYS_TMR_TickCounterFrequencyGet() / 10ul) {
//                _timeBlink = SYS_TMR_TickCountGet();
//                ledDisp_Dt.buff[0] = 0;
//                ledDisp_Dt.buff[1] = 0;
//                ledDisp_Dt.buff[2] = ~ledDisp_Dt.buff[2];
//            }
//
//            if (SYS_TMR_TickCountGet() - _bootTick >= SYS_TMR_TickCounterFrequencyGet()*10) {
//                _bootTick = SYS_TMR_TickCountGet();
//                bootCfg_Dt.state = BOOT_CONFIG_LOAD;
//            }
//            if (CONFIG_Get()) bootCfg_Dt.state = BOOT_CONFIG_WAIT_COMFIRM;
//            break;
//        case BOOT_CONFIG_WAIT_COMFIRM:
//            if (SYS_TMR_TickCountGet() - _timeBlink >= SYS_TMR_TickCounterFrequencyGet() / 10ul) {
//                _timeBlink = SYS_TMR_TickCountGet();
//                ledDisp_Dt.buff[0] = 0;
//                ledDisp_Dt.buff[1] = 0;
//                ledDisp_Dt.buff[2] = ~ledDisp_Dt.buff[2];
//            }
//            if (!CONFIG_Get()) {
//                bootCfg_Dt.proactiveSaveFlag = true;
//                bootCfg_Dt.state = BOOT_CONFIG_SAVE;
//            }
//            break;
//        case BOOT_CONFIG_SAVE:
//            ledDisp_Dt.buff[0] = 0xFF;
//            ledDisp_Dt.buff[1] = 0xFF;
//            ledDisp_Dt.buff[2] = 0xFF;
//#if defined(DEBUG_MODULE_BOOT_CFG) || defined(DEBUG_MODULE_ALL)
//            SYS_CONSOLE_MESSAGE("BOOT-CONFIG: Save \n\r");
//#endif
//            SaveAppConfig(false);
//            SaveAppConfigCounter();
//            SaveAppConfigRtu();
//            SaveAppConfigAnalog();
//            SaveExtendData();
//            COUNTER_ResetValue();
//            ExtendIO_ResetCounterValue();
//            _WaitForFlashTransfer(1000);
//
//            bootCfg_Dt.state = BOOT_CONFIG_LOAD;
//            break;
//        case BOOT_CONFIG_LOAD:
//            ledDisp_Dt.buff[0] = 0xFF;
//            ledDisp_Dt.buff[1] = 0xFF;
//            ledDisp_Dt.buff[2] = 0xFF;
//#if defined(DEBUG_MODULE_BOOT_CFG) || defined(DEBUG_MODULE_ALL)
//            SYS_CONSOLE_MESSAGE("BOOT-CONFIG: Load \n\r");
//#endif
//            DRV_SST26_Read(bootCfg_Dt.driverHandle, &NVMValidationStruct, sizeof (NVMValidationStruct), (uint32_t) BOOT_CONFIG_START_ADDRESS);
//            DRV_SST26_Read(bootCfg_Dt.driverHandle, &_appConfig, sizeof (_appConfig), (uint32_t) BOOT_CONFIG_START_ADDRESS + sizeof (NVMValidationStruct));
//            //            DRV_SST26_Read(bootCfg_Dt.driverHandle, &_appConfigDevice, sizeof (_appConfigDevice), (uint32_t) BOOT_CONFIG_START_ADDRESS + sizeof (NVMValidationStruct) + sizeof (_appConfig));
//            //            DRV_SST26_Read(bootCfg_Dt.driverHandle, &_appConfigAssets, sizeof (_appConfigAssets), (uint32_t) BOOT_CONFIG_START_ADDRESS + sizeof (NVMValidationStruct) + sizeof (_appConfig) + sizeof (_appConfigDevice));
//            DRV_SST26_Read(bootCfg_Dt.driverHandle, &_appCounter, sizeof (_appCounter), (uint32_t) BEGIN_ADDRESS_APPCOUNTER);
//            DRV_SST26_Read(bootCfg_Dt.driverHandle, &_appRtuTags, sizeof (_appRtuTags), (uint32_t) BEGIN_ADDRESS_APPRTU);
//            DRV_SST26_Read(bootCfg_Dt.driverHandle, &_appAnalog, sizeof (_appAnalog), (uint32_t) BEGIN_ADDRESS_ANALOG);
//            DRV_SST26_Read(bootCfg_Dt.driverHandle, &_extend, sizeof (_extend), (uint32_t) BEGIN_ADDRESS_EXTEND);
//
//            _WaitForFlashTransfer(1000);
//            InFlash_LoadDeviceInfo((uint8_t *) & _appConfigDevice, sizeof (_appConfigDevice));
//
//            bootCfg_Dt.state = BOOT_CONFIG_VALIDATE;
//            break;
//        case BOOT_CONFIG_VALIDATE:
//#if defined(DEBUG_MODULE_BOOT_CFG) || defined(DEBUG_MODULE_ALL)
//            SYS_CONSOLE_MESSAGE("BOOT-CONFIG: Validate \n\r");
//#endif
//            if ((NVMValidationStruct.wConfigurationLength != ((uint16_t)sizeof (APP_CONFIG))) ||
//                    (NVMValidationStruct.wOriginalChecksum != wOriginalglbAppCfgChecksum) ||
//                    (NVMValidationStruct.wCurrentChecksum != TCPIP_Helper_CalcIPChecksum((uint8_t *) & _appConfig, sizeof (APP_CONFIG), 0))) {
//                if (bootCfg_Dt.proactiveSaveFlag) {
//                    while (1) {
//                        LEDDISP_Tasks();
//                        ledDisp_Dt.buff[0] = ~ledDisp_Dt.buff[0];
//                        ledDisp_Dt.buff[1] = ~ledDisp_Dt.buff[1];
//                        ledDisp_Dt.buff[2] = ~ledDisp_Dt.buff[2];
//                        CORETIMER_DelayMs(100);
//                    }
//                } else {
//                    SYS_CONSOLE_MESSAGE("BOOT-CONFIG: FAIL \n\r");
//
//                    if (--_numFail > 0) {
//                        bootCfg_Dt.state = BOOT_CONFIG_SAVE;
//                        break;
//                    } else {
//                        _numFail = 3;
//                        while (1) {
//                            LEDDISP_Tasks();
//                            ledDisp_Dt.buff[0] = ~ledDisp_Dt.buff[0];
//                            ledDisp_Dt.buff[1] = ~ledDisp_Dt.buff[1];
//                            ledDisp_Dt.buff[2] = ~ledDisp_Dt.buff[2];
//                            CORETIMER_DelayMs(100);
//                        }
//                    }
//                }
//
//            } else bootCfg_Dt.state = BOOT_CONFIG_COMPLETE;
//            _numFail = 3;
//
//            break;
//        case BOOT_CONFIG_COMPLETE:
//            ledDisp_Dt.buff[0] = 0;
//            ledDisp_Dt.buff[1] = 0;
//            ledDisp_Dt.buff[2] = 0;
//#if defined(DEBUG_MODULE_BOOT_CFG) || defined(DEBUG_MODULE_ALL)
//            SYS_CONSOLE_MESSAGE("BOOT-CONFIG: Complete \n\r");
//#endif
//            memcpy(&glbAppCfg, &_appConfig, sizeof (glbAppCfg));
//            memcpy(&glbAppDev, &_appConfigDevice, sizeof (glbAppDev));
//            //            memcpy(&glbAppAsset, &_appConfigAssets, sizeof (glbAppAsset));
//            memcpy(&glbAppCnter, &_appCounter, sizeof (glbAppCnter));
//            memcpy(&glbAppRtu, &_appRtuTags, sizeof (glbAppRtu));
//            memcpy(&glbAppAnlg, &_appAnalog, sizeof (glbAppAnlg));
//            memcpy(&glbExtend, &_extend, sizeof (glbExtend));
//
//            // position
//            uint8_t startPosition = START_ADDR_POSITION_COUNTER;
//            for (uint8_t idx = 0; idx < MAX_POSITION_SIZE; idx++) {
//                mbTCP_Dt.holdRegs[startPosition] = glbAppCfg.position[idx];
//                startPosition++;
//            }
//            //            bootCfg_Dt.netH = TCPIP_STACK_NetHandleGet("PIC32INT");
//            //            bool rs = TCPIP_STACK_NetBiosNameSet(bootCfg_Dt.netH, (char *) & glbAppCfg.NetBIOSName);
//            //            SYS_CONSOLE_PRINT("rs: %d \n\r",rs);
//
//            CORETIMER_DelayMs(500);
//
//            return true;
//            break;
//    }
//    return false;
//}
//
//void _setStartAddrFlash(uint32_t addr) {
//    bootCfg_Dt.currentAddrFlash = addr;
//}
//
//void _WriteStreamByte(uint8_t *pvData, uint16_t wLen) {
//    if (wLen == 0u) return;
//
//    if ((bootCfg_Dt.currentAddrFlash + wLen) > MPFS_RESERVE_BLOCK) {
//        while (1) {
//            LEDDISP_Tasks();
//            ledDisp_Dt.buff[0] = ~ledDisp_Dt.buff[0];
//            ledDisp_Dt.buff[1] = ~ledDisp_Dt.buff[1];
//            ledDisp_Dt.buff[2] = ~ledDisp_Dt.buff[2];
//            CORETIMER_DelayMs(100);
//        }
//    }
//    uint16_t byteCanWrite = (FLASH_PAGE_SIZE - (bootCfg_Dt.currentAddrFlash % FLASH_PAGE_SIZE));
//    uint8_t *firstData;
//
//    firstData = (uint8_t *) malloc(byteCanWrite * sizeof (uint8_t));
//    if (firstData == NULL) {
//#if defined(DEBUG_MODULE_BOOT_CFG) || defined(DEBUG_MODULE_ALL)
//        SYS_CONSOLE_MESSAGE("ERROR: Memory allocation failed\n");
//#endif
//        return;
//    }
//
//    memcpy(firstData, pvData, byteCanWrite);
//
//    if (byteCanWrite > wLen) byteCanWrite = wLen;
//
//    _WaitForFlashTransfer(500);
//    DRV_SST26_ByteWrite(bootCfg_Dt.driverHandle, firstData, (uint32_t) bootCfg_Dt.currentAddrFlash, byteCanWrite);
//    bootCfg_Dt.currentAddrFlash += byteCanWrite;
//    pvData += byteCanWrite;
//    wLen -= byteCanWrite;
//
//    free(firstData);
//
//    uint16_t numBlocks = wLen / FLASH_PAGE_SIZE;
//
//    while (numBlocks--) {
//        _WaitForFlashTransfer(500);
//        DRV_SST26_PageWrite(bootCfg_Dt.driverHandle, pvData, (uint32_t) bootCfg_Dt.currentAddrFlash);
//        bootCfg_Dt.currentAddrFlash += FLASH_PAGE_SIZE;
//        pvData += FLASH_PAGE_SIZE;
//        wLen -= FLASH_PAGE_SIZE;
//    }
//    if (wLen) {
//        _WaitForFlashTransfer(500);
//        DRV_SST26_ByteWrite(bootCfg_Dt.driverHandle, pvData, (uint32_t) bootCfg_Dt.currentAddrFlash, wLen);
//        bootCfg_Dt.currentAddrFlash += wLen;
//    }
//}
//
//void SaveAppConfig(bool forceSave) {
//    // Get proper values for the validation structure indicating that we can use
//    // these EEPROM/Flash contents on future boot ups
//    //    NVM_VALIDATION_STRUCT NVMValidationStruct;
//    if (isLockSaveFlash()&& !forceSave) return;
//
//    memcpy(&_appConfig, &glbAppCfg, sizeof (_appConfig));
//    //    memcpy(&_appConfigDevice, &glbAppDev, sizeof (_appConfigDevice));
//    //    memcpy(&_appConfigAssets, &glbAppAsset, sizeof (_appConfigAssets));
//
//    NVMValidationStruct.wOriginalChecksum = wOriginalglbAppCfgChecksum;
//    NVMValidationStruct.wCurrentChecksum = TCPIP_Helper_CalcIPChecksum((uint8_t *) & _appConfig, (uint16_t)sizeof (APP_CONFIG), 0);
//    NVMValidationStruct.wConfigurationLength = (uint16_t)sizeof (APP_CONFIG);
//
//    DRV_SST26_SectorErase(bootCfg_Dt.driverHandle, (uint32_t) BOOT_CONFIG_START_ADDRESS);
//    CORETIMER_DelayMs(100);
//    _setStartAddrFlash((uint32_t) BOOT_CONFIG_START_ADDRESS);
//    _WriteStreamByte((uint8_t *) & NVMValidationStruct, sizeof (NVMValidationStruct));
//    _WriteStreamByte((uint8_t *) & _appConfig, sizeof (_appConfig));
//    //    _WriteStreamByte((uint8_t *) & _appConfigDevice, sizeof (_appConfigDevice));
//    //    _WriteStreamByte((uint8_t *) & glbAppAsset, sizeof (glbAppAsset));
//}
//
//void SaveAppConfigCounter() {
//    if (isLockSaveFlash()) return;
//
//    memcpy(&_appCounter, &glbAppCnter, sizeof (_appCounter));
//
//    DRV_SST26_SectorErase(bootCfg_Dt.driverHandle, (uint32_t) BEGIN_ADDRESS_APPCOUNTER);
//    CORETIMER_DelayMs(100);
//    _setStartAddrFlash(BEGIN_ADDRESS_APPCOUNTER);
//    _WriteStreamByte((uint8_t *) & _appCounter, sizeof (APP_COUNTER));
//}
//
//void SaveAppConfigRtu() {
//    if (isLockSaveFlash()) return;
//
//    memcpy(&_appRtuTags, &glbAppRtu, sizeof (_appRtuTags));
//
//    DRV_SST26_SectorErase(bootCfg_Dt.driverHandle, (uint32_t) BEGIN_ADDRESS_APPRTU);
//    CORETIMER_DelayMs(100);
//    _setStartAddrFlash(BEGIN_ADDRESS_APPRTU);
//    _WriteStreamByte((uint8_t *) & _appRtuTags, sizeof (APP_RTU_TAG));
//}
//
//void SaveAppConfigAnalog() {
//    if (isLockSaveFlash()) return;
//
//    memcpy(&_appAnalog, &glbAppAnlg, sizeof (_appAnalog));
//
//    DRV_SST26_SectorErase(bootCfg_Dt.driverHandle, (uint32_t) BEGIN_ADDRESS_ANALOG);
//    CORETIMER_DelayMs(100);
//    _setStartAddrFlash(BEGIN_ADDRESS_ANALOG);
//    _WriteStreamByte((uint8_t *) & _appAnalog, sizeof (APP_ANALOG));
//}
//
//void SaveGetSample() {
//    //    if (isLockSaveFlash()) return;
//
//    memcpy(&_getSample, &glbGetSample, sizeof (_getSample));
//
//    DRV_SST26_SectorErase(bootCfg_Dt.driverHandle, (uint32_t) BEGIN_ADDRESS_GETSAMPLE);
//    CORETIMER_DelayMs(100);
//    _setStartAddrFlash(BEGIN_ADDRESS_GETSAMPLE);
//    _WriteStreamByte((uint8_t *) & _getSample, sizeof (GET_SAMPLE));
//}
//
//void SaveExtendData() {
//    if (isLockSaveFlash()) return;
//
//    memcpy(&_extend, &glbExtend, sizeof (_extend));
//
//    DRV_SST26_SectorErase(bootCfg_Dt.driverHandle, (uint32_t) BEGIN_ADDRESS_EXTEND);
//    CORETIMER_DelayMs(100);
//    _setStartAddrFlash(BEGIN_ADDRESS_EXTEND);
//    _WriteStreamByte((uint8_t *) & _extend, sizeof (EXTEND_CONFIG));
//}
//
//static bool isLockSaveFlash() {
//    if (SYS_TMR_TickCountGet() - _lockSaveFlash >= SYS_TMR_TickCounterFrequencyGet() * 10ul ||
//            _firstSave > 0) {
//        _lockSaveFlash = SYS_TMR_TickCountGet();
//        _firstSave -= (_firstSave == 0) ? 0 : 1;
//    } else {
//        return true;
//    }
//    return false;
//}
//
//static bool _WaitForFlashTransfer(uint32_t timeoutMs) {
//    uint32_t startTick = SYS_TMR_TickCountGet();
//    uint32_t timeoutTicks = (SYS_TMR_TickCounterFrequencyGet() / 1000U) * timeoutMs;
//
//    while (DRV_SST26_TransferStatusGet(bootCfg_Dt.driverHandle) != DRV_SST26_TRANSFER_COMPLETED) {
//        if ((SYS_TMR_TickCountGet() - startTick) > timeoutTicks)
//            return false;
//    }
//    return true;
//}
