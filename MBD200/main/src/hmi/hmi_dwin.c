//#include "hmi_dwin.h"
////#include "modbusRTU/modbusRTU_phy_layer.h"
//#include "generic_types.h"
//#include "rtc/rtc.h"
//#include "sim/core/sim_basic.h"
//
//static const HMI_UART_INTERFACE _hmiUartFunc = {
//    .read_t = (HMI_UART_READ) UART6_Read,
//    .readIsBusy = (HMI_UART_READ_IS_BUSY) UART6_ReadIsBusy,
//    .readAbort = (HMI_UART_READ_ABORT) UART6_ReadAbort,
//    .write_t = (HMI_UART_WRITE) UART6_Write,
//    .writeIsBusy = (HMI_UART_WRITE_IS_BUSY) UART6_WriteIsBusy,
//    .writeCallbackRegister = (HMI_UART_WRITE_CALLBACK_REGISTER) UART6_WriteCallbackRegister
//};
//
//static const HMI_TAG_ENTRY _hmiTagTable[HMI_TAG_MAX_COUNT] = {
//    [HMI_TAG_SWITCH_PAGE] =
//    { .startAddress = HMI_ADDR_PIC_SET, .dataSize = 4},
//    [HMI_TAG_DATETIME] =
//    { .startAddress = HMI_ADDR_DATETIME, .dataSize = 20},
//    [HMI_TAG_NETWORK_SIGNAL] =
//    { .startAddress = HMI_ADDR_SIGNAL1, .dataSize = 32},
////    [HMI_TAG_DEVICE_STATUS] =
////    { .startAddress = HMI_ADDR_SDCARD_STS, .dataSize = 128},    
//    [HMI_TAG_ROW_STATUS] =
//    { .startAddress = HMI_ADDR_ROW_STATUS(0), .dataSize = 4},
//    [HMI_TAG_ROW_VALUE] =
//    { .startAddress = HMI_ADDR_ROW_VALUE(0), .dataSize = 32},
//    [HMI_TAG_PAGE1_ROW_NAME] =
//    { .startAddress = HMI_ADDR_ROW_NAME(0), .dataSize = 160},
//    [HMI_TAG_PAGE1_ROW_UNIT] =
//    { .startAddress = HMI_ADDR_ROW_UNIT(0), .dataSize = 160},
//    [HMI_TAG_PAGE2_ROW_NAME] =
//    { .startAddress = HMI_ADDR_ROW_NAME(5), .dataSize = 160},
//    [HMI_TAG_PAGE2_ROW_UNIT] =
//    { .startAddress = HMI_ADDR_ROW_UNIT(5), .dataSize = 160},
//
//    [HMI_TAG_DEVICE_INFO] =
//    { .startAddress = HMI_ADDR_DEVICE_INFO, .dataSize = 8},
//};
//
//static HMI_TAG_QUEUE _hmiTagQueue = {0};
//static uint8_t _hmiTxBuff[256];
//static bool _uartItfIsReady = true, _hmiBootReady = false;
//static HMI_PENDING _valuePending;
//static HMI_PENDING _statusPending;
//static uint8_t _curPage = 28;
//
//static void _initQueue(HMI_TAG_QUEUE *q) {
//    q->front = 0;
//    q->rear = -1;
//    q->size = 0;
//}
//
//static bool _isQueueFull(HMI_TAG_QUEUE *q) {
//    return q->size == HMI_QUEUE_SIZE;
//}
//
//static bool _isQueueEmpty(HMI_TAG_QUEUE *q) {
//    return q->size == 0;
//}
//
//static bool _enqueue(HMI_TAG_QUEUE *q, const HMI_TAG_TYPE *tag) {
//    if (_isQueueFull(q)) {
//        return false; // Queue is full
//    }
//    q->rear = (q->rear + 1) % HMI_QUEUE_SIZE;
//    q->items[q->rear] = *tag;
//    q->size++;
//    return true;
//}
//
//static bool _dequeue(HMI_TAG_QUEUE *q, HMI_TAG_TYPE *tag) {
//    if (_isQueueEmpty(q))
//        return false; // Queue is empty
//
//    *tag = q->items[q->front];
//    q->front = (q->front + 1) % HMI_QUEUE_SIZE;
//    q->size--;
//    return true;
//}
//
//static void _hmiUartWriteCallbackHandler(uintptr_t context) {
//    memset(_hmiTxBuff, 0, sizeof (_hmiTxBuff));
//    _uartItfIsReady = true;
//}
//
//static void _hmiFlushBuffer(uint8_t len, uint8_t instruction, uint16_t address) {
//    _hmiTxBuff[0] = HMI_FRAME_HEADER1;
//    _hmiTxBuff[1] = HMI_FRAME_HEADER2;
//    _hmiTxBuff[2] = len + 3;
//    _hmiTxBuff[3] = instruction;
//    _hmiTxBuff[4] = (address >> 8) & 0xFF;
//    _hmiTxBuff[5] = (address >> 0) & 0xFF;
//
//    _hmiUartFunc.write_t(_hmiTxBuff, len + HMI_HEADER_LEN);
//    _uartItfIsReady = false;
//
//    //    SYS_CONSOLE_PRINT("%u ", res);
//    //    if (address == 0x4001u) {
//    //        for (uint8_t i = 0; i < 32; i++)
//    //            SYS_CONSOLE_PRINT("%x ", _hmiTxBuff[i]);
//    //    }
//}
//
//static uint16_t _buildFrameData(HMI_TAG_TYPE type, uint8_t maxSize, uint16_t startAddress, uint16_t *targetAddress) {
//    uint8_t * payload = &_hmiTxBuff[HMI_HEADER_LEN];
//    uint16_t endAddress = startAddress + (maxSize / 2);
//
//    switch (type) {
//        case HMI_TAG_SWITCH_PAGE:
//        {
//            payload[0] = 0x5A; //dwin fixed 
//            payload[1] = 0x01; //dwin fixed     
//            payload[2] = 0x00;
//            payload[3] = _curPage;
//           _curPage = 28 + ((_curPage + 1) % 2); // swap trang 28 29
////            _curPage = (_curPage % 4) + 1; 
//            *targetAddress = startAddress;
//            return true;
//        }
//        case HMI_TAG_DATETIME:
//        {
//            snprintf((char*) payload, maxSize,
//                    "%04u-%02u-%02u %02u:%02u:%02u",
//                    rtcDt.sysTime.year, rtcDt.sysTime.month, rtcDt.sysTime.day, rtcDt.sysTime.hour, rtcDt.sysTime.minute, rtcDt.sysTime.second);
//
//            *targetAddress = startAddress;
//            return true;
//        }
//        case HMI_TAG_DEVICE_INFO:
//        {
////            snprintf((char*) payload, maxSize, "%s", gDeviceInfo.fw_code);
//            *targetAddress = startAddress;
//            return true;
//        }
//        case HMI_TAG_NETWORK_SIGNAL: //cot mang 0x5015
//        {
//
//            SIM_BASIC_INFO* simData = SIMBasic_GetInfo();
//            uint16_t offset;
//
//            uint8_t percent;
//            if (simData->rssi == 99 || simData->rssi == 0) percent = 0;
//            else percent = (uint8_t)((float)simData->rssi / 31.0f * 4.0f + 0.5f);
//
//            offset = (HMI_ADDR_SIGNAL1 - startAddress) * 2;
//            payload[offset] = 0x00;
//            payload[offset + 1] = percent; 
//
//            // VP 0x5016 cho tên mang
//            offset = (HMI_ADDR_NETWORK1 - startAddress) * 2;
//            uint8_t nameLen = strlen(simData->networkName);
//            if (nameLen > 0) {
//                uint8_t maxText = 10; 
//                nameLen = (nameLen > maxText) ? maxText : nameLen;
//                memcpy((char *)&payload[offset], simData->networkName, nameLen);
//                
//                payload[offset + nameLen] = 0xFF;
//                payload[offset + nameLen + 1] = 0xFF;
//            }
//
//            *targetAddress = startAddress;
//            return true;
//            
//        }
//        case HMI_TAG_PAGE1_ROW_NAME:
//        {
//            for (uint8_t i = 0; i < (MAX_HMI_PARA / 4 * 1); i++) {
//                uint8_t idxHMI = gAppCfg.hmi[i];
//                uint8_t indexOfType = gSensorCfg.entry[idxHMI].indexOfType;
//                if (idxHMI != 30 && gSensorCfg.entry[idxHMI].enable) {
//                    uint16_t offset;
//                    uint8_t len;
//                    char * str = NULL;
//                    offset = (HMI_ADDR_ROW_NAME(i) - startAddress) * 2;
//                    if (gSensorCfg.entry[idxHMI].type == SENSOR_MBRTU)
//                        str = gMbrtuCfg.entry[indexOfType].name;
//                    else if (gSensorCfg.entry[idxHMI].type == SENSOR_ANALOG)
//                        str = gAnalogCfg.entry[indexOfType].name;
//                    else if (gSensorCfg.entry[idxHMI].type == SENSOR_INPUT_CAPTURE)
//                        str = gInCaptureCfg.entry[indexOfType].name;
//
//                    if (str) {
//                        len = strlen(str);
//                        len = (len <= ((HMI_ADDR_ROW_NAME(i + 1) - HMI_ADDR_ROW_NAME(i)) * 2)) ? len : ((HMI_ADDR_ROW_NAME(i + 1) - HMI_ADDR_ROW_NAME(i)) * 2);
//                        memcpy((char *) &payload[offset], str, len);
//                    }
//                }
//            }
//
//            *targetAddress = startAddress;
//            return true;
//        }
//        case HMI_TAG_PAGE2_ROW_NAME:
//        {
//            for (uint8_t i = (MAX_HMI_PARA / 4 * 1); i < (MAX_HMI_PARA / 4 * 2); i++) {
//                uint8_t idxHMI = gAppCfg.hmi[i];
//                uint8_t indexOfType = gSensorCfg.entry[idxHMI].indexOfType;
//                if (idxHMI != 30 && gSensorCfg.entry[idxHMI].enable) {
//                    uint16_t offset;
//                    uint8_t len;
//                    char * str = NULL;
//                    offset = (HMI_ADDR_ROW_NAME(i) - startAddress) * 2;
//                    if (gSensorCfg.entry[idxHMI].type == SENSOR_MBRTU)
//                        str = gMbrtuCfg.entry[indexOfType].name;
//                    else if (gSensorCfg.entry[idxHMI].type == SENSOR_ANALOG)
//                        str = gAnalogCfg.entry[indexOfType].name;
//                    else if (gSensorCfg.entry[idxHMI].type == SENSOR_INPUT_CAPTURE)
//                        str = gInCaptureCfg.entry[indexOfType].name;
//
//                    if (str) {
//                        len = strlen(str);
//                        len = (len <= ((HMI_ADDR_ROW_NAME(i + 1) - HMI_ADDR_ROW_NAME(i)) * 2)) ? len :
//                                ((HMI_ADDR_ROW_NAME(i + 1) - HMI_ADDR_ROW_NAME(i)) * 2);
//                        memcpy((char *) &payload[offset], str, len);
//                    }
//                }
//            }
//
//            *targetAddress = startAddress;
//            return true;
//        }
//      
//        case HMI_TAG_PAGE1_ROW_UNIT:
//        {
//            for (uint8_t i = 0; i < (MAX_HMI_PARA / 4 * 1); i++) {
//                uint8_t idxHMI = gAppCfg.hmi[i];
//                uint8_t indexOfType = gSensorCfg.entry[idxHMI].indexOfType;
//                if (idxHMI != 30 && gSensorCfg.entry[idxHMI].enable) {
//                    uint16_t offset;
//                    uint8_t len;
//                    char * str = NULL;
//                    offset = (HMI_ADDR_ROW_UNIT(i) - startAddress) * 2;
//                    if (gSensorCfg.entry[idxHMI].type == SENSOR_MBRTU)
//                        str = gMbrtuCfg.entry[indexOfType].unit;
//                    else if (gSensorCfg.entry[idxHMI].type == SENSOR_ANALOG)
//                        str = gAnalogCfg.entry[indexOfType].unit;
//                    else if (gSensorCfg.entry[idxHMI].type == SENSOR_INPUT_CAPTURE)
//                        str = gInCaptureCfg.entry[indexOfType].unit;
//
//                    if (str) {
//                        len = strlen(str);
//                        len = (len <= ((HMI_ADDR_ROW_UNIT(i + 1) - HMI_ADDR_ROW_UNIT(i)) * 2)) ? len :
//                                ((HMI_ADDR_ROW_UNIT(i + 1) - HMI_ADDR_ROW_UNIT(i)) * 2);
//                        memcpy((char *) &payload[offset], str, len);
//                    }
//                }
//            }
//
//            *targetAddress = startAddress;
//            return true;
//        }
//        case HMI_TAG_PAGE2_ROW_UNIT:
//        {
//            for (uint8_t i = (MAX_HMI_PARA / 4 * 1); i < (MAX_HMI_PARA / 4 * 2); i++) {
//                uint8_t idxHMI = gAppCfg.hmi[i];
//                uint8_t indexOfType = gSensorCfg.entry[idxHMI].indexOfType;
//                if (idxHMI != 30 && gSensorCfg.entry[idxHMI].enable) {
//                    uint16_t offset;
//                    uint8_t len;
//                    char * str = NULL;
//                    offset = (HMI_ADDR_ROW_UNIT(i) - startAddress) * 2;
//                    if (gSensorCfg.entry[idxHMI].type == SENSOR_MBRTU)
//                        str = gMbrtuCfg.entry[indexOfType].unit;
//                    else if (gSensorCfg.entry[idxHMI].type == SENSOR_ANALOG)
//                        str = gAnalogCfg.entry[indexOfType].unit;
//                    else if (gSensorCfg.entry[idxHMI].type == SENSOR_INPUT_CAPTURE)
//                        str = gInCaptureCfg.entry[indexOfType].unit;
//
//                    if (str) {
//                        len = strlen(str);
//                        len = (len <= ((HMI_ADDR_ROW_UNIT(i + 1) - HMI_ADDR_ROW_UNIT(i)) * 2)) ? len :
//                                ((HMI_ADDR_ROW_UNIT(i + 1) - HMI_ADDR_ROW_UNIT(i)) * 2);
//                        memcpy((char *) &payload[offset], str, len);
//                    }
//                }
//            }
//
//            *targetAddress = startAddress;
//            return true;
//        }
// 
//        case HMI_TAG_ROW_VALUE:
//        {
//            uint8_t i = 0;
//            for (; i < MAX_HMI_PARA; i++)
//                if (_valuePending.entry[i].isPending) break;
//
//            if (i >= MAX_HMI_PARA) break;
//            _valuePending.entry[i].isPending = false;
//            uint8_t len;
//            len = strlen(_valuePending.entry[i].buffer);
//            len = (len <= ((HMI_ADDR_ROW_VALUE(i + 1) - HMI_ADDR_ROW_VALUE(i)) * 2)) ? len :
//                    ((HMI_ADDR_ROW_VALUE(i + 1) - HMI_ADDR_ROW_VALUE(i)) * 2);
//            memcpy((char *) payload, _valuePending.entry[i].buffer, len);
//            *targetAddress = HMI_ADDR_ROW_VALUE(i);
//
//            if (_valuePending.numPending > 0)
//                _valuePending.numPending--;
//            _valuePending.lock = false;
//            return true;
//        }
//        case HMI_TAG_ROW_STATUS:
//        {
//            uint8_t i = 0;
//            for (; i < MAX_HMI_PARA; i++)
//                if (_statusPending.entry[i].isPending) break;
//
//            if (i >= MAX_HMI_PARA) break;
//            _statusPending.entry[i].isPending = false;
//            payload[0] = 0x00;
//            payload[1] = _statusPending.entry[i].buffer[0] - '0';
//            //                payload[1] = 0x02;
//            *targetAddress = HMI_ADDR_ROW_STATUS(i);
//
//            if (_statusPending.numPending > 0)
//                _statusPending.numPending--;
//            _statusPending.lock = false;
//            return true;
//        }
//        case HMI_TAG_DEVICE_STATUS:
//        {
//            uint16_t offset;
//            uint8_t len;
//            char str[16] = "";
//
////            if (sdCard_Dt.status == 0) snprintf(str, sizeof (str), "NO INSERT");
////            else if (sdCard_Dt.status == 1) snprintf(str, sizeof (str), "ERROR");
////            else if (sdCard_Dt.status == 2) snprintf(str, sizeof (str), "INSERTED");
////            else if (sdCard_Dt.status == 3) snprintf(str, sizeof (str), "GOOD");
////            else snprintf(str, sizeof (str), "FAIL");
////
////            offset = (HMI_ADDR_SDCARD_STS - startAddress) * 2;
////            len = strlen(str);
////            len = (len <= ((HMI_ADDR_FTP_STS - HMI_ADDR_SDCARD_STS) * 2)) ? len : ((HMI_ADDR_FTP_STS - HMI_ADDR_SDCARD_STS) * 2);
////            memcpy((char *) &payload[offset], str, len);
////
////
////            if (appData.ftpStatus == GOOD) snprintf(str, sizeof (str), "GOOD");
////            else if (appData.ftpStatus == BAD) snprintf(str, sizeof (str), "ERROR");
////            else snprintf(str, sizeof (str), "UNKNOWN");
////
////            offset = (HMI_ADDR_FTP_STS - startAddress) * 2;
////            len = strlen(str);
////            len = (len <= ((HMI_ADDR_SIM1_STS - HMI_ADDR_FTP_STS) * 2)) ? len : ((HMI_ADDR_SIM1_STS - HMI_ADDR_FTP_STS) * 2);
////            memcpy((char *) &payload[offset], str, len);
////
////
////            offset = (HMI_ADDR_SIM1_STS - startAddress) * 2;
////            len = strlen(gsmCOM_Dt.SIM[0].status);
////            len = (len <= ((HMI_ADDR_SIM2_STS - HMI_ADDR_SIM1_STS) * 2)) ? len : ((HMI_ADDR_SIM2_STS - HMI_ADDR_SIM1_STS) * 2);
////            memcpy((char *) &payload[offset], gsmCOM_Dt.SIM[0].status, len);
////
////
////            offset = (HMI_ADDR_SIM2_STS - startAddress) * 2;
////            len = strlen(gsmCOM_Dt.SIM[1].status);
////            len = (len <= ((endAddress - HMI_ADDR_SIM2_STS) * 2)) ? len : ((endAddress - HMI_ADDR_SIM2_STS) * 2);
////            memcpy((char *) &payload[offset], gsmCOM_Dt.SIM[1].status, len);
//
//            *targetAddress = startAddress;
//            return true;
//        }
//        default: break;
//    }
//    return false;
//}
//
//void HMIDwin_Initialize() {
//    _initQueue(&_hmiTagQueue);
//    _hmiUartFunc.writeCallbackRegister(_hmiUartWriteCallbackHandler, (uintptr_t) NULL);
//}
//
//void HMIDwin_Tasks() {
//    static HMI_TAG_TYPE tagType = HMI_TAG_MAX_COUNT;
//    static uint32_t dateTimeTick = 0;
//    static uint32_t swPageTick = 0;
//    static uint32_t bootTick = 0;
//    static uint32_t cleanTick = 0;
//    static uint32_t timeoutTick = 0;
//    static bool bootFirst = false;
//
//    uint32_t currentTick = SYS_TMR_TickCountGet();
//    uint32_t tickPerSecond = SYS_TMR_TickCounterFrequencyGet();
//
//    if (!bootFirst && (currentTick - bootTick > tickPerSecond * 5)) {
//        bootTick = currentTick;
//        bootFirst = true;
//        _hmiBootReady = true;
//        tagType = HMI_TAG_SWITCH_PAGE;
//        _enqueue(&_hmiTagQueue, &tagType);
//        tagType = HMI_TAG_NETWORK_SIGNAL;
//        _enqueue(&_hmiTagQueue, &tagType);
//        tagType = HMI_TAG_DEVICE_STATUS;
//        _enqueue(&_hmiTagQueue, &tagType);
//        tagType = HMI_TAG_DEVICE_INFO;
//        _enqueue(&_hmiTagQueue, &tagType);
//        tagType = HMI_TAG_PAGE1_ROW_NAME;
//        _enqueue(&_hmiTagQueue, &tagType);
//        tagType = HMI_TAG_PAGE1_ROW_UNIT;
//        _enqueue(&_hmiTagQueue, &tagType);
//        tagType = HMI_TAG_PAGE2_ROW_NAME;
//        _enqueue(&_hmiTagQueue, &tagType);
//        tagType = HMI_TAG_PAGE2_ROW_UNIT;
//        _enqueue(&_hmiTagQueue, &tagType);
//
//    }
//
//    if (!_uartItfIsReady && (currentTick - timeoutTick > tickPerSecond * 3)) {
//        timeoutTick = currentTick;
//        _uartItfIsReady = true;
//    }
//
//    if (_hmiBootReady) {
//        if (currentTick - dateTimeTick > tickPerSecond / 2) {
//            dateTimeTick = currentTick;
//            tagType = HMI_TAG_DATETIME;
//            _enqueue(&_hmiTagQueue, &tagType);
//        }
//        // Nhay signal
//        static uint32_t signalTick = 0;
//        if (currentTick - signalTick > tickPerSecond * 2) { // C? 2 giây c?p nh?t sóng 1 l?n
//            signalTick = currentTick;
//            tagType = HMI_TAG_NETWORK_SIGNAL;
//            _enqueue(&_hmiTagQueue, &tagType);
//        }
//        //-----------------------------------
//        if (currentTick - swPageTick > tickPerSecond * 30) {
//            swPageTick = currentTick;
//            tagType = HMI_TAG_SWITCH_PAGE;
//            _enqueue(&_hmiTagQueue, &tagType);
//        }
//
//        if (!_valuePending.lock && _valuePending.numPending > 0) {
//            _valuePending.lock = true;
//            tagType = HMI_TAG_ROW_VALUE;
//            _enqueue(&_hmiTagQueue, &tagType);
//        }
//
//        if (!_statusPending.lock && _statusPending.numPending > 0) {
//            _statusPending.lock = true;
//            tagType = HMI_TAG_ROW_STATUS;
//            _enqueue(&_hmiTagQueue, &tagType);
//        }
//
//        if (currentTick - cleanTick > tickPerSecond * 5) {
//            cleanTick = currentTick;
//
//            for (uint8_t i = 0; i < MAX_HMI_PARA; i++) {
//                uint8_t idxHMI = gAppCfg.hmi[i];
//                if (idxHMI == 30 || gSensorCfg.entry[idxHMI].enable == false) {
//                    HMIDwin_TriggerSendStatus(i, 0);
//                    HMIDwin_TriggerSendValue(i, HMI_DATA_NONE, 0);
//                }
//            }
//        }
//    }
//
//    if (_uartItfIsReady && _dequeue(&_hmiTagQueue, &tagType)) {
//        if (tagType >= HMI_TAG_MAX_COUNT) return;
//
//        const HMI_TAG_ENTRY* tagEntry = &_hmiTagTable[tagType];
//        bool res = 0;
//        uint16_t targetAddress = 0;
//        res = _buildFrameData(tagType, tagEntry->dataSize, tagEntry->startAddress, &targetAddress);
//
//        if (res) {
//            timeoutTick = currentTick;
//            _hmiFlushBuffer(tagEntry->dataSize, HMI_INSTRUCTION_WRITE, targetAddress);
//        }
//    }
//}
////bool HMIDwin_TriggerSendStatus(uint8_t idxRow, STATUS status)
//bool HMIDwin_TriggerSendStatus(uint8_t idxRow, uint8_t status) {
//    HMI_TAG_TYPE tagType = HMI_TAG_ROW_STATUS;
//
//    if (!_hmiBootReady || idxRow >= MAX_HMI_PARA) return false;
//    if (status == STATUS_BAD)
//        _statusPending.entry[idxRow].buffer[0] = '0';
//    else if (status == STATUS_IDENTIFYING)
//        _statusPending.entry[idxRow].buffer[0] = '1';
//    else
//        _statusPending.entry[idxRow].buffer[0] = '2';
//
//    if (_statusPending.entry[idxRow].isPending == false)
//        _statusPending.numPending++;
//    _statusPending.entry[idxRow].isPending = true;
//    _statusPending.lock = false;
//    _enqueue(&_hmiTagQueue, &tagType);
//    return true;
//}
//
//bool HMIDwin_TriggerSendValue(uint8_t idxRow, HMI_TAG_DATA_TYPE dataType, float data) {
//    HMI_TAG_TYPE tagType = HMI_TAG_ROW_VALUE;
//
//    if (!_hmiBootReady || idxRow >= MAX_HMI_PARA) return false;
//    if (dataType == HMI_DATA_FLOAT)
//        snprintf(_valuePending.entry[idxRow].buffer, sizeof (_valuePending.entry[idxRow].buffer), "%.2f", (float) data);
//    else if (dataType == HMI_DATA_UINT)
//        snprintf(_valuePending.entry[idxRow].buffer, sizeof (_valuePending.entry[idxRow].buffer), "%u", (uint32_t) data);
//    else if (dataType == HMI_DATA_INT)
//        snprintf(_valuePending.entry[idxRow].buffer, sizeof (_valuePending.entry[idxRow].buffer), "%d", (int32_t) data);
//    else
//        snprintf(_valuePending.entry[idxRow].buffer, sizeof (_valuePending.entry[idxRow].buffer), "%s", "");
//
//    if (_valuePending.entry[idxRow].isPending == false)
//        _valuePending.numPending++;
//    _valuePending.entry[idxRow].isPending = true;
//    _valuePending.lock = false;
//    _enqueue(&_hmiTagQueue, &tagType);
//    return true;
//}
//
//bool HMIDwin_TriggerSend(HMI_TAG_TYPE tagType) {
//    if (!_hmiBootReady || tagType >= HMI_TAG_MAX_COUNT) return false;
//    _enqueue(&_hmiTagQueue, &tagType);
//    return true;
//}