#include "system_config.h"
#include "system_definitions.h"
#include "http_net_print.h"
//#include "bootloader/bootloader_nvm_interface.h"

#include "tcpip/tcpip.h"
#include "tcpip/src/common/helpers.h"
#include "net_pres/pres/net_pres_socketapi.h"
#include "system/sys_random_h2_adapter.h"
#include "system/sys_time_h2_adapter.h"
#include "tcpip/tcpip.h"
#include "tcpip/src/common/helpers.h"

static const char * __TAG__ = "CUSTOM_HTTP";
static char _dynVarBuffer[512];
static bool lastFailure = false;
bool _passwordIsChanged = false;
//static uint8_t md5Hash[20];
//static char _dynVarBuffer[200];

/****************************************************************************
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
  Section:
    Customized HTTP NET Functions
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 ****************************************************************************/


void TCPIP_HTTP_NET_DynAcknowledge(TCPIP_HTTP_NET_CONN_HANDLE connHandle, const void *buffer, const TCPIP_HTTP_NET_USER_CALLBACK *pCBack) {
    HTTP_APP_DYNVAR_BUFFER *pDynBuffer = (HTTP_APP_DYNVAR_BUFFER*) ((const uint8_t *) buffer - offsetof(struct HTTP_APP_DYNVAR_BUFFER, data));
    pDynBuffer->busy = 0;
}

void TCPIP_HTTP_NET_EventReport(TCPIP_HTTP_NET_CONN_HANDLE connHandle, TCPIP_HTTP_NET_EVENT_TYPE evType, const void *evInfo, const TCPIP_HTTP_NET_USER_CALLBACK *pCBack) {
    const char *evMsg = (const char *) evInfo;

    if (evType < 0) { // display errors only
        if (evMsg == 0) {
            evMsg = "none";
        }
    }
}

bool TCPIP_HTTP_NET_SSINotification(TCPIP_HTTP_NET_CONN_HANDLE connHandle, TCPIP_HTTP_SSI_NOTIFY_DCPT *pSSINotifyDcpt, const TCPIP_HTTP_NET_USER_CALLBACK *pCBack) {

    return false;
}

/****************************************************************************
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
  Section:
    GET Form Handlers
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 ****************************************************************************/

TCPIP_HTTP_NET_IO_RESULT TCPIP_HTTP_NET_ConnectionGetExecute(TCPIP_HTTP_NET_CONN_HANDLE connHandle, const TCPIP_HTTP_NET_USER_CALLBACK *pCBack) {

    return TCPIP_HTTP_NET_IO_RES_DONE;
}

/****************************************************************************
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
  Section:
    POST Form Handlers
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 ****************************************************************************/

#if defined(TCPIP_HTTP_NET_USE_POST)

static TCPIP_HTTP_NET_IO_RESULT HTTPPostSensorGeneral(TCPIP_HTTP_NET_CONN_HANDLE connHandle);
static TCPIP_HTTP_NET_IO_RESULT HTTPPostHmiDisplay(TCPIP_HTTP_NET_CONN_HANDLE connHandle);
static TCPIP_HTTP_NET_IO_RESULT HTTPPostAnalog(TCPIP_HTTP_NET_CONN_HANDLE connHandle);
static TCPIP_HTTP_NET_IO_RESULT HTTPPostModbus(TCPIP_HTTP_NET_CONN_HANDLE connHandle);
static TCPIP_HTTP_NET_IO_RESULT HTTPPostInputCapture(TCPIP_HTTP_NET_CONN_HANDLE connHandle);
static TCPIP_HTTP_NET_IO_RESULT HTTPPostOutput(TCPIP_HTTP_NET_CONN_HANDLE connHandle);
static TCPIP_HTTP_NET_IO_RESULT HTTPPostUser(TCPIP_HTTP_NET_CONN_HANDLE connHandle);
static TCPIP_HTTP_NET_IO_RESULT HTTPPostTime(TCPIP_HTTP_NET_CONN_HANDLE connHandle);
static TCPIP_HTTP_NET_IO_RESULT HTTPPostSerialCom(TCPIP_HTTP_NET_CONN_HANDLE connHandle);
static TCPIP_HTTP_NET_IO_RESULT HTTPPostNetwork(TCPIP_HTTP_NET_CONN_HANDLE connHandle);
static TCPIP_HTTP_NET_IO_RESULT HTTPPostSim(TCPIP_HTTP_NET_CONN_HANDLE connHandle);
static TCPIP_HTTP_NET_IO_RESULT HTTPPostFtp(TCPIP_HTTP_NET_CONN_HANDLE connHandle);
static TCPIP_HTTP_NET_IO_RESULT HTTPPostMqtt(TCPIP_HTTP_NET_CONN_HANDLE connHandle);
static TCPIP_HTTP_NET_IO_RESULT HTTPPostStorage(TCPIP_HTTP_NET_CONN_HANDLE connHandle);

TCPIP_HTTP_NET_IO_RESULT TCPIP_HTTP_NET_ConnectionPostExecute(TCPIP_HTTP_NET_CONN_HANDLE connHandle, const TCPIP_HTTP_NET_USER_CALLBACK *pCBack) {
    // Resolve which function to use and pass along
    uint8_t filename[20];
    // Load the file name
    // Make sure uint8_t filename[] above is large enough for your longest name
    filename[0] = 0;
    SYS_FS_FileNameGet(TCPIP_HTTP_NET_ConnectionFileGet(connHandle), filename, sizeof (filename));

    if (!memcmp(filename, "sensor-general.html", 19))
        return HTTPPostSensorGeneral(connHandle);
    if (!memcmp(filename, "hmi-display.html", 16))
        return HTTPPostHmiDisplay(connHandle);
    if (!memcmp(filename, "analog.html", 11))
        return HTTPPostAnalog(connHandle);
    if (!memcmp(filename, "modbus.html", 11))
        return HTTPPostModbus(connHandle);
    if (!memcmp(filename, "input-capture.html", 18))
        return HTTPPostInputCapture(connHandle);
    if (!memcmp(filename, "output.html", 11))
        return HTTPPostOutput(connHandle);
    if (!memcmp(filename, "user.html", 9))
        return HTTPPostUser(connHandle);
    if (!memcmp(filename, "time.html", 9))
        return HTTPPostTime(connHandle);
    if (!memcmp(filename, "serial-com.html", 15))
        return HTTPPostSerialCom(connHandle);
    if (!memcmp(filename, "network.html", 12))
        return HTTPPostNetwork(connHandle);
    if (!memcmp(filename, "sim.html", 8))
        return HTTPPostSim(connHandle);
    if (!memcmp(filename, "ftp.html", 8))
        return HTTPPostFtp(connHandle);
    if (!memcmp(filename, "mqtt.html", 9))
        return HTTPPostMqtt(connHandle);
    if (!memcmp(filename, "storage.html", 12))
        return HTTPPostStorage(connHandle);

    TCPIP_HTTP_NET_ConnectionStatusSet(connHandle, TCPIP_HTTP_NET_STAT_UPLOAD_ERROR);
    return TCPIP_HTTP_NET_IO_RES_DONE;
}
//
//static TCPIP_HTTP_NET_IO_RESULT HTTPPostReset(TCPIP_HTTP_NET_CONN_HANDLE connHandle) {
//    bool bConfigFailure = false;
//    uint8_t *httpDataBuff = 0;
//    uint16_t httpBuffSize;
//    uint32_t byteCount;
//
//    byteCount = TCPIP_HTTP_NET_ConnectionByteCountGet(connHandle);
//
//    if (byteCount > TCPIP_HTTP_NET_ConnectionReadBufferSize(connHandle)) { // Configuration Failure
//        lastFailure = true;
//        TCPIP_HTTP_NET_ConnectionStatusSet(connHandle, TCPIP_HTTP_NET_STAT_REDIRECT);
//        return TCPIP_HTTP_NET_IO_RES_DONE;
//    }
//
//    // Ensure that all data is waiting to be parsed.  If not, keep waiting for
//    // all of it to arrive.
//    if (TCPIP_HTTP_NET_ConnectionReadIsReady(connHandle) < byteCount)
//        return TCPIP_HTTP_NET_IO_RES_NEED_DATA;
//
//    // Use current config in non-volatile memory as defaults
//    httpDataBuff = TCPIP_HTTP_NET_ConnectionDataBufferGet(connHandle); // chi lay con tro, tro den buffer, chua co data
//    httpBuffSize = TCPIP_HTTP_NET_ConnectionDataBufferSizeGet(connHandle);
//
//    // Read all browser POST data
//    while (TCPIP_HTTP_NET_ConnectionByteCountGet(connHandle)) {
//        // Read a form field name
//        if (TCPIP_HTTP_NET_ConnectionPostNameRead(connHandle, httpDataBuff, 12) != TCPIP_HTTP_NET_READ_OK) {
//            bConfigFailure = true;
//            break;
//        }
//
//        // Read a form field value
//        if (TCPIP_HTTP_NET_ConnectionPostValueRead(connHandle, httpDataBuff + 12, httpBuffSize - 12 - 2) != TCPIP_HTTP_NET_READ_OK) {
//            bConfigFailure = true;
//            break;
//        }
//
//        // Parse the value that was read
//        if (!strcmp((char *) httpDataBuff, (const char *) "reset")) {// Save new static IP Address
//            uint8_t check = atoi((char *) (httpDataBuff + 12));
//            if (check != 1) {
//                bConfigFailure = true;
//                break;
//            } else {
//                _passwordIsChanged = check;
//                SYS_RESET_SoftwareReset();
//            }
//        } else if (!strcmp((char *) httpDataBuff, (const char *) "switch")) {
//            uint8_t check = atoi((char *) (httpDataBuff + 12));
//            if (check != 1) {
//                bConfigFailure = true;
//                break;
//            } else {
//                //                if (newFirmware)
//                //                    bootloader_SwapAndReset();
//            }
//        }
//        //        else if (!strcmp((char *) httpDataBuff, (const char *) "md5Hash")) {
//        //            char tempStr[35];
//        //            uint8_t i, j;
//        //            snprintf(tempStr, sizeof (tempStr), "%s", (char *) (httpDataBuff + 12));
//        //            for (i = 0, j = 0; i < 16; i++, j++) {
//        //                md5Hash[j] = utilities_HexFromChars(tempStr[2 * i], tempStr[2 * i + 1]);
//        //            }
//        //        }
//    }
//
//    if (bConfigFailure == false) {
//        strcpy((char *) httpDataBuff, "index.htm?");
//    } else { // Configuration error
//        lastFailure = true;
//        if (httpDataBuff) {
//            strcpy((char *) httpDataBuff, "index.htm");
//        }
//    }
//
//    TCPIP_HTTP_NET_ConnectionStatusSet(connHandle, TCPIP_HTTP_NET_STAT_REDIRECT);
//
//    return TCPIP_HTTP_NET_IO_RES_DONE;
//}
//
//static TCPIP_HTTP_NET_IO_RESULT HTTPPostNetworkConfig(TCPIP_HTTP_NET_CONN_HANDLE connHandle) {
//    bool bConfigFailure = false;
//    uint8_t *httpDataBuff = 0;
//    uint16_t httpBuffSize;
//    uint32_t byteCount;
//
//    //    char newAppUsernameDevice[24];
//    //    char newAppPasswordDevice[24];
//    //    char oldAppPasswordDevice[24];
//
//    // Check to see if the browser is attempting to submit more data than we
//    // can parse at once.  This function needs to receive all updated
//    // parameters and validate them all before committing them to memory so that
//    // orphaned configuration parameters do not get written (for example, if a
//    // static IP address is given, but the subnet mask fails parsing, we
//    // should not use the static IP address).  Everything needs to be processed
//    // in a single transaction.  If this is impossible, fail and notify the user.
//    // As a web devloper, if you add parameters to the network info and run into this
//    // problem, you could fix this by to splitting your update web page into two
//    // seperate web pages (causing two transactional writes).  Alternatively,
//    // you could fix it by storing a static shadow copy of network info someplace
//    // in memory and using it when info is complete.
//    // Lastly, you could increase the TCP RX FIFO size for the HTTP server.
//    // This will allow more data to be POSTed by the web browser before hitting this limit.
//    byteCount = TCPIP_HTTP_NET_ConnectionByteCountGet(connHandle);
//
//    if (byteCount > TCPIP_HTTP_NET_ConnectionReadBufferSize(connHandle)) { // Configuration Failure
//        lastFailure = true;
//        TCPIP_HTTP_NET_ConnectionStatusSet(connHandle, TCPIP_HTTP_NET_STAT_REDIRECT);
//        return TCPIP_HTTP_NET_IO_RES_DONE;
//    }
//
//    // Ensure that all data is waiting to be parsed.  If not, keep waiting for
//    // all of it to arrive.
//    if (TCPIP_HTTP_NET_ConnectionReadIsReady(connHandle) < byteCount)
//        return TCPIP_HTTP_NET_IO_RES_NEED_DATA;
//
//    // Use current config in non-volatile memory as defaults
//    httpDataBuff = TCPIP_HTTP_NET_ConnectionDataBufferGet(connHandle); // chi lay con tro, tro den buffer, chua co data
//    httpBuffSize = TCPIP_HTTP_NET_ConnectionDataBufferSizeGet(connHandle);
//
//    // Read all browser POST data
//    while (TCPIP_HTTP_NET_ConnectionByteCountGet(connHandle)) {
//        // Read a form field name
//        if (TCPIP_HTTP_NET_ConnectionPostNameRead(connHandle, httpDataBuff, 12) != TCPIP_HTTP_NET_READ_OK) {
//            bConfigFailure = true;
//            break;
//        }
//
//        // Read a form field value
//        if (TCPIP_HTTP_NET_ConnectionPostValueRead(connHandle, httpDataBuff + 12, httpBuffSize - 12 - 2) != TCPIP_HTTP_NET_READ_OK) {
//            bConfigFailure = true;
//            break;
//        }
//
//        // Parse the value that was read
//        //        if (!strcmp((char *) httpDataBuff, (const char *) "dhcp_en")) {// Save new static IP Address
//        //            uint8_t isDHCPEnable = atoi((char *) (httpDataBuff + 12));
//        //            if (isDHCPEnable != 1 && isDHCPEnable != 0) {
//        //                bConfigFailure = true;
//        //                break;
//        //            }
//        //            glbAppCfg.network.isDHCPEn = isDHCPEnable;
//        //        } else if (!strcmp((char *) httpDataBuff, (const char *) "IPAddress")) {// Save new static IP Address
//        //            if (glbAppCfg.network.isDHCPEn == false) {
//        //                if (!TCPIP_Helper_StringToIPAddress((char *) (httpDataBuff + 12), &glbAppCfg.network.ipAddr)) {
//        //                    bConfigFailure = true;
//        //                    break;
//        //                }
//        //            }
//        //        } else if (!strcmp((char *) httpDataBuff, (const char *) "gateway")) {// Read new gateway address
//        //            if (glbAppCfg.network.isDHCPEn == false) {
//        //                if (!TCPIP_Helper_StringToIPAddress((char *) (httpDataBuff + 12), &glbAppCfg.network.gateway)) {
//        //                    bConfigFailure = true;
//        //                    break;
//        //                }
//        //            }
//        //        } else if (!strcmp((char *) httpDataBuff, (const char *) "Subnet")) {// Read new static subnet
//        //            if (glbAppCfg.network.isDHCPEn == false) {
//        //                if (!TCPIP_Helper_StringToIPAddress((char *) (httpDataBuff + 12), &glbAppCfg.network.ipMask)) {
//        //                    bConfigFailure = true;
//        //                    break;
//        //                }
//        //            }
//        //        } else if (!strcmp((char *) httpDataBuff, (const char *) "username")) {
//        //            snprintf(newAppUsernameDevice, sizeof (newAppUsernameDevice), "%s", (char *) (httpDataBuff + 12));
//        //        } else if (!strcmp((char *) httpDataBuff, (const char *) "oldpass")) {
//        //            snprintf(oldAppPasswordDevice, sizeof (oldAppPasswordDevice), "%s", (char *) (httpDataBuff + 12));
//        //        } else if (!strcmp((char *) httpDataBuff, (const char *) "newpass")) {
//        //            snprintf(newAppPasswordDevice, sizeof (newAppPasswordDevice), "%s", (char *) (httpDataBuff + 12));
//        //            if (strncmp(glbAppCfg.network.app_password_device, oldAppPasswordDevice, strlen(glbAppCfg.network.app_password_device)) == 0) {
//        //                snprintf(glbAppCfg.network.app_username_device, sizeof (glbAppCfg.network.app_username_device), "%s", newAppUsernameDevice);
//        //                snprintf(glbAppCfg.network.app_password_device, sizeof (glbAppCfg.network.app_password_device), "%s", newAppPasswordDevice);
//        //                _passwordIsChanged = true;
//        //            } else if (strncmp(glbAppCfg.network.app_password_device, oldAppPasswordDevice, strlen(glbAppCfg.network.app_password_device)) != 0) {
//        //                _passwordIsChanged = false;
//        //            }
//        //        }
//    }
//
//    if (bConfigFailure == false) {
//        //        if (glbAppCfg.network.isDHCPEn == false) {
//        //            TCPIP_NET_HANDLE netH;
//        //            bool dhcpRes;
//        //            netH = TCPIP_STACK_IndexToNet(0);
//        //            dhcpRes = TCPIP_DHCP_Disable(netH);
//        //            if (dhcpRes) {
//        //                TCPIP_STACK_NetAddressSet(netH, &glbAppCfg.network.ipAddr, &glbAppCfg.network.ipMask, true);
//        //                TCPIP_STACK_NetAddressGatewaySet(netH, &glbAppCfg.network.gateway);
//        //            }
//        //            TCPIP_STACK_NetBiosNameSet(netH, glbAppCfg.network.NetBIOSName);
//        //        } else if (glbAppCfg.network.isDHCPEn == true) {
//        //            TCPIP_NET_HANDLE netH;
//        //            netH = TCPIP_STACK_IndexToNet(0);
//        //            TCPIP_DHCP_Request(netH, glbAppCfg.network.ipAddr);
//        //            TCPIP_STACK_NetBiosNameSet(netH, glbAppCfg.network.NetBIOSName);
//        //        }
//        //
//        //        SaveAppConfig(false);
//
//        //        CORETIMER_DelayMs(1000);
//
//        SYS_RESET_SoftwareReset();
//        // All parsing complete!  Save new settings and force an interface restart
//        // Set the interface to restart and display reconnecting information
//        strcpy((char *) httpDataBuff, "network.htm?");
//        //        TCPIP_Helper_FormatNetBIOSName((uint8_t *) httpNetData.nbnsName);
//        //        memcpy((void *) (httpDataBuff + 20), httpNetData.nbnsName, 16);
//        //        httpDataBuff[20 + 16] = 0x00; // Force null termination
//        //        for (i = 20; i < 20u + 16u; ++i) {
//        //            if (httpDataBuff[i] == ' ')
//        //                httpDataBuff[i] = 0x00;
//        //        }
//        //        // save current interface and mark as valid
//        //        httpNetData.currNet = TCPIP_HTTP_NET_ConnectionNetHandle(connHandle);
//        //        strncpy(httpNetData.ifName, TCPIP_STACK_NetNameGet(httpNetData.currNet), sizeof (httpNetData.ifName) - 1);
//        //        httpNetData.ifName[sizeof (httpNetData.ifName) - 1] = 0;
//    } else { // Configuration error
//        lastFailure = true;
//        if (httpDataBuff) {
//            strcpy((char *) httpDataBuff, "index.htm");
//        }
//    }
//
//    TCPIP_HTTP_NET_ConnectionStatusSet(connHandle, TCPIP_HTTP_NET_STAT_REDIRECT);
//
//    return TCPIP_HTTP_NET_IO_RES_DONE;
//}

static TCPIP_HTTP_NET_IO_RESULT HTTPPostSensorGeneral(TCPIP_HTTP_NET_CONN_HANDLE connHandle) {
    bool bConfigFailure = false;
    uint8_t *httpDataBuff = 0;
    uint16_t httpBuffSize;
    uint32_t byteCount;

    uint8_t id = 0xFF;
    char actionStr[16] = {0};

    SENSOR_ENTRY_CONFIG tempCfg = {0};
    uint16_t tempLogInterval = 0;
    FORMAT_FILE tempDataFormat = (FORMAT_FILE) 0;
    FILE_TYPE tempFileType = (FILE_TYPE) 0;
    char tempFileName[FILE_NAME_LEN] = {0};
    bool tempCompress = false;
    bool tempFtpEnable = false;
    bool tempMqttEnable = false;
    bool tempSdEnable = false;

    struct {
        bool id;
        bool sensorType, sensorId, sourceType;
        bool success_source_type, success_tag_id, success_and_value, success_expected_value;
        bool error_source_type, error_tag_id, error_and_value, error_expected_value;
        bool calib_source_type, calib_tag_id, calib_and_value, calib_expected_value;
        bool sensorCalibrated;
        bool enable;

        bool logInterval, dataFormat, fileName, fileType, compress, ftpEnable, mqttEnable, sdEnable;
    } received = {0};

    byteCount = TCPIP_HTTP_NET_ConnectionByteCountGet(connHandle);

    if (byteCount > TCPIP_HTTP_NET_ConnectionReadBufferSize(connHandle)) {
        TCPIP_HTTP_NET_ConnectionStatusSet(connHandle, TCPIP_HTTP_NET_STAT_REDIRECT);
        return TCPIP_HTTP_NET_IO_RES_DONE;
    }

    if (TCPIP_HTTP_NET_ConnectionReadIsReady(connHandle) < byteCount)
        return TCPIP_HTTP_NET_IO_RES_NEED_DATA;

    httpDataBuff = TCPIP_HTTP_NET_ConnectionDataBufferGet(connHandle);
    httpBuffSize = TCPIP_HTTP_NET_ConnectionDataBufferSizeGet(connHandle);

    while (TCPIP_HTTP_NET_ConnectionByteCountGet(connHandle)) {
        if (TCPIP_HTTP_NET_ConnectionPostNameRead(connHandle, httpDataBuff, 32) != TCPIP_HTTP_NET_READ_OK ||
                TCPIP_HTTP_NET_ConnectionPostValueRead(connHandle, httpDataBuff + 32, httpBuffSize - 32 - 2) != TCPIP_HTTP_NET_READ_OK) {
            bConfigFailure = true;
            break;
        }

        char *paramName = (char *) httpDataBuff;
        char *paramValue = (char *) (httpDataBuff + 32);

        if (!strcmp(paramName, "action")) {
            strncpy(actionStr, paramValue, sizeof (actionStr) - 1);
        } else if (!strcmp(paramName, "id")) {
            id = atoi(paramValue);
            if (id > 0) id--;
            received.id = true;
        } else if (!strcmp(paramName, "sensorType")) {
            tempCfg.type = (SENSOR_TYPE) atoi(paramValue);
            received.sensorType = true;
        } else if (!strcmp(paramName, "sensorId")) {
            tempCfg.indexOfType = (uint8_t) atoi(paramValue);
            received.sensorId = true;
        } else if (!strcmp(paramName, "sourceType")) {
            tempCfg.typeStatus = (STATUS_SOURCE) atoi(paramValue);
            received.sourceType = true;
        } else if (!strcmp(paramName, "success_source_type")) {
            tempCfg.typeGood = (STATUS_SOURCE) atoi(paramValue);
            received.success_source_type = true;
        } else if (!strcmp(paramName, "success_tag_id")) {
            tempCfg.indexOfTypeGood = (uint8_t) atoi(paramValue);
            received.success_tag_id = true;
        } else if (!strcmp(paramName, "success_and_value")) {
            tempCfg.goodValueAND = (uint16_t) atoi(paramValue);
            received.success_and_value = true;
        } else if (!strcmp(paramName, "success_expected_value")) {
            tempCfg.goodValueCompare = (uint16_t) atoi(paramValue);
            received.success_expected_value = true;
        } else if (!strcmp(paramName, "error_source_type")) {
            tempCfg.typeErr = (STATUS_SOURCE) atoi(paramValue);
            received.error_source_type = true;
        } else if (!strcmp(paramName, "error_tag_id")) {
            tempCfg.indexOfTypeErr = (uint8_t) atoi(paramValue);
            received.error_tag_id = true;
        } else if (!strcmp(paramName, "error_and_value")) {
            tempCfg.errorValueAND = (uint16_t) atoi(paramValue);
            received.error_and_value = true;
        } else if (!strcmp(paramName, "error_expected_value")) {
            tempCfg.errorValueCompare = (uint16_t) atoi(paramValue);
            received.error_expected_value = true;
        } else if (!strcmp(paramName, "calib_source_type")) {
            tempCfg.typeCalib = (STATUS_SOURCE) atoi(paramValue);
            received.calib_source_type = true;
        } else if (!strcmp(paramName, "calib_tag_id")) {
            tempCfg.indexOfTypeCalib = (uint8_t) atoi(paramValue);
            received.calib_tag_id = true;
        } else if (!strcmp(paramName, "calib_and_value")) {
            tempCfg.calibValueAND = (uint16_t) atoi(paramValue);
            received.calib_and_value = true;
        } else if (!strcmp(paramName, "calib_expected_value")) {
            tempCfg.calibValueCompare = (uint16_t) atoi(paramValue);
            received.calib_expected_value = true;
        } else if (!strcmp(paramName, "sensorCalibrated")) {
            tempCfg.calibrate = (!strcmp(paramValue, "on") || !strcmp(paramValue, "1"));
            received.sensorCalibrated = true;
        } else if (!strcmp(paramName, "sensorActive")) {
            tempCfg.enable = (!strcmp(paramValue, "on") || !strcmp(paramValue, "1"));
            received.enable = true;
        } else if (!strcmp(paramName, "logInterval")) {
            tempLogInterval = (uint16_t) atoi(paramValue);
            received.logInterval = true;
        } else if (!strcmp(paramName, "dataFormat")) {
            tempDataFormat = (FORMAT_FILE) atoi(paramValue);
            received.dataFormat = true;
        } else if (!strcmp(paramName, "fileName")) {
            strncpy(tempFileName, paramValue, sizeof (tempFileName) - 1);
            tempFileName[sizeof (tempFileName) - 1] = '\0';
            received.fileName = true;
        } else if (!strcmp(paramName, "fileType")) {
            tempFileType = (FILE_TYPE) atoi(paramValue);
            received.fileType = true;
        } else if (!strcmp(paramName, "compress")) {
            tempCompress = (!strcmp(paramValue, "on") || !strcmp(paramValue, "1"));
            received.compress = true;
        } else if (!strcmp(paramName, "ftpEnable")) {
            tempFtpEnable = (!strcmp(paramValue, "on") || !strcmp(paramValue, "1"));
            received.ftpEnable = true;
        } else if (!strcmp(paramName, "mqttEnable")) {
            tempMqttEnable = (!strcmp(paramValue, "on") || !strcmp(paramValue, "1"));
            received.mqttEnable = true;
        } else if (!strcmp(paramName, "sdEnable")) {
            tempSdEnable = (!strcmp(paramValue, "on") || !strcmp(paramValue, "1"));
            received.sdEnable = true;
        }
    }

    TCPIP_HTTP_NET_STATUS status = TCPIP_HTTP_NET_STAT_UPLOAD_ERROR;

    if (!bConfigFailure) {
        bool configChanged = false;

        if (!strcmp(actionStr, "delete")) {
            if (received.id && id < gSensorCfg.numSensor) {
                for (uint8_t i = id; i < gSensorCfg.numSensor - 1; i++)
                    gSensorCfg.entry[i] = gSensorCfg.entry[i + 1];

                memset(&gSensorCfg.entry[gSensorCfg.numSensor - 1], 0, sizeof (SENSOR_ENTRY_CONFIG));
                gSensorCfg.numSensor--;
                configChanged = true;
            }
        } else if (!strcmp(actionStr, "add") || !strcmp(actionStr, "edit")) {
            uint8_t targetId = id;
            bool canProceed = false;

            if (!strcmp(actionStr, "add")) {
                if (gSensorCfg.numSensor < MAX_SENSOR) {
                    targetId = gSensorCfg.numSensor;
                    canProceed = true;
                }
            } else {
                if (received.id && targetId < gSensorCfg.numSensor)
                    canProceed = true;
            }

            if (canProceed) {
                SENSOR_ENTRY_CONFIG finalCfg;

                if (!strcmp(actionStr, "edit")) {
                    finalCfg = gSensorCfg.entry[targetId];
                } else {
                    memset(&finalCfg, 0, sizeof (SENSOR_ENTRY_CONFIG));
                }

                if (received.sensorType) finalCfg.type = tempCfg.type;
                if (received.sensorId) finalCfg.indexOfType = tempCfg.indexOfType;
                if (received.sourceType) finalCfg.typeStatus = tempCfg.typeStatus;

                if (received.success_source_type) finalCfg.typeGood = tempCfg.typeGood;
                if (received.success_tag_id) finalCfg.indexOfTypeGood = tempCfg.indexOfTypeGood;
                if (received.success_and_value) finalCfg.goodValueAND = tempCfg.goodValueAND;
                if (received.success_expected_value) finalCfg.goodValueCompare = tempCfg.goodValueCompare;

                if (received.error_source_type) finalCfg.typeErr = tempCfg.typeErr;
                if (received.error_tag_id) finalCfg.indexOfTypeErr = tempCfg.indexOfTypeErr;
                if (received.error_and_value) finalCfg.errorValueAND = tempCfg.errorValueAND;
                if (received.error_expected_value) finalCfg.errorValueCompare = tempCfg.errorValueCompare;

                if (received.calib_source_type) finalCfg.typeCalib = tempCfg.typeCalib;
                if (received.calib_tag_id) finalCfg.indexOfTypeCalib = tempCfg.indexOfTypeCalib;
                if (received.calib_and_value) finalCfg.calibValueAND = tempCfg.calibValueAND;
                if (received.calib_expected_value) finalCfg.calibValueCompare = tempCfg.calibValueCompare;

                if (received.sensorCalibrated) finalCfg.calibrate = tempCfg.calibrate;
                else finalCfg.calibrate = false;
                if (received.enable) finalCfg.enable = tempCfg.enable;
                else finalCfg.enable = false;

                gSensorCfg.entry[targetId] = finalCfg;

                if (!strcmp(actionStr, "add"))
                    gSensorCfg.numSensor++;

                configChanged = true;
            }
        }

        if (received.logInterval) {
            gSensorCfg.logInterval = tempLogInterval;
            configChanged = true;
        }
        if (received.dataFormat) {
            gSensorCfg.formatFile = tempDataFormat;
            configChanged = true;
        }
        if (received.fileType) {
            gSensorCfg.typefile = tempFileType;
            configChanged = true;
        }
        if (received.fileName) {
            strncpy(gSensorCfg.filenameTemplate, tempFileName, sizeof (gSensorCfg.filenameTemplate) - 1);
            gSensorCfg.filenameTemplate[sizeof (gSensorCfg.filenameTemplate) - 1] = '\0';
            configChanged = true;
        }
        if (received.compress) {
            gSensorCfg.compressed = tempCompress;
            configChanged = true;
        }
        if (received.ftpEnable) {
            gSensorCfg.uploadFtp = tempFtpEnable;
            configChanged = true;
        }
        if (received.mqttEnable) {
            gSensorCfg.uploadMqtt = tempMqttEnable;
            configChanged = true;
        }
        if (received.sdEnable) {
            gSensorCfg.saveSdcard = tempSdEnable;
            configChanged = true;
        }

        if (configChanged)
            ExtFlash_SaveConfig(EXTFL_DATA_SENSOR_CFG, NULL);

        status = TCPIP_HTTP_NET_STAT_UPLOAD_OK;
    }

    TCPIP_HTTP_NET_ConnectionStatusSet(connHandle, status);

    return TCPIP_HTTP_NET_IO_RES_DONE;
}

static TCPIP_HTTP_NET_IO_RESULT HTTPPostHmiDisplay(TCPIP_HTTP_NET_CONN_HANDLE connHandle) {
    bool bConfigFailure = false;
    uint8_t *httpDataBuff = 0;
    uint16_t httpBuffSize;
    uint32_t byteCount;

    HMI_CONFIG tempHmiCfg = {0};

    byteCount = TCPIP_HTTP_NET_ConnectionByteCountGet(connHandle);
    if (byteCount > TCPIP_HTTP_NET_ConnectionReadBufferSize(connHandle)) {
        TCPIP_HTTP_NET_ConnectionStatusSet(connHandle, TCPIP_HTTP_NET_STAT_REDIRECT);
        return TCPIP_HTTP_NET_IO_RES_DONE;
    }

    if (TCPIP_HTTP_NET_ConnectionReadIsReady(connHandle) < byteCount)
        return TCPIP_HTTP_NET_IO_RES_NEED_DATA;

    httpDataBuff = TCPIP_HTTP_NET_ConnectionDataBufferGet(connHandle);
    httpBuffSize = TCPIP_HTTP_NET_ConnectionDataBufferSizeGet(connHandle);

    while (TCPIP_HTTP_NET_ConnectionByteCountGet(connHandle)) {
        if (TCPIP_HTTP_NET_ConnectionPostNameRead(connHandle, httpDataBuff, 32) != TCPIP_HTTP_NET_READ_OK ||
                TCPIP_HTTP_NET_ConnectionPostValueRead(connHandle, httpDataBuff + 32, httpBuffSize - 32 - 2) != TCPIP_HTTP_NET_READ_OK) {
            bConfigFailure = true;
            break;
        }

        char *paramName = (char *) httpDataBuff;
        char *paramValue = (char *) (httpDataBuff + 32);

        int posIndex = -1;
        if (sscanf(paramName, "sensor_pos_%d", &posIndex) == 1) {
            if (posIndex >= 0 && posIndex < MAX_HMI_PARA) {
                tempHmiCfg.sensorIdx[posIndex] = (uint8_t) atoi(paramValue);

                if (posIndex + 1 > tempHmiCfg.numEntry) {
                    tempHmiCfg.numEntry = posIndex + 1;
                }
            }
        }
    }

    TCPIP_HTTP_NET_STATUS status = TCPIP_HTTP_NET_STAT_UPLOAD_ERROR;

    if (!bConfigFailure) {
        memcpy(&gAppCfg.hmi, &tempHmiCfg, sizeof (HMI_CONFIG));

        ExtFlash_SaveConfig(EXTFL_DATA_APP_CFG, NULL);
        status = TCPIP_HTTP_NET_STAT_UPLOAD_OK;
    }

    TCPIP_HTTP_NET_ConnectionStatusSet(connHandle, status);

    return TCPIP_HTTP_NET_IO_RES_DONE;
}

static TCPIP_HTTP_NET_IO_RESULT HTTPPostAnalog(TCPIP_HTTP_NET_CONN_HANDLE connHandle) {
    bool bConfigFailure = false;
    uint8_t *httpDataBuff = 0;
    uint16_t httpBuffSize;
    uint32_t byteCount;

    uint8_t id = 0xFF;
    ANALOG_CHANNEL_CONFIG tempCfg = {0};

    struct {
        bool id;
        bool name, unit, raw_min, raw_max, eu_min, eu_max;
        bool scale_type, scale_data_type, scale_value;
        bool offset_before, op_1, op_2, offset_after, enabled;
    } received = {0};

    byteCount = TCPIP_HTTP_NET_ConnectionByteCountGet(connHandle);
    if (byteCount > TCPIP_HTTP_NET_ConnectionReadBufferSize(connHandle)) {
        TCPIP_HTTP_NET_ConnectionStatusSet(connHandle, TCPIP_HTTP_NET_STAT_REDIRECT);
        return TCPIP_HTTP_NET_IO_RES_DONE;
    }
    if (TCPIP_HTTP_NET_ConnectionReadIsReady(connHandle) < byteCount)
        return TCPIP_HTTP_NET_IO_RES_NEED_DATA;

    httpDataBuff = TCPIP_HTTP_NET_ConnectionDataBufferGet(connHandle);
    httpBuffSize = TCPIP_HTTP_NET_ConnectionDataBufferSizeGet(connHandle);

    while (TCPIP_HTTP_NET_ConnectionByteCountGet(connHandle)) {
        if (TCPIP_HTTP_NET_ConnectionPostNameRead(connHandle, httpDataBuff, 32) != TCPIP_HTTP_NET_READ_OK ||
                TCPIP_HTTP_NET_ConnectionPostValueRead(connHandle, httpDataBuff + 32, httpBuffSize - 32 - 2) != TCPIP_HTTP_NET_READ_OK) {
            bConfigFailure = true;
            break;
        }

        char *paramName = (char *) httpDataBuff;
        char *paramValue = (char *) (httpDataBuff + 32);

        if (!strcmp(paramName, "analog_channel_id")) {
            id = atoi(paramValue);
            if (id > 0) id--;
            received.id = true;
        } else if (!strcmp(paramName, "sensor_name")) {
            strncpy(tempCfg.name, paramValue, SENSOR_NAME_LEN - 1);
            tempCfg.name[SENSOR_NAME_LEN - 1] = '\0';
            received.name = true;
        } else if (!strcmp(paramName, "unit")) {
            strncpy(tempCfg.unit, paramValue, SENSOR_UNIT_LEN - 1);
            tempCfg.unit[SENSOR_UNIT_LEN - 1] = '\0';
            received.unit = true;
        } else if (!strcmp(paramName, "raw_min")) {
            tempCfg.inputLow = atof(paramValue);
            received.raw_min = true;
        } else if (!strcmp(paramName, "raw_max")) {
            tempCfg.inputHigh = atof(paramValue);
            received.raw_max = true;
        } else if (!strcmp(paramName, "eu_min")) {
            tempCfg.outputLow = atof(paramValue);
            received.eu_min = true;
        } else if (!strcmp(paramName, "eu_max")) {
            tempCfg.outputHigh = atof(paramValue);
            received.eu_max = true;
        } else if (!strcmp(paramName, "scale_type")) {
            tempCfg.scaleType = (SENSOR_SCALE_TYPE) atoi(paramValue);
            received.scale_type = true;
        } else if (!strcmp(paramName, "scale_data_type")) {
            tempCfg.scaleDataType = (SENSOR_DATA_TYPE) atoi(paramValue);
            received.scale_data_type = true;
        } else if (!strcmp(paramName, "scale_value")) {
            tempCfg.scaleValue = atof(paramValue);
            received.scale_value = true;
        } else if (!strcmp(paramName, "offset_before")) {
            tempCfg.offsetPreVal = atof(paramValue);
            received.offset_before = true;
        } else if (!strcmp(paramName, "operator_1")) {
            tempCfg.offSetPreOperator = (OPERATOR) atoi(paramValue);
            received.op_1 = true;
        } else if (!strcmp(paramName, "operator_2")) {
            tempCfg.offsetSubOperator = (OPERATOR) atoi(paramValue);
            received.op_2 = true;
        } else if (!strcmp(paramName, "offset_after")) {
            tempCfg.offsetSubVal = atof(paramValue);
            received.offset_after = true;
        } else if (!strcmp(paramName, "enabled")) {
            if (!strcmp(paramValue, "on") || !strcmp(paramValue, "1")) {
                tempCfg.enable = true;
                received.enabled = true;
            }
        }
    }

    TCPIP_HTTP_NET_STATUS status = TCPIP_HTTP_NET_STAT_UPLOAD_ERROR;

    if (!bConfigFailure && received.id && (id < MAX_ANALOG_CHANNEL)) {

        ANALOG_CHANNEL_CONFIG finalCfg = gAnalogCfg.entry[id];

        if (received.name) strcpy(finalCfg.name, tempCfg.name);
        if (received.unit) strcpy(finalCfg.unit, tempCfg.unit);
        if (received.raw_min) finalCfg.inputLow = tempCfg.inputLow;
        if (received.raw_max) finalCfg.inputHigh = tempCfg.inputHigh;
        if (received.eu_min) finalCfg.outputLow = tempCfg.outputLow;
        if (received.eu_max) finalCfg.outputHigh = tempCfg.outputHigh;
        if (received.scale_type) finalCfg.scaleType = tempCfg.scaleType;
        if (received.scale_data_type) finalCfg.scaleDataType = tempCfg.scaleDataType;
        if (received.scale_value) finalCfg.scaleValue = tempCfg.scaleValue;
        if (received.offset_before) finalCfg.offsetPreVal = tempCfg.offsetPreVal;
        if (received.op_1) finalCfg.offSetPreOperator = tempCfg.offSetPreOperator;
        if (received.op_2) finalCfg.offsetSubOperator = tempCfg.offsetSubOperator;
        if (received.offset_after) finalCfg.offsetSubVal = tempCfg.offsetSubVal;
        if (received.enabled) finalCfg.enable = tempCfg.enable;
        else finalCfg.enable = false;

        if (finalCfg.inputHigh <= finalCfg.inputLow)
            bConfigFailure = true;

        if (strlen(finalCfg.name) == 0)
            bConfigFailure = true;


        if (!bConfigFailure) {
            Adc_TriggerReinit();
            gAnalogCfg.entry[id] = finalCfg;
            ExtFlash_SaveConfig(EXTFL_DATA_ANALOG_CFG, NULL);
            status = TCPIP_HTTP_NET_STAT_UPLOAD_OK;
        }
    }

    TCPIP_HTTP_NET_ConnectionStatusSet(connHandle, status);
    return TCPIP_HTTP_NET_IO_RES_DONE;
}

static TCPIP_HTTP_NET_IO_RESULT HTTPPostModbus(TCPIP_HTTP_NET_CONN_HANDLE connHandle) {
    bool bConfigFailure = false;
    uint8_t *httpDataBuff = 0;
    uint16_t httpBuffSize;
    uint32_t byteCount;

    char actionStr[16] = {0};
    uint8_t id = 0xFF;
    MODBUSRTU_TAG_ENTRY tempCfg = {0};

    struct {
        bool id, action;
        bool name, unit, type, ipAddress, port;
        bool slaveAddress, function, regAddress, quantity, rawDataType, byteOder;
        bool inputMin, inputMax, outputMin, outputMax;
        bool conversion, scaleType, scaleDataType, scaleValue;
        bool offsetPreVal, offsetSubVal, offSetPreOperator, offsetSubOperator;
        bool enabled;
    } received = {0};

    byteCount = TCPIP_HTTP_NET_ConnectionByteCountGet(connHandle);

    if (byteCount > TCPIP_HTTP_NET_ConnectionReadBufferSize(connHandle)) {
        TCPIP_HTTP_NET_ConnectionStatusSet(connHandle, TCPIP_HTTP_NET_STAT_REDIRECT);
        return TCPIP_HTTP_NET_IO_RES_DONE;
    }

    if (TCPIP_HTTP_NET_ConnectionReadIsReady(connHandle) < byteCount)
        return TCPIP_HTTP_NET_IO_RES_NEED_DATA;

    httpDataBuff = TCPIP_HTTP_NET_ConnectionDataBufferGet(connHandle);
    httpBuffSize = TCPIP_HTTP_NET_ConnectionDataBufferSizeGet(connHandle);

    /* Phase 1: Parse all data into temporary buffer */
    while (TCPIP_HTTP_NET_ConnectionByteCountGet(connHandle)) {
        if (TCPIP_HTTP_NET_ConnectionPostNameRead(connHandle, httpDataBuff, 32) != TCPIP_HTTP_NET_READ_OK) {
            bConfigFailure = true;
            break;
        }

        if (TCPIP_HTTP_NET_ConnectionPostValueRead(connHandle, httpDataBuff + 32, httpBuffSize - 32 - 2) != TCPIP_HTTP_NET_READ_OK) {
            bConfigFailure = true;
            break;
        }

        char *paramName = (char *) httpDataBuff;
        char *paramValue = (char *) (httpDataBuff + 32);

        if (!strcmp(paramName, "action")) {
            strncpy(actionStr, paramValue, sizeof (actionStr) - 1);
            received.action = true;
        } else if (!strcmp(paramName, "modbus_device_id") || !strcmp(paramName, "device_id")) {
            id = (uint8_t) atoi(paramValue);
            if (id > 0) id--;
            received.id = true;
        } else if (!strcmp(paramName, "device_name")) {
            strncpy(tempCfg.name, paramValue, SENSOR_NAME_LEN - 1);
            received.name = true;
        } else if (!strcmp(paramName, "device_unit")) {
            strncpy(tempCfg.unit, paramValue, SENSOR_UNIT_LEN - 1);
            received.unit = true;
        } else if (!strcmp(paramName, "modbus_mode")) {
            tempCfg.type = (MODBUS_TYPE) atoi(paramValue);
            received.type = true;
        } else if (!strcmp(paramName, "tcp_ip")) {
            TCPIP_Helper_StringToIPAddress(paramValue, &tempCfg.ipAddress);
            received.ipAddress = true;
        } else if (!strcmp(paramName, "tcp_port")) {
            tempCfg.port = (uint16_t) atoi(paramValue);
            received.port = true;
        } else if (!strcmp(paramName, "slave_id")) {
            tempCfg.slaveAddress = (uint8_t) atoi(paramValue);
            received.slaveAddress = true;
        } else if (!strcmp(paramName, "function_code")) {
            tempCfg.function = (uint8_t) atoi(paramValue);
            received.function = true;
        } else if (!strcmp(paramName, "register_address")) {
            tempCfg.regAddress = (uint16_t) atoi(paramValue);
            received.regAddress = true;
        } else if (!strcmp(paramName, "quantity")) {
            tempCfg.quantity = (uint8_t) atoi(paramValue);
            received.quantity = true;
        } else if (!strcmp(paramName, "data_type")) {
            tempCfg.rawDataType = (SENSOR_DATA_TYPE) atoi(paramValue);
            received.rawDataType = true;
        } else if (!strcmp(paramName, "byte_order")) {
            tempCfg.byteOder = (BYTE_ORDER_TYPE) atoi(paramValue);
            received.byteOder = true;
        } else if (!strcmp(paramName, "raw_min")) {
            tempCfg.inputMin = atof(paramValue);
            received.inputMin = true;
        } else if (!strcmp(paramName, "raw_max")) {
            tempCfg.inputMax = atof(paramValue);
            received.inputMax = true;
        } else if (!strcmp(paramName, "output_min")) {
            tempCfg.outputMin = atof(paramValue);
            received.outputMin = true;
        } else if (!strcmp(paramName, "output_max")) {
            tempCfg.outputMax = atof(paramValue);
            received.outputMax = true;
        } else if (!strcmp(paramName, "scale_type")) {
            tempCfg.scaleType = (SENSOR_SCALE_TYPE) atoi(paramValue);
            received.scaleType = true;
        } else if (!strcmp(paramName, "scale_data_type")) {
            tempCfg.scaleDataType = (SENSOR_DATA_TYPE) atoi(paramValue);
            received.scaleDataType = true;
        } else if (!strcmp(paramName, "scale_value")) {
            tempCfg.scaleValue = atof(paramValue);
            received.scaleValue = true;
        } else if (!strcmp(paramName, "offset_before")) {
            tempCfg.offsetPreVal = atof(paramValue);
            received.offsetPreVal = true;
        } else if (!strcmp(paramName, "operator_1")) {
            tempCfg.offSetPreOperator = (OPERATOR) atoi(paramValue);
            received.offSetPreOperator = true;
        } else if (!strcmp(paramName, "operator_2")) {
            tempCfg.offsetSubOperator = (OPERATOR) atoi(paramValue);
            received.offsetSubOperator = true;
        } else if (!strcmp(paramName, "offset_after")) {
            tempCfg.offsetSubVal = atof(paramValue);
            received.offsetSubVal = true;
        } else if (!strcmp(paramName, "enabled")) {
            if (!strcmp(paramValue, "on") || !strcmp(paramValue, "1")) {
                tempCfg.enable = true;
                received.enabled = true;
            }
        } else if (!strcmp(paramName, "conversion")) {
            if (!strcmp(paramValue, "on") || !strcmp(paramValue, "1")) {
                tempCfg.conversion = true;
                received.conversion = true;
            }
        }
    }

    /* Phase 2: Action handler & Data merge */
    TCPIP_HTTP_NET_STATUS status = TCPIP_HTTP_NET_STAT_UPLOAD_ERROR;

    if (!bConfigFailure && received.action) {

        if (!strcmp(actionStr, "add") || !strcmp(actionStr, "edit")) {
            bool isAdd = !strcmp(actionStr, "add");

            if (isAdd && (gMbrtuCfg.numTag >= MAX_MODBUS_TAG)) {
                bConfigFailure = true;
            } else if (!isAdd && (!received.id || id >= gMbrtuCfg.numTag)) {
                bConfigFailure = true;
            }

            if (!bConfigFailure) {
                MODBUSRTU_TAG_ENTRY finalCfg;

                if (isAdd) {
                    memset(&finalCfg, 0, sizeof (MODBUSRTU_TAG_ENTRY));
                } else {
                    finalCfg = gMbrtuCfg.entry[id];
                }

                if (received.name) strcpy(finalCfg.name, tempCfg.name);
                if (received.unit) strcpy(finalCfg.unit, tempCfg.unit);
                if (received.type) finalCfg.type = tempCfg.type;
                if (received.ipAddress) finalCfg.ipAddress = tempCfg.ipAddress;
                if (received.port) finalCfg.port = tempCfg.port;
                if (received.slaveAddress) finalCfg.slaveAddress = tempCfg.slaveAddress;
                if (received.function) finalCfg.function = tempCfg.function;
                if (received.regAddress) finalCfg.regAddress = tempCfg.regAddress;
                if (received.quantity) finalCfg.quantity = tempCfg.quantity;
                if (received.rawDataType) finalCfg.rawDataType = tempCfg.rawDataType;
                if (received.byteOder) finalCfg.byteOder = tempCfg.byteOder;

                if (received.conversion) finalCfg.conversion = tempCfg.conversion;
                else finalCfg.conversion = false;
                if (received.inputMin) finalCfg.inputMin = tempCfg.inputMin;
                if (received.inputMax) finalCfg.inputMax = tempCfg.inputMax;
                if (received.outputMin) finalCfg.outputMin = tempCfg.outputMin;
                if (received.outputMax) finalCfg.outputMax = tempCfg.outputMax;

                if (received.scaleType) finalCfg.scaleType = tempCfg.scaleType;
                if (received.scaleDataType) finalCfg.scaleDataType = tempCfg.scaleDataType;
                if (received.scaleValue) finalCfg.scaleValue = tempCfg.scaleValue;

                if (received.offsetPreVal) finalCfg.offsetPreVal = tempCfg.offsetPreVal;
                if (received.offSetPreOperator) finalCfg.offSetPreOperator = tempCfg.offSetPreOperator;
                if (received.offsetSubOperator) finalCfg.offsetSubOperator = tempCfg.offsetSubOperator;
                if (received.offsetSubVal) finalCfg.offsetSubVal = tempCfg.offsetSubVal;

                if (received.enabled) finalCfg.enable = tempCfg.enable;
                else finalCfg.enable = false;

                if (strlen(finalCfg.name) == 0) {
                    bConfigFailure = true;
                }

                if (!bConfigFailure) {
                    if (isAdd) {
                        gMbrtuCfg.entry[gMbrtuCfg.numTag] = finalCfg;
                        gMbrtuCfg.numTag++;
                    } else {
                        gMbrtuCfg.entry[id] = finalCfg;
                    }
                    ExtFlash_SaveConfig(EXTFL_DATA_MBRTU_CFG, NULL);
                    status = TCPIP_HTTP_NET_STAT_UPLOAD_OK;
                }
            }
        } else if (!strcmp(actionStr, "delete")) {
            if (received.id && (id < gMbrtuCfg.numTag)) {
                /* Shift array to fill the gap */
                for (uint8_t i = id; i < gMbrtuCfg.numTag - 1; i++) {
                    gMbrtuCfg.entry[i] = gMbrtuCfg.entry[i + 1];
                }

                gMbrtuCfg.numTag--;
                memset(&gMbrtuCfg.entry[gMbrtuCfg.numTag], 0, sizeof (MODBUSRTU_TAG_ENTRY));

                ExtFlash_SaveConfig(EXTFL_DATA_MBRTU_CFG, NULL);
                status = TCPIP_HTTP_NET_STAT_UPLOAD_OK;
            }
        }
    }

    TCPIP_HTTP_NET_ConnectionStatusSet(connHandle, status);
    return TCPIP_HTTP_NET_IO_RES_DONE;
}

static TCPIP_HTTP_NET_IO_RESULT HTTPPostInputCapture(TCPIP_HTTP_NET_CONN_HANDLE connHandle) {
    bool bConfigFailure = false;
    uint8_t *httpDataBuff = 0;
    uint16_t httpBuffSize;
    uint32_t byteCount;

    uint8_t id = 0xFF;
    INPUT_CAPTURE_CHANNEL_CONFIG tempCfg = {0};

    struct {
        bool id;
        bool name, unit, pulse_per_unit;
        bool scale_type, scale_data_type, scale_value;
        bool offset_before, op_1, op_2, offset_after;
        bool enabled;
    } received = {0};

    byteCount = TCPIP_HTTP_NET_ConnectionByteCountGet(connHandle);
    if (byteCount > TCPIP_HTTP_NET_ConnectionReadBufferSize(connHandle)) {
        TCPIP_HTTP_NET_ConnectionStatusSet(connHandle, TCPIP_HTTP_NET_STAT_REDIRECT);
        return TCPIP_HTTP_NET_IO_RES_DONE;
    }
    if (TCPIP_HTTP_NET_ConnectionReadIsReady(connHandle) < byteCount)
        return TCPIP_HTTP_NET_IO_RES_NEED_DATA;

    httpDataBuff = TCPIP_HTTP_NET_ConnectionDataBufferGet(connHandle);
    httpBuffSize = TCPIP_HTTP_NET_ConnectionDataBufferSizeGet(connHandle);

    while (TCPIP_HTTP_NET_ConnectionByteCountGet(connHandle)) {
        if (TCPIP_HTTP_NET_ConnectionPostNameRead(connHandle, httpDataBuff, 32) != TCPIP_HTTP_NET_READ_OK ||
                TCPIP_HTTP_NET_ConnectionPostValueRead(connHandle, httpDataBuff + 32, httpBuffSize - 32 - 2) != TCPIP_HTTP_NET_READ_OK) {
            bConfigFailure = true;
            break;
        }

        char *paramName = (char *) httpDataBuff;
        char *paramValue = (char *) (httpDataBuff + 32);

        if (!strcmp(paramName, "di_channel_id")) {
            id = atoi(paramValue);
            received.id = true;
        } else if (!strcmp(paramName, "channel_name")) {
            strncpy(tempCfg.name, paramValue, SENSOR_NAME_LEN - 1);
            tempCfg.name[SENSOR_NAME_LEN - 1] = '\0';
            received.name = true;
        } else if (!strcmp(paramName, "unit")) {
            strncpy(tempCfg.unit, paramValue, SENSOR_UNIT_LEN - 1);
            tempCfg.unit[SENSOR_UNIT_LEN - 1] = '\0';
            received.unit = true;
        } else if (!strcmp(paramName, "pulse_per_unit")) {
            tempCfg.valPerPulse = atof(paramValue);
            received.pulse_per_unit = true;
        } else if (!strcmp(paramName, "scale_type")) {
            tempCfg.scaleType = (SENSOR_SCALE_TYPE) atoi(paramValue);
            received.scale_type = true;
        } else if (!strcmp(paramName, "scale_data_type")) {
            tempCfg.scaleDataType = (SENSOR_DATA_TYPE) atoi(paramValue);
            received.scale_data_type = true;
        } else if (!strcmp(paramName, "scale_value")) {
            tempCfg.scaleValue = atof(paramValue);
            received.scale_value = true;
        } else if (!strcmp(paramName, "offset_before")) {
            tempCfg.offsetPreVal = atof(paramValue);
            received.offset_before = true;
        } else if (!strcmp(paramName, "operator_1")) {
            tempCfg.offSetPreOperator = (OPERATOR) atoi(paramValue);
            received.op_1 = true;
        } else if (!strcmp(paramName, "operator_2")) {
            tempCfg.offsetSubOperator = (OPERATOR) atoi(paramValue);
            received.op_2 = true;
        } else if (!strcmp(paramName, "offset_after")) {
            tempCfg.offsetSubVal = atof(paramValue);
            received.offset_after = true;
        } else if (!strcmp(paramName, "enabled")) {
            if (!strcmp(paramValue, "on") || !strcmp(paramValue, "1")) {
                tempCfg.enable = true;
                received.enabled = true;
            }
        }
    }

    TCPIP_HTTP_NET_STATUS status = TCPIP_HTTP_NET_STAT_UPLOAD_ERROR;

    if (!bConfigFailure && received.id && (id < MAX_INPUT_CAPTURE)) {

        INPUT_CAPTURE_CHANNEL_CONFIG finalCfg = gInCaptureCfg.entry[id];

        if (received.name) strcpy(finalCfg.name, tempCfg.name);
        if (received.unit) strcpy(finalCfg.unit, tempCfg.unit);
        if (received.pulse_per_unit) finalCfg.valPerPulse = tempCfg.valPerPulse;
        if (received.scale_type) finalCfg.scaleType = tempCfg.scaleType;
        if (received.scale_data_type) finalCfg.scaleDataType = tempCfg.scaleDataType;
        if (received.scale_value) finalCfg.scaleValue = tempCfg.scaleValue;
        if (received.offset_before) finalCfg.offsetPreVal = tempCfg.offsetPreVal;
        if (received.op_1) finalCfg.offSetPreOperator = tempCfg.offSetPreOperator;
        if (received.op_2) finalCfg.offsetSubOperator = tempCfg.offsetSubOperator;
        if (received.offset_after) finalCfg.offsetSubVal = tempCfg.offsetSubVal;

        if (received.enabled) finalCfg.enable = tempCfg.enable;
        else finalCfg.enable = false;

        if (strlen(finalCfg.name) == 0)
            bConfigFailure = true;

        if (!bConfigFailure) {
            gInCaptureCfg.entry[id] = finalCfg;
            ExtFlash_SaveConfig(EXTFL_DATA_INCAPTURE_CFG, NULL);
            status = TCPIP_HTTP_NET_STAT_UPLOAD_OK;
        }
    }

    TCPIP_HTTP_NET_ConnectionStatusSet(connHandle, status);
    return TCPIP_HTTP_NET_IO_RES_DONE;
}

static TCPIP_HTTP_NET_IO_RESULT HTTPPostOutput(TCPIP_HTTP_NET_CONN_HANDLE connHandle) {
    bool bConfigFailure = false;
    uint8_t *httpDataBuff = 0;
    uint16_t httpBuffSize;
    uint32_t byteCount;

    uint8_t id = 0xFF;
    bool target_state = false;
    DO_CHANNEL_CONFIG tempCfg = {0};

    struct {
        bool id;
        bool name;
        bool description;
        bool mode;
        bool on_time;
        bool off_time;
        bool pulse_count;
        bool state;
    } received = {0};

    byteCount = TCPIP_HTTP_NET_ConnectionByteCountGet(connHandle);

    if (byteCount > TCPIP_HTTP_NET_ConnectionReadBufferSize(connHandle)) {
        TCPIP_HTTP_NET_ConnectionStatusSet(connHandle, TCPIP_HTTP_NET_STAT_REDIRECT);
        return TCPIP_HTTP_NET_IO_RES_DONE;
    }

    if (TCPIP_HTTP_NET_ConnectionReadIsReady(connHandle) < byteCount)
        return TCPIP_HTTP_NET_IO_RES_NEED_DATA;

    httpDataBuff = TCPIP_HTTP_NET_ConnectionDataBufferGet(connHandle);
    httpBuffSize = TCPIP_HTTP_NET_ConnectionDataBufferSizeGet(connHandle);

    while (TCPIP_HTTP_NET_ConnectionByteCountGet(connHandle)) {
        if (TCPIP_HTTP_NET_ConnectionPostNameRead(connHandle, httpDataBuff, 32) != TCPIP_HTTP_NET_READ_OK ||
                TCPIP_HTTP_NET_ConnectionPostValueRead(connHandle, httpDataBuff + 32, httpBuffSize - 32 - 2) != TCPIP_HTTP_NET_READ_OK) {
            bConfigFailure = true;
            break;
        }

        char *paramName = (char *) httpDataBuff;
        char *paramValue = (char *) (httpDataBuff + 32);

        if (!strcmp(paramName, "do_id")) {
            id = atoi(paramValue);
            received.id = true;
        } else if (!strcmp(paramName, "output_name")) {
            strncpy(tempCfg.name, paramValue, SENSOR_NAME_LEN - 1);
            tempCfg.name[SENSOR_NAME_LEN - 1] = '\0';
            received.name = true;
        } else if (!strcmp(paramName, "description")) {
            strncpy(tempCfg.describe, paramValue, SENSOR_NAME_LEN - 1);
            tempCfg.describe[SENSOR_NAME_LEN - 1] = '\0';
            received.description = true;
        } else if (!strcmp(paramName, "mode")) {
            tempCfg.mode = (CTRL_MODE_TYPE) atoi(paramValue);
            received.mode = true;
        } else if (!strcmp(paramName, "on_time")) {
            tempCfg.ontime = (uint16_t) atoi(paramValue);
            received.on_time = true;
        } else if (!strcmp(paramName, "off_time")) {
            tempCfg.offtime = (uint16_t) atoi(paramValue);
            received.off_time = true;
        } else if (!strcmp(paramName, "pulse_count")) {
            tempCfg.pulseCount = (uint16_t) atoi(paramValue);
            received.pulse_count = true;
        } else if (!strcmp(paramName, "state")) { // B? sung x? lý field "state"
            target_state = (atoi(paramValue) > 0);
            received.state = true;
        }
    }

    TCPIP_HTTP_NET_STATUS status = TCPIP_HTTP_NET_STAT_UPLOAD_ERROR;

    if (!bConfigFailure && received.id && (id < MAX_DIGITAL_OUTPUT)) {
        DO_CHANNEL_CONFIG finalCfg = gAppCfg.io.out[id];

        if (received.name) strcpy(finalCfg.name, tempCfg.name);
        if (received.description) strcpy(finalCfg.describe, tempCfg.describe);
        if (received.mode) finalCfg.mode = tempCfg.mode;
        if (received.on_time) finalCfg.ontime = tempCfg.ontime;
        if (received.off_time) finalCfg.offtime = tempCfg.offtime;
        if (received.pulse_count) finalCfg.pulseCount = tempCfg.pulseCount;

        if (strlen(finalCfg.name) == 0)
            bConfigFailure = true;

        if (!bConfigFailure) {
            gAppCfg.io.out[id] = finalCfg;
            ExtFlash_SaveConfig(EXTFL_DATA_APP_CFG, NULL);

            if (received.state)
                DigitalOutput_Set(id, target_state);

            status = TCPIP_HTTP_NET_STAT_UPLOAD_OK;
        }
    }

    TCPIP_HTTP_NET_ConnectionStatusSet(connHandle, status);

    return TCPIP_HTTP_NET_IO_RES_DONE;
}

static TCPIP_HTTP_NET_IO_RESULT HTTPPostUser(TCPIP_HTTP_NET_CONN_HANDLE connHandle) {
    bool bConfigFailure = false;
    uint8_t *httpDataBuff = 0;
    uint16_t httpBuffSize;
    uint32_t byteCount;

    byteCount = TCPIP_HTTP_NET_ConnectionByteCountGet(connHandle);

    if (byteCount > TCPIP_HTTP_NET_ConnectionReadBufferSize(connHandle)) {
        TCPIP_HTTP_NET_ConnectionStatusSet(connHandle, TCPIP_HTTP_NET_STAT_REDIRECT);
        return TCPIP_HTTP_NET_IO_RES_DONE;
    }

    if (TCPIP_HTTP_NET_ConnectionReadIsReady(connHandle) < byteCount)
        return TCPIP_HTTP_NET_IO_RES_NEED_DATA;

    httpDataBuff = TCPIP_HTTP_NET_ConnectionDataBufferGet(connHandle);
    httpBuffSize = TCPIP_HTTP_NET_ConnectionDataBufferSizeGet(connHandle);

    while (TCPIP_HTTP_NET_ConnectionByteCountGet(connHandle)) {
        if (TCPIP_HTTP_NET_ConnectionPostNameRead(connHandle, httpDataBuff, 32) != TCPIP_HTTP_NET_READ_OK) {
            bConfigFailure = true;
            break;
        }

        if (TCPIP_HTTP_NET_ConnectionPostValueRead(connHandle, httpDataBuff + 32, httpBuffSize - 32 - 2) != TCPIP_HTTP_NET_READ_OK) {
            bConfigFailure = true;
            break;
        }

    }

    TCPIP_HTTP_NET_STATUS status;
    if (bConfigFailure == false) {
        status = TCPIP_HTTP_NET_STAT_UPLOAD_OK;
    } else
        status = TCPIP_HTTP_NET_STAT_UPLOAD_ERROR;

    TCPIP_HTTP_NET_ConnectionStatusSet(connHandle, status);

    return TCPIP_HTTP_NET_IO_RES_DONE;
}

static TCPIP_HTTP_NET_IO_RESULT HTTPPostTime(TCPIP_HTTP_NET_CONN_HANDLE connHandle) {
    bool bConfigFailure = false;
    uint8_t *httpDataBuff = 0;
    uint16_t httpBuffSize;
    uint32_t byteCount;

    char actionStr[20] = {0};
    DATETIME_CONFIG tempTimeCfg = {0};
    TIME tempManualTime = {0};

    struct {
        bool action;
        bool timezone;
        bool ntp_enable;
        bool ntp_server_1;
        bool ntp_server_2;
        bool ntp_sync_interval;
        bool ntp_port;
        bool manual_date;
        bool manual_time;
    } received = {0};

    byteCount = TCPIP_HTTP_NET_ConnectionByteCountGet(connHandle);

    if (byteCount > TCPIP_HTTP_NET_ConnectionReadBufferSize(connHandle)) {
        TCPIP_HTTP_NET_ConnectionStatusSet(connHandle, TCPIP_HTTP_NET_STAT_REDIRECT);
        return TCPIP_HTTP_NET_IO_RES_DONE;
    }

    if (TCPIP_HTTP_NET_ConnectionReadIsReady(connHandle) < byteCount)
        return TCPIP_HTTP_NET_IO_RES_NEED_DATA;

    httpDataBuff = TCPIP_HTTP_NET_ConnectionDataBufferGet(connHandle);
    httpBuffSize = TCPIP_HTTP_NET_ConnectionDataBufferSizeGet(connHandle);

    while (TCPIP_HTTP_NET_ConnectionByteCountGet(connHandle)) {
        if (TCPIP_HTTP_NET_ConnectionPostNameRead(connHandle, httpDataBuff, 32) != TCPIP_HTTP_NET_READ_OK ||
                TCPIP_HTTP_NET_ConnectionPostValueRead(connHandle, httpDataBuff + 32, httpBuffSize - 32 - 2) != TCPIP_HTTP_NET_READ_OK) {
            bConfigFailure = true;
            break;
        }

        char *paramName = (char *) httpDataBuff;
        char *paramValue = (char *) (httpDataBuff + 32);

        if (!strcmp(paramName, "action")) {
            strncpy(actionStr, paramValue, sizeof (actionStr) - 1);
            received.action = true;
        } else if (!strcmp(paramName, "timezone")) {
            tempTimeCfg.timeZone = (int8_t) atoi(paramValue);
            received.timezone = true;
        } else if (!strcmp(paramName, "ntp_enable")) {
            tempTimeCfg.syncNtpEnable = (!strcmp(paramValue, "on") || !strcmp(paramValue, "1"));
            received.ntp_enable = true;
        } else if (!strcmp(paramName, "ntp_server_1")) {
            strncpy(tempTimeCfg.ntpServerPrimary, paramValue, URL_LEN - 1);
            tempTimeCfg.ntpServerPrimary[URL_LEN - 1] = '\0';
            received.ntp_server_1 = true;
        } else if (!strcmp(paramName, "ntp_server_2")) {
            strncpy(tempTimeCfg.ntpServerBackup, paramValue, URL_LEN - 1);
            tempTimeCfg.ntpServerBackup[URL_LEN - 1] = '\0';
            received.ntp_server_2 = true;
        } else if (!strcmp(paramName, "ntp_sync_interval")) {
            tempTimeCfg.syncInterval = (uint32_t) atol(paramValue);
            received.ntp_sync_interval = true;
        } else if (!strcmp(paramName, "ntp_port")) {
            tempTimeCfg.ntpPort = (uint16_t) atoi(paramValue);
            received.ntp_port = true;
        } else if (!strcmp(paramName, "manual_date")) {
            int y, m, d;
            if (sscanf(paramValue, "%d-%d-%d", &y, &m, &d) == 3) {
                tempManualTime.year = (uint16_t) y;
                tempManualTime.month = (uint8_t) m;
                tempManualTime.day = (uint8_t) d;
                received.manual_date = true;
            }
        } else if (!strcmp(paramName, "manual_time")) {
            int hh, mm, ss;
            if (sscanf(paramValue, "%d:%d:%d", &hh, &mm, &ss) == 3) {
                tempManualTime.hour = (uint8_t) hh;
                tempManualTime.minute = (uint8_t) mm;
                tempManualTime.second = (uint8_t) ss;
                received.manual_time = true;
            }
        }
    }

    TCPIP_HTTP_NET_STATUS status = TCPIP_HTTP_NET_STAT_UPLOAD_ERROR;

    if (!bConfigFailure && received.action) {
        bool configChanged = false;

        if (!strcmp(actionStr, "timezone")) {
            if (received.timezone) {
                gAppCfg.time.timeZone = tempTimeCfg.timeZone;
                configChanged = true;
            }
        } else if (!strcmp(actionStr, "ntp_config")) {
            if (received.ntp_enable) gAppCfg.time.syncNtpEnable = tempTimeCfg.syncNtpEnable;
            if (received.ntp_server_1) strcpy(gAppCfg.time.ntpServerPrimary, tempTimeCfg.ntpServerPrimary);
            if (received.ntp_server_2) strcpy(gAppCfg.time.ntpServerBackup, tempTimeCfg.ntpServerBackup);
            if (received.ntp_sync_interval) gAppCfg.time.syncInterval = tempTimeCfg.syncInterval;
            if (received.ntp_port) gAppCfg.time.ntpPort = tempTimeCfg.ntpPort;

            configChanged = true;
        } else if (!strcmp(actionStr, "manual_time")) {
            if (received.manual_date && received.manual_time) {
                TIME readyToSetTime = tempManualTime;
                Rtc_updateFromManual(&readyToSetTime);
            }
        } else if (!strcmp(actionStr, "sync_ntp_now")) {
            if (gAppCfg.network.uplink == UPLINK_GSM)
                SIMMain_NTPTrigger();
            if (gAppCfg.network.uplink == UPLINK_ALL ||
                    gAppCfg.network.uplink == UPLINK_ETH)
                EthNtp_TriggerUpdate();
        }

        if (configChanged)
            ExtFlash_SaveConfig(EXTFL_DATA_APP_CFG, NULL);

        status = TCPIP_HTTP_NET_STAT_UPLOAD_OK;
    }

    TCPIP_HTTP_NET_ConnectionStatusSet(connHandle, status);

    return TCPIP_HTTP_NET_IO_RES_DONE;
}

static TCPIP_HTTP_NET_IO_RESULT HTTPPostSerialCom(TCPIP_HTTP_NET_CONN_HANDLE connHandle) {
    bool bConfigFailure = false;
    uint8_t *httpDataBuff = 0;
    uint16_t httpBuffSize;
    uint32_t byteCount;

    byteCount = TCPIP_HTTP_NET_ConnectionByteCountGet(connHandle);

    if (byteCount > TCPIP_HTTP_NET_ConnectionReadBufferSize(connHandle)) {
        TCPIP_HTTP_NET_ConnectionStatusSet(connHandle, TCPIP_HTTP_NET_STAT_REDIRECT);
        return TCPIP_HTTP_NET_IO_RES_DONE;
    }

    if (TCPIP_HTTP_NET_ConnectionReadIsReady(connHandle) < byteCount)
        return TCPIP_HTTP_NET_IO_RES_NEED_DATA;

    httpDataBuff = TCPIP_HTTP_NET_ConnectionDataBufferGet(connHandle);
    httpBuffSize = TCPIP_HTTP_NET_ConnectionDataBufferSizeGet(connHandle);

    while (TCPIP_HTTP_NET_ConnectionByteCountGet(connHandle)) {
        if (TCPIP_HTTP_NET_ConnectionPostNameRead(connHandle, httpDataBuff, 32) != TCPIP_HTTP_NET_READ_OK) {
            bConfigFailure = true;
            break;
        }

        if (TCPIP_HTTP_NET_ConnectionPostValueRead(connHandle, httpDataBuff + 32, httpBuffSize - 32 - 2) != TCPIP_HTTP_NET_READ_OK) {
            bConfigFailure = true;
            break;
        }

    }

    TCPIP_HTTP_NET_STATUS status;
    if (bConfigFailure == false) {
        status = TCPIP_HTTP_NET_STAT_UPLOAD_OK;
    } else
        status = TCPIP_HTTP_NET_STAT_UPLOAD_ERROR;

    TCPIP_HTTP_NET_ConnectionStatusSet(connHandle, status);

    return TCPIP_HTTP_NET_IO_RES_DONE;
}

static TCPIP_HTTP_NET_IO_RESULT HTTPPostNetwork(TCPIP_HTTP_NET_CONN_HANDLE connHandle) {
    bool bConfigFailure = false;
    uint8_t *httpDataBuff = 0;
    uint16_t httpBuffSize;
    uint32_t byteCount;

    NETWORK_CONFIG tempNetCfg = {0};

    struct {
        bool uplink_mode;
        bool ip_mode;
        bool app_uname;
        bool app_password;
        bool ip_address;
        bool subnet_mask;
        bool gateway;
        bool dns_primary;
        bool dns_secondary;
        bool hostname;
    } received = {0};

    byteCount = TCPIP_HTTP_NET_ConnectionByteCountGet(connHandle);

    if (byteCount > TCPIP_HTTP_NET_ConnectionReadBufferSize(connHandle)) {
        TCPIP_HTTP_NET_ConnectionStatusSet(connHandle, TCPIP_HTTP_NET_STAT_REDIRECT);
        return TCPIP_HTTP_NET_IO_RES_DONE;
    }

    if (TCPIP_HTTP_NET_ConnectionReadIsReady(connHandle) < byteCount)
        return TCPIP_HTTP_NET_IO_RES_NEED_DATA;

    httpDataBuff = TCPIP_HTTP_NET_ConnectionDataBufferGet(connHandle);
    httpBuffSize = TCPIP_HTTP_NET_ConnectionDataBufferSizeGet(connHandle);

    while (TCPIP_HTTP_NET_ConnectionByteCountGet(connHandle)) {
        if (TCPIP_HTTP_NET_ConnectionPostNameRead(connHandle, httpDataBuff, 32) != TCPIP_HTTP_NET_READ_OK ||
                TCPIP_HTTP_NET_ConnectionPostValueRead(connHandle, httpDataBuff + 32, httpBuffSize - 32 - 2) != TCPIP_HTTP_NET_READ_OK) {
            bConfigFailure = true;
            break;
        }

        char *paramName = (char *) httpDataBuff;
        char *paramValue = (char *) (httpDataBuff + 32);

        if (!strcmp(paramName, "uplink_mode")) {
            tempNetCfg.uplink = (INTERNET_UPLINK) atoi(paramValue);
            received.uplink_mode = true;
        } else if (!strcmp(paramName, "ip_mode")) {
            tempNetCfg.isDHCPEn = (!strcmp(paramValue, "dhcp"));
            received.ip_mode = true;
        } else if (!strcmp(paramName, "app_uname")) {
            strncpy(tempNetCfg.deviceUsername, paramValue, USERNAME_LEN - 1);
            tempNetCfg.deviceUsername[USERNAME_LEN - 1] = '\0';
            received.app_uname = true;
        } else if (!strcmp(paramName, "app_password")) {
            // Ch? c?p nh?t m?t kh?u n?u khác r?ng và không ph?i chu?i "......" (?n pass t? web)
            if (strlen(paramValue) > 0 && strcmp(paramValue, "......") != 0) {
                strncpy(tempNetCfg.devicePassword, paramValue, PASSWORD_LEN - 1);
                tempNetCfg.devicePassword[PASSWORD_LEN - 1] = '\0';
            }
            received.app_password = true;
        } else if (!strcmp(paramName, "ip_address")) {
            TCPIP_Helper_StringToIPAddress(paramValue, (IPV4_ADDR *) & tempNetCfg.ipAddr);
            received.ip_address = true;
        } else if (!strcmp(paramName, "subnet_mask")) {
            TCPIP_Helper_StringToIPAddress(paramValue, (IPV4_ADDR *) & tempNetCfg.ipMask);
            received.subnet_mask = true;
        } else if (!strcmp(paramName, "gateway")) {
            TCPIP_Helper_StringToIPAddress(paramValue, (IPV4_ADDR *) & tempNetCfg.gateway);
            received.gateway = true;
        } else if (!strcmp(paramName, "dns_primary")) {
            TCPIP_Helper_StringToIPAddress(paramValue, (IPV4_ADDR *) & tempNetCfg.primaryDNS);
            received.dns_primary = true;
        } else if (!strcmp(paramName, "dns_secondary")) {
            TCPIP_Helper_StringToIPAddress(paramValue, (IPV4_ADDR *) & tempNetCfg.secondDNS);
            received.dns_secondary = true;
        } else if (!strcmp(paramName, "hostname")) {
            strncpy(tempNetCfg.netBIOSName, paramValue, BIOS_NAME_LEN - 1);
            tempNetCfg.netBIOSName[BIOS_NAME_LEN - 1] = '\0';
            received.hostname = true;
        }
    }

    TCPIP_HTTP_NET_STATUS status = TCPIP_HTTP_NET_STAT_UPLOAD_ERROR;

    if (!bConfigFailure) {
        if (received.uplink_mode) gAppCfg.network.uplink = tempNetCfg.uplink;
        if (received.ip_mode) gAppCfg.network.isDHCPEn = tempNetCfg.isDHCPEn;
        if (received.app_uname) strcpy(gAppCfg.network.deviceUsername, tempNetCfg.deviceUsername);
        if (received.app_password && strlen(tempNetCfg.devicePassword) > 0) {
            strcpy(gAppCfg.network.devicePassword, tempNetCfg.devicePassword);
        }
        if (received.ip_address) gAppCfg.network.ipAddr = tempNetCfg.ipAddr;
        if (received.subnet_mask) gAppCfg.network.ipMask = tempNetCfg.ipMask;
        if (received.gateway) gAppCfg.network.gateway = tempNetCfg.gateway;
        if (received.dns_primary) gAppCfg.network.primaryDNS = tempNetCfg.primaryDNS;
        if (received.dns_secondary) gAppCfg.network.secondDNS = tempNetCfg.secondDNS;
        if (received.hostname) strcpy(gAppCfg.network.netBIOSName, tempNetCfg.netBIOSName);

        ExtFlash_SaveConfig(EXTFL_DATA_APP_CFG, NULL);

        status = TCPIP_HTTP_NET_STAT_UPLOAD_OK;
    }

    TCPIP_HTTP_NET_ConnectionStatusSet(connHandle, status);

    return TCPIP_HTTP_NET_IO_RES_DONE;
}

static TCPIP_HTTP_NET_IO_RESULT HTTPPostSim(TCPIP_HTTP_NET_CONN_HANDLE connHandle) {
    bool bConfigFailure = false;
    uint8_t *httpDataBuff = 0;
    uint16_t httpBuffSize;
    uint32_t byteCount;

    GSM_CONFIG tempGsmCfg = {0};
    int sim_slot_val = 0;

    struct {
        bool sim_slot;
        bool apn;
        bool username;
        bool password;
    } received = {0};

    byteCount = TCPIP_HTTP_NET_ConnectionByteCountGet(connHandle);

    if (byteCount > TCPIP_HTTP_NET_ConnectionReadBufferSize(connHandle)) {
        TCPIP_HTTP_NET_ConnectionStatusSet(connHandle, TCPIP_HTTP_NET_STAT_REDIRECT);
        return TCPIP_HTTP_NET_IO_RES_DONE;
    }

    if (TCPIP_HTTP_NET_ConnectionReadIsReady(connHandle) < byteCount)
        return TCPIP_HTTP_NET_IO_RES_NEED_DATA;

    httpDataBuff = TCPIP_HTTP_NET_ConnectionDataBufferGet(connHandle);
    httpBuffSize = TCPIP_HTTP_NET_ConnectionDataBufferSizeGet(connHandle);

    while (TCPIP_HTTP_NET_ConnectionByteCountGet(connHandle)) {
        if (TCPIP_HTTP_NET_ConnectionPostNameRead(connHandle, httpDataBuff, 32) != TCPIP_HTTP_NET_READ_OK ||
                TCPIP_HTTP_NET_ConnectionPostValueRead(connHandle, httpDataBuff + 32, httpBuffSize - 32 - 2) != TCPIP_HTTP_NET_READ_OK) {
            bConfigFailure = true;
            break;
        }

        char *paramName = (char *) httpDataBuff;
        char *paramValue = (char *) (httpDataBuff + 32);

        if (!strcmp(paramName, "sim_slot")) {
            sim_slot_val = atoi(paramValue);
            received.sim_slot = true;
        } else if (!strcmp(paramName, "apn")) {
            strncpy(tempGsmCfg.APN, paramValue, APN_LEN - 1);
            tempGsmCfg.APN[APN_LEN - 1] = '\0';
            received.apn = true;
        } else if (!strcmp(paramName, "username")) {
            strncpy(tempGsmCfg.usernameAPN, paramValue, USERNAME_LEN - 1);
            tempGsmCfg.usernameAPN[USERNAME_LEN - 1] = '\0';
            received.username = true;
        } else if (!strcmp(paramName, "password")) {
            if (strlen(paramValue) > 0 && strcmp(paramValue, "......") != 0) {
                strncpy(tempGsmCfg.passwordAPN, paramValue, PASSWORD_LEN - 1);
                tempGsmCfg.passwordAPN[PASSWORD_LEN - 1] = '\0';
            }
            received.password = true;
        }
    }

    TCPIP_HTTP_NET_STATUS status = TCPIP_HTTP_NET_STAT_UPLOAD_ERROR;

    if (!bConfigFailure) {
        if (received.apn) strcpy(gAppCfg.gsm.APN, tempGsmCfg.APN);
        if (received.username) strcpy(gAppCfg.gsm.usernameAPN, tempGsmCfg.usernameAPN);
        if (received.password && strlen(tempGsmCfg.passwordAPN) > 0) {
            strcpy(gAppCfg.gsm.passwordAPN, tempGsmCfg.passwordAPN);
        }

        ExtFlash_SaveConfig(EXTFL_DATA_APP_CFG, NULL);

        status = TCPIP_HTTP_NET_STAT_UPLOAD_OK;
    }

    TCPIP_HTTP_NET_ConnectionStatusSet(connHandle, status);

    return TCPIP_HTTP_NET_IO_RES_DONE;
}

static TCPIP_HTTP_NET_IO_RESULT HTTPPostFtp(TCPIP_HTTP_NET_CONN_HANDLE connHandle) {
    bool bConfigFailure = false;
    uint8_t *httpDataBuff = 0;
    uint16_t httpBuffSize;
    uint32_t byteCount;

    FTP_SERVER_CONFIG tempFtpCfg = {0};
    int ftp_id = -1;

    struct {
        bool ftp_id;
        bool ftp_enable;
        bool ftp_host;
        bool ftp_port;
        bool ftp_username;
        bool ftp_password;
        bool ftp_remote_path;
        bool folder_structure;
    } received = {0};

    byteCount = TCPIP_HTTP_NET_ConnectionByteCountGet(connHandle);

    if (byteCount > TCPIP_HTTP_NET_ConnectionReadBufferSize(connHandle)) {
        TCPIP_HTTP_NET_ConnectionStatusSet(connHandle, TCPIP_HTTP_NET_STAT_REDIRECT);
        return TCPIP_HTTP_NET_IO_RES_DONE;
    }

    if (TCPIP_HTTP_NET_ConnectionReadIsReady(connHandle) < byteCount)
        return TCPIP_HTTP_NET_IO_RES_NEED_DATA;

    httpDataBuff = TCPIP_HTTP_NET_ConnectionDataBufferGet(connHandle);
    httpBuffSize = TCPIP_HTTP_NET_ConnectionDataBufferSizeGet(connHandle);

    while (TCPIP_HTTP_NET_ConnectionByteCountGet(connHandle)) {
        if (TCPIP_HTTP_NET_ConnectionPostNameRead(connHandle, httpDataBuff, 32) != TCPIP_HTTP_NET_READ_OK ||
                TCPIP_HTTP_NET_ConnectionPostValueRead(connHandle, httpDataBuff + 32, httpBuffSize - 32 - 2) != TCPIP_HTTP_NET_READ_OK) {
            bConfigFailure = true;
            break;
        }

        char *paramName = (char *) httpDataBuff;
        char *paramValue = (char *) (httpDataBuff + 32);

        if (!strcmp(paramName, "ftp_id")) {
            ftp_id = atoi(paramValue);
            received.ftp_id = true;
        } else if (!strcmp(paramName, "ftp_enable")) {
            tempFtpCfg.enable = (!strcmp(paramValue, "true") || !strcmp(paramValue, "1"));
            received.ftp_enable = true;
        } else if (!strcmp(paramName, "ftp_host")) {
            strncpy(tempFtpCfg.hostname, paramValue, URL_LEN - 1);
            tempFtpCfg.hostname[URL_LEN - 1] = '\0';
            received.ftp_host = true;
        } else if (!strcmp(paramName, "ftp_port")) {
            tempFtpCfg.port = (uint16_t) atoi(paramValue);
            received.ftp_port = true;
        } else if (!strcmp(paramName, "ftp_username")) {
            strncpy(tempFtpCfg.username, paramValue, USERNAME_LEN - 1);
            tempFtpCfg.username[USERNAME_LEN - 1] = '\0';
            received.ftp_username = true;
        } else if (!strcmp(paramName, "ftp_password")) {
            if (strlen(paramValue) > 0 && strcmp(paramValue, "......") != 0) {
                strncpy(tempFtpCfg.password, paramValue, PASSWORD_LEN - 1);
                tempFtpCfg.password[PASSWORD_LEN - 1] = '\0';
            }
            received.ftp_password = true;
        } else if (!strcmp(paramName, "ftp_remote_path")) {
            strncpy(tempFtpCfg.dirPath, paramValue, DIR_PATH_LEN - 1);
            tempFtpCfg.dirPath[DIR_PATH_LEN - 1] = '\0';
            received.ftp_remote_path = true;
        } else if (!strcmp(paramName, "folder_structure")) {
            tempFtpCfg.makeFolder = (MAKE_FOLDER) atoi(paramValue);
            received.folder_structure = true;
        }
    }

    TCPIP_HTTP_NET_STATUS status = TCPIP_HTTP_NET_STAT_UPLOAD_ERROR;

    if (!bConfigFailure && received.ftp_id && ftp_id >= 1 && ftp_id <= MAX_FTP_SERVER) {

        int index = ftp_id - 1;
        if (index >= 0) {
            if (received.ftp_enable) gAppCfg.ftpServer[index].enable = tempFtpCfg.enable;
            else gAppCfg.ftpServer[index].enable = false;
            if (received.ftp_host) strcpy(gAppCfg.ftpServer[index].hostname, tempFtpCfg.hostname);
            if (received.ftp_port) gAppCfg.ftpServer[index].port = tempFtpCfg.port;
            if (received.ftp_username) strcpy(gAppCfg.ftpServer[index].username, tempFtpCfg.username);

            if (received.ftp_password && strlen(tempFtpCfg.password) > 0) {
                strcpy(gAppCfg.ftpServer[index].password, tempFtpCfg.password);
            }

            if (received.ftp_remote_path) strcpy(gAppCfg.ftpServer[index].dirPath, tempFtpCfg.dirPath);
            if (received.folder_structure) gAppCfg.ftpServer[index].makeFolder = tempFtpCfg.makeFolder;

            ExtFlash_SaveConfig(EXTFL_DATA_APP_CFG, NULL);
            status = TCPIP_HTTP_NET_STAT_UPLOAD_OK;
        }
    }

    TCPIP_HTTP_NET_ConnectionStatusSet(connHandle, status);

    return TCPIP_HTTP_NET_IO_RES_DONE;
}

static TCPIP_HTTP_NET_IO_RESULT HTTPPostMqtt(TCPIP_HTTP_NET_CONN_HANDLE connHandle) {
    bool bConfigFailure = false;
    uint8_t *httpDataBuff = 0;
    uint16_t httpBuffSize;
    uint32_t byteCount;

    byteCount = TCPIP_HTTP_NET_ConnectionByteCountGet(connHandle);

    if (byteCount > TCPIP_HTTP_NET_ConnectionReadBufferSize(connHandle)) {
        TCPIP_HTTP_NET_ConnectionStatusSet(connHandle, TCPIP_HTTP_NET_STAT_REDIRECT);
        return TCPIP_HTTP_NET_IO_RES_DONE;
    }

    if (TCPIP_HTTP_NET_ConnectionReadIsReady(connHandle) < byteCount)
        return TCPIP_HTTP_NET_IO_RES_NEED_DATA;

    httpDataBuff = TCPIP_HTTP_NET_ConnectionDataBufferGet(connHandle);
    httpBuffSize = TCPIP_HTTP_NET_ConnectionDataBufferSizeGet(connHandle);

    while (TCPIP_HTTP_NET_ConnectionByteCountGet(connHandle)) {
        if (TCPIP_HTTP_NET_ConnectionPostNameRead(connHandle, httpDataBuff, 32) != TCPIP_HTTP_NET_READ_OK) {
            bConfigFailure = true;
            break;
        }

        if (TCPIP_HTTP_NET_ConnectionPostValueRead(connHandle, httpDataBuff + 32, httpBuffSize - 32 - 2) != TCPIP_HTTP_NET_READ_OK) {
            bConfigFailure = true;
            break;
        }

    }

    TCPIP_HTTP_NET_STATUS status;
    if (bConfigFailure == false) {
        status = TCPIP_HTTP_NET_STAT_UPLOAD_OK;
    } else
        status = TCPIP_HTTP_NET_STAT_UPLOAD_ERROR;

    TCPIP_HTTP_NET_ConnectionStatusSet(connHandle, status);

    return TCPIP_HTTP_NET_IO_RES_DONE;
}

static TCPIP_HTTP_NET_IO_RESULT HTTPPostStorage(TCPIP_HTTP_NET_CONN_HANDLE connHandle) {
    bool bConfigFailure = false;
    uint8_t *httpDataBuff = 0;
    uint16_t httpBuffSize;
    uint32_t byteCount;

    byteCount = TCPIP_HTTP_NET_ConnectionByteCountGet(connHandle);

    if (byteCount > TCPIP_HTTP_NET_ConnectionReadBufferSize(connHandle)) {
        TCPIP_HTTP_NET_ConnectionStatusSet(connHandle, TCPIP_HTTP_NET_STAT_REDIRECT);
        return TCPIP_HTTP_NET_IO_RES_DONE;
    }

    if (TCPIP_HTTP_NET_ConnectionReadIsReady(connHandle) < byteCount)
        return TCPIP_HTTP_NET_IO_RES_NEED_DATA;

    httpDataBuff = TCPIP_HTTP_NET_ConnectionDataBufferGet(connHandle);
    httpBuffSize = TCPIP_HTTP_NET_ConnectionDataBufferSizeGet(connHandle);

    while (TCPIP_HTTP_NET_ConnectionByteCountGet(connHandle)) {
        if (TCPIP_HTTP_NET_ConnectionPostNameRead(connHandle, httpDataBuff, 32) != TCPIP_HTTP_NET_READ_OK) {
            bConfigFailure = true;
            break;
        }

        if (TCPIP_HTTP_NET_ConnectionPostValueRead(connHandle, httpDataBuff + 32, httpBuffSize - 32 - 2) != TCPIP_HTTP_NET_READ_OK) {
            bConfigFailure = true;
            break;
        }

    }

    TCPIP_HTTP_NET_STATUS status;
    if (bConfigFailure == false) {
        status = TCPIP_HTTP_NET_STAT_UPLOAD_OK;
    } else
        status = TCPIP_HTTP_NET_STAT_UPLOAD_ERROR;

    TCPIP_HTTP_NET_ConnectionStatusSet(connHandle, status);

    return TCPIP_HTTP_NET_IO_RES_DONE;
}
#endif // #if defined(TCPIP_HTTP_NET_USE_POST)

/**********************************************************
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 Authencication
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 **********************************************************/

#if defined(TCPIP_HTTP_NET_USE_AUTHENTICATION)

uint8_t TCPIP_HTTP_NET_ConnectionFileAuthenticate(TCPIP_HTTP_NET_CONN_HANDLE connHandle, const char *cFile, const TCPIP_HTTP_NET_USER_CALLBACK * pCBack) {

    // You can match additional strings here to password protect other files.
    // You could switch this and exclude files from authentication.
    // You could also always return 0x00 to require auth for all files.
    // You can return different values (0x00 to 0x79) to track "realms" for below.

    return 0x00;
}
#endif


#if defined(TCPIP_HTTP_NET_USE_AUTHENTICATION)

uint8_t TCPIP_HTTP_NET_ConnectionUserAuthenticate(TCPIP_HTTP_NET_CONN_HANDLE connHandle, const char *cUser, const char *cPass, const TCPIP_HTTP_NET_USER_CALLBACK * pCBack) {
    if (strcmp(cUser, (const char *) "admin") == 0
            && strcmp(cPass, (const char *) "admin") == 0)

        return 0x80; // accept

    return 0x00; // Provided user/pass is invalid
}
#endif

/****************************************************************************
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
  Section:
    Dynamic Variable Callback Functions
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 ****************************************************************************/
TCPIP_HTTP_DYN_PRINT_RES TCPIP_HTTP_Print_analog(TCPIP_HTTP_NET_CONN_HANDLE connHandle, const TCPIP_HTTP_DYN_VAR_DCPT * vDcpt) {
    HTTP_APP_DYNVAR_BUFFER *pBuffer = HTTP_APP_GetDynamicBuffer();

    if (pBuffer == 0)
        return TCPIP_HTTP_DYN_PRINT_RES_AGAIN;

    uint32_t step = (uint32_t) (uintptr_t) TCPIP_HTTP_NET_ConnectionUserDataGet(connHandle);
    char *ptr = (char *) pBuffer->data;
    size_t spaceLeft = pBuffer->bufferSize;
    int written = 0;

    if (step == 0) {
        written = snprintf(ptr, spaceLeft, "{");
        TCPIP_HTTP_NET_DynamicWrite(vDcpt, pBuffer->data, written, true);

        TCPIP_HTTP_NET_ConnectionUserDataSet(connHandle, (const void*) (uintptr_t) 1);
        return TCPIP_HTTP_DYN_PRINT_RES_PROCESS_AGAIN;
    }

    uint32_t chIndex = step - 1;

    if (chIndex < MAX_ANALOG_CHANNEL) {
        written = snprintf(ptr, spaceLeft,
                "\"%lu\": {\"name\": \"%s\",\"value\": %.10g,\"unit\": \"%s\",\"enable\": %s,\"signal\": %d,",
                (unsigned long) (chIndex + 1),
                gAnalogCfg.entry[chIndex].name,
                adcDt.entry[chIndex].value,
                gAnalogCfg.entry[chIndex].unit,
                gAnalogCfg.entry[chIndex].enable ? "true" : "false",
                (int) gAnalogCfg.entry[chIndex].adcType
                );
        ptr += written;
        spaceLeft -= written;

        written = snprintf(ptr, spaceLeft,
                "\"rawMin\": %.10g,\"rawMax\": %.10g,\"euMin\": %.10g,\"euMax\": %.10g,",
                gAnalogCfg.entry[chIndex].inputLow,
                gAnalogCfg.entry[chIndex].inputHigh,
                gAnalogCfg.entry[chIndex].outputLow,
                gAnalogCfg.entry[chIndex].outputHigh
                );
        ptr += written;
        spaceLeft -= written;

        char rawInput[24];
        snprintf(rawInput, sizeof (rawInput), "%.5g mA", adcDt.entry[chIndex].rawValue);
        written = snprintf(ptr, spaceLeft,
                "\"status\": %d,\"sampleRate\": %d,\"rawInput\": \"%s\",\"min24\": %.10g,\"max24\": %.10g,",
                adcDt.entry[chIndex].status,
                500,
                rawInput,
                22.1, // Gi? s? min24
                31.7 // Gi? s? max24
                );
        ptr += written;
        spaceLeft -= written;

        written = snprintf(ptr, spaceLeft,
                "\"scaleType\": %d,\"scaleDataType\": %d,\"scaleValue\": %.10g,",
                (int) gAnalogCfg.entry[chIndex].scaleType,
                (int) gAnalogCfg.entry[chIndex].scaleDataType,
                gAnalogCfg.entry[chIndex].scaleValue
                );
        ptr += written;
        spaceLeft -= written;

        written = snprintf(ptr, spaceLeft,
                "\"offsetBefore\": %.10g,\"operator1\": %d,\"offsetAfter\": %.10g,\"operator2\": %d}%s",
                gAnalogCfg.entry[chIndex].offsetPreVal,
                (int) gAnalogCfg.entry[chIndex].offSetPreOperator,
                gAnalogCfg.entry[chIndex].offsetSubVal,
                (int) gAnalogCfg.entry[chIndex].offsetSubOperator,
                (chIndex == MAX_ANALOG_CHANNEL - 1) ? "" : ","
                );
        ptr += written;
        spaceLeft -= written;

        TCPIP_HTTP_NET_DynamicWrite(vDcpt, pBuffer->data, (ptr - (char *) pBuffer->data), true);
        step++;

        TCPIP_HTTP_NET_ConnectionUserDataSet(connHandle, (const void*) (uintptr_t) step);
        return TCPIP_HTTP_DYN_PRINT_RES_PROCESS_AGAIN;
    }

    if (chIndex >= MAX_ANALOG_CHANNEL) {
        written = snprintf(ptr, spaceLeft, "}");
        TCPIP_HTTP_NET_DynamicWrite(vDcpt, pBuffer->data, written, true);

        TCPIP_HTTP_NET_ConnectionUserDataSet(connHandle, (const void*) 0);

        return TCPIP_HTTP_DYN_PRINT_RES_DONE;
    }

    return TCPIP_HTTP_DYN_PRINT_RES_DONE;
}

TCPIP_HTTP_DYN_PRINT_RES TCPIP_HTTP_Print_inputcapture(TCPIP_HTTP_NET_CONN_HANDLE connHandle, const TCPIP_HTTP_DYN_VAR_DCPT * vDcpt) {
    HTTP_APP_DYNVAR_BUFFER *pBuffer = HTTP_APP_GetDynamicBuffer();

    if (pBuffer == 0)
        return TCPIP_HTTP_DYN_PRINT_RES_AGAIN;

    uint32_t step = (uint32_t) (uintptr_t) TCPIP_HTTP_NET_ConnectionUserDataGet(connHandle);
    char *ptr = (char *) pBuffer->data;
    size_t spaceLeft = pBuffer->bufferSize;
    int written = 0;

    if (step == 0) {
        written = snprintf(ptr, spaceLeft, "{");
        TCPIP_HTTP_NET_DynamicWrite(vDcpt, pBuffer->data, written, true);

        TCPIP_HTTP_NET_ConnectionUserDataSet(connHandle, (const void*) (uintptr_t) 1);
        return TCPIP_HTTP_DYN_PRINT_RES_PROCESS_AGAIN;
    }

    uint32_t chIndex = step - 1;

    if (chIndex < MAX_INPUT_CAPTURE) {
        written = snprintf(ptr, spaceLeft,
                "\"%lu\": {\"enable\": %s,\"name\": \"%s\",\"value\": %.10g,\"state\": %s,\"counter\": %llu,\"ppu\": %.10g,\"unit\": \"%s\",",
                (unsigned long) chIndex,
                gInCaptureCfg.entry[chIndex].enable ? "true" : "false",
                gInCaptureCfg.entry[chIndex].name,
                inputCaptureDt.entry[chIndex].value,
                "false", // Gi? s? runtime state
                inputCaptureDt.entry[chIndex].counter,
                gInCaptureCfg.entry[chIndex].valPerPulse,
                gInCaptureCfg.entry[chIndex].unit
                );
        ptr += written;
        spaceLeft -= written;

        written = snprintf(ptr, spaceLeft,
                "\"status\": %d,\"freq\": %.10g,\"scaleType\": %d,\"scaleDataType\": %d,\"scaleValue\": %.10g,",
                inputCaptureDt.entry[chIndex].status,
                inputCaptureDt.entry[chIndex].freq,
                (int) gInCaptureCfg.entry[chIndex].scaleType,
                (int) gInCaptureCfg.entry[chIndex].scaleDataType,
                gInCaptureCfg.entry[chIndex].scaleValue
                );
        ptr += written;
        spaceLeft -= written;

        written = snprintf(ptr, spaceLeft,
                "\"offsetBefore\": %.10g,\"operator1\": %d,\"operator2\": %d,\"offsetAfter\": %.10g}%s",
                gInCaptureCfg.entry[chIndex].offsetPreVal,
                (int) gInCaptureCfg.entry[chIndex].offSetPreOperator,
                (int) gInCaptureCfg.entry[chIndex].offsetSubOperator,
                gInCaptureCfg.entry[chIndex].offsetSubVal,
                (chIndex == MAX_INPUT_CAPTURE - 1) ? "" : ","
                );
        ptr += written;
        spaceLeft -= written;

        TCPIP_HTTP_NET_DynamicWrite(vDcpt, pBuffer->data, (ptr - (char *) pBuffer->data), true);

        step++;
        TCPIP_HTTP_NET_ConnectionUserDataSet(connHandle, (const void*) (uintptr_t) step);
        return TCPIP_HTTP_DYN_PRINT_RES_PROCESS_AGAIN;
    }

    if (chIndex >= MAX_INPUT_CAPTURE) {
        written = snprintf(ptr, spaceLeft, "}");
        TCPIP_HTTP_NET_DynamicWrite(vDcpt, pBuffer->data, written, true);

        TCPIP_HTTP_NET_ConnectionUserDataSet(connHandle, (const void*) 0);

        return TCPIP_HTTP_DYN_PRINT_RES_DONE;
    }

    return TCPIP_HTTP_DYN_PRINT_RES_DONE;
}

static double _getModbusEntryValueAsDouble(uint16_t index) {
    switch (mbrtuMasterDt.entry[index].dataType) {
        case DATA_UINT: return (double) mbrtuMasterDt.entry[index].value.uintVal;
        case DATA_INT: return (double) mbrtuMasterDt.entry[index].value.intVal;
        case DATA_FLOAT: return (double) mbrtuMasterDt.entry[index].value.floatVal;

        default: return 0.0;
    }
}

TCPIP_HTTP_DYN_PRINT_RES TCPIP_HTTP_Print_modbus(TCPIP_HTTP_NET_CONN_HANDLE connHandle, const TCPIP_HTTP_DYN_VAR_DCPT * vDcpt) {
    HTTP_APP_DYNVAR_BUFFER *pBuffer = HTTP_APP_GetDynamicBuffer();

    if (pBuffer == 0)
        return TCPIP_HTTP_DYN_PRINT_RES_AGAIN;

    uint32_t step = (uint32_t) (uintptr_t) TCPIP_HTTP_NET_ConnectionUserDataGet(connHandle);
    char *ptr = (char *) pBuffer->data;
    size_t spaceLeft = pBuffer->bufferSize;
    int written = 0;

    if (step == 0) {
        written = snprintf(ptr, spaceLeft, "{");
        TCPIP_HTTP_NET_DynamicWrite(vDcpt, pBuffer->data, written, true);

        TCPIP_HTTP_NET_ConnectionUserDataSet(connHandle, (const void*) (uintptr_t) 1);
        return TCPIP_HTTP_DYN_PRINT_RES_PROCESS_AGAIN;
    }

    uint32_t chIndex = step - 1;

    if (chIndex < gMbrtuCfg.numTag) {
        written = snprintf(ptr, spaceLeft,
                "\"%lu\": {\"name\": \"%s\",\"value\": %.5g,\"unit\": \"%s\",\"modbusType\": %d,\"slaveId\": %d,\"tcpIp\": \"%d.%d.%d.%d\",\"tcpPort\": %u,\"status\": %d,",
                (unsigned long) (chIndex + 1),
                gMbrtuCfg.entry[chIndex].name,
                _getModbusEntryValueAsDouble(chIndex),
                gMbrtuCfg.entry[chIndex].unit,
                (int) gMbrtuCfg.entry[chIndex].type,
                (int) gMbrtuCfg.entry[chIndex].slaveAddress,
                gMbrtuCfg.entry[chIndex].ipAddress.v[0],
                gMbrtuCfg.entry[chIndex].ipAddress.v[1],
                gMbrtuCfg.entry[chIndex].ipAddress.v[2],
                gMbrtuCfg.entry[chIndex].ipAddress.v[3],
                (unsigned int) gMbrtuCfg.entry[chIndex].port,
                mbrtuMasterDt.entry[chIndex].status
                );
        ptr += written;
        spaceLeft -= written;

        written = snprintf(ptr, spaceLeft,
                "\"functionCode\": \"%u\",\"registerAddress\": \"%u\",\"quantity\": %u,\"dataType\": %d,\"byteOrder\": %d,\"enableDevice\": %s,\"conversion\": %s,",
                (unsigned int) gMbrtuCfg.entry[chIndex].function,
                (unsigned int) gMbrtuCfg.entry[chIndex].regAddress,
                (unsigned int) gMbrtuCfg.entry[chIndex].quantity,
                (int) gMbrtuCfg.entry[chIndex].rawDataType,
                (int) gMbrtuCfg.entry[chIndex].byteOder,
                gMbrtuCfg.entry[chIndex].enable ? "true" : "false",
                gMbrtuCfg.entry[chIndex].conversion ? "true" : "false"
                );
        ptr += written;
        spaceLeft -= written;

        written = snprintf(ptr, spaceLeft,
                "\"inputMin\": %.10g,\"inputMax\": %.10g,\"outputMin\": %.10g,\"outputMax\": %.10g,\"scaleType\": %d,\"scaleDataType\": %d,\"scaleValue\": %.10g,",
                gMbrtuCfg.entry[chIndex].inputMin,
                gMbrtuCfg.entry[chIndex].inputMax,
                gMbrtuCfg.entry[chIndex].outputMin,
                gMbrtuCfg.entry[chIndex].outputMax,
                (int) gMbrtuCfg.entry[chIndex].scaleType,
                (int) gMbrtuCfg.entry[chIndex].scaleDataType,
                gMbrtuCfg.entry[chIndex].scaleValue
                );
        ptr += written;
        spaceLeft -= written;

        written = snprintf(ptr, spaceLeft,
                "\"offsetBefore\": %.10g,\"operator1\": %d,\"operator2\": %d,\"offsetAfter\": %.10g}%s",
                gMbrtuCfg.entry[chIndex].offsetPreVal,
                (int) gMbrtuCfg.entry[chIndex].offSetPreOperator,
                (int) gMbrtuCfg.entry[chIndex].offsetSubOperator,
                gMbrtuCfg.entry[chIndex].offsetSubVal,
                (chIndex == gMbrtuCfg.numTag - 1) ? "" : ","
                );
        ptr += written;
        spaceLeft -= written;

        TCPIP_HTTP_NET_DynamicWrite(vDcpt, pBuffer->data, (ptr - (char *) pBuffer->data), true);

        step++;
        TCPIP_HTTP_NET_ConnectionUserDataSet(connHandle, (const void*) (uintptr_t) step);
        return TCPIP_HTTP_DYN_PRINT_RES_PROCESS_AGAIN;
    }

    if (chIndex >= gMbrtuCfg.numTag) {
        written = snprintf(ptr, spaceLeft, "}");
        TCPIP_HTTP_NET_DynamicWrite(vDcpt, pBuffer->data, written, true);

        TCPIP_HTTP_NET_ConnectionUserDataSet(connHandle, (const void*) 0);

        return TCPIP_HTTP_DYN_PRINT_RES_DONE;
    }

    return TCPIP_HTTP_DYN_PRINT_RES_DONE;
}

TCPIP_HTTP_DYN_PRINT_RES TCPIP_HTTP_Print_output(TCPIP_HTTP_NET_CONN_HANDLE connHandle, const TCPIP_HTTP_DYN_VAR_DCPT * vDcpt) {
    HTTP_APP_DYNVAR_BUFFER *pBuffer = HTTP_APP_GetDynamicBuffer();

    if (pBuffer == 0)
        return TCPIP_HTTP_DYN_PRINT_RES_AGAIN;

    uint32_t step = (uint32_t) (uintptr_t) TCPIP_HTTP_NET_ConnectionUserDataGet(connHandle);
    char *ptr = (char *) pBuffer->data;
    size_t spaceLeft = pBuffer->bufferSize;
    int written = 0;


    if (step == 0) {
        written = snprintf(ptr, spaceLeft, "{");
        TCPIP_HTTP_NET_DynamicWrite(vDcpt, pBuffer->data, written, true);

        TCPIP_HTTP_NET_ConnectionUserDataSet(connHandle, (const void*) (uintptr_t) 1);
        return TCPIP_HTTP_DYN_PRINT_RES_PROCESS_AGAIN;
    }

    uint32_t chIndex = step - 1;

    if (chIndex < MAX_DIGITAL_OUTPUT) {
        written = snprintf(ptr, spaceLeft,
                "\"%lu\": {\"pin\": \"DO%lu\",\"name\": \"%s\",\"desc\": \"%s\",",
                (unsigned long) chIndex,
                (unsigned long) chIndex,
                gAppCfg.io.out[chIndex].name,
                gAppCfg.io.out[chIndex].describe
                );
        ptr += written;
        spaceLeft -= written;

        written = snprintf(ptr, spaceLeft,
                "\"state\": %s,\"level\": %s,\"mode\": %d,\"on_time\": %u,\"off_time\": %u,\"pulse_count\": %u}%s",
                doDt.out[chIndex].state ? "true" : "false",
                doDt.out[chIndex].level ? "true" : "false",
                (int) gAppCfg.io.out[chIndex].mode,
                (unsigned int) gAppCfg.io.out[chIndex].ontime,
                (unsigned int) gAppCfg.io.out[chIndex].offtime,
                (unsigned int) gAppCfg.io.out[chIndex].pulseCount,
                (chIndex == MAX_DIGITAL_OUTPUT - 1) ? "" : ","
                );
        ptr += written;
        spaceLeft -= written;

        TCPIP_HTTP_NET_DynamicWrite(vDcpt, pBuffer->data, (ptr - (char *) pBuffer->data), true);

        step++;
        TCPIP_HTTP_NET_ConnectionUserDataSet(connHandle, (const void*) (uintptr_t) step);
        return TCPIP_HTTP_DYN_PRINT_RES_PROCESS_AGAIN;
    }

    if (chIndex >= MAX_DIGITAL_OUTPUT) {
        written = snprintf(ptr, spaceLeft, "}");
        TCPIP_HTTP_NET_DynamicWrite(vDcpt, pBuffer->data, written, true);

        TCPIP_HTTP_NET_ConnectionUserDataSet(connHandle, (const void*) 0);

        return TCPIP_HTTP_DYN_PRINT_RES_DONE;
    }

    return TCPIP_HTTP_DYN_PRINT_RES_DONE;
}

TCPIP_HTTP_DYN_PRINT_RES TCPIP_HTTP_Print_systime(TCPIP_HTTP_NET_CONN_HANDLE connHandle, const TCPIP_HTTP_DYN_VAR_DCPT * vDcpt) {
    HTTP_APP_DYNVAR_BUFFER *pBuffer = HTTP_APP_GetDynamicBuffer();
    if (pBuffer == 0)
        return TCPIP_HTTP_DYN_PRINT_RES_AGAIN;

    int written = snprintf((char *) pBuffer->data, pBuffer->bufferSize,
            "{"
            "\"systemTime\": \"%04u-%02u-%02uT%02u:%02u:%02u\""
            "}",
            (unsigned int) rtcDt.sysTime.year,
            (unsigned int) rtcDt.sysTime.month,
            (unsigned int) rtcDt.sysTime.day,
            (unsigned int) rtcDt.sysTime.hour,
            (unsigned int) rtcDt.sysTime.minute,
            (unsigned int) rtcDt.sysTime.second
            );

    TCPIP_HTTP_NET_DynamicWrite(vDcpt, pBuffer->data, written, true);

    return TCPIP_HTTP_DYN_PRINT_RES_DONE;
}

TCPIP_HTTP_DYN_PRINT_RES TCPIP_HTTP_Print_timeconfig(TCPIP_HTTP_NET_CONN_HANDLE connHandle, const TCPIP_HTTP_DYN_VAR_DCPT *vDcpt) {
    HTTP_APP_DYNVAR_BUFFER *pBuffer = HTTP_APP_GetDynamicBuffer();
    if (pBuffer == 0)
        return TCPIP_HTTP_DYN_PRINT_RES_AGAIN;

    int written = snprintf((char *) pBuffer->data, pBuffer->bufferSize,
            "{"
            "\"timezone\": %d,"
            "\"dateFormat\": %d,"
            "\"timeFormat\": %d,"
            "\"ntpEnable\": %s,"
            "\"ntpServer1\": \"%s\","
            "\"ntpServer2\": \"%s\","
            "\"ntpInterval\": \"%lu\","
            "\"ntpPort\": %u,"
            "\"ntpStatus\": %s,"
            "\"lastSync\": \"%s\""
            "}",
            (int) gAppCfg.time.timeZone,
            0, // Gi? s? (runtime ho?c HMI config) cho dateFormat
            0, // Gi? s? (runtime ho?c HMI config) cho timeFormat
            gAppCfg.time.syncNtpEnable ? "true" : "false",
            gAppCfg.time.ntpServerPrimary,
            gAppCfg.time.ntpServerBackup,
            (unsigned long) gAppCfg.time.syncInterval,
            (unsigned int) gAppCfg.time.ntpPort,
            "true", // Gi? s? runtime status cho NTP
            "10 last minute" // Gi? s? runtime text cho l?n ??ng b? cu?i
            );

    TCPIP_HTTP_NET_DynamicWrite(vDcpt, pBuffer->data, written, true);

    return TCPIP_HTTP_DYN_PRINT_RES_DONE;
}

TCPIP_HTTP_DYN_PRINT_RES TCPIP_HTTP_Print_sensorGeneral(TCPIP_HTTP_NET_CONN_HANDLE connHandle, const TCPIP_HTTP_DYN_VAR_DCPT *vDcpt) {
    HTTP_APP_DYNVAR_BUFFER *pBuffer = HTTP_APP_GetDynamicBuffer();

    if (pBuffer == 0) {
        LOG_DEBUG("%s - %s\t pBuffer = 0", __TAG__, __func__);
        return TCPIP_HTTP_DYN_PRINT_RES_AGAIN;
    }

    uint32_t step = (uint32_t) (uintptr_t) TCPIP_HTTP_NET_ConnectionUserDataGet(connHandle);
    char *ptr = (char *) pBuffer->data;
    size_t spaceLeft = pBuffer->bufferSize;
    int written = 0;

    if (step == 0) {
        written = snprintf(ptr, spaceLeft, "{\"analogSensors\":[");
        ptr += written;
        spaceLeft -= written;

        bool isFirst = true;
        for (int i = 0; i < MAX_ANALOG_CHANNEL; i++) {
            if (gAnalogCfg.entry[i].enable) {
                written = snprintf(ptr, spaceLeft, "%s{\"id\":%d,\"name\":\"%s\"}", isFirst ? "" : ",", i, gAnalogCfg.entry[i].name);
                ptr += written;
                spaceLeft -= written;
                isFirst = false;
            }
        }

        written = snprintf(ptr, spaceLeft, "],\"modbusTags\":[");
        ptr += written;
        spaceLeft -= written;
        isFirst = true;
        for (int i = 0; i < MAX_MODBUS_TAG; i++) {
            if (gMbrtuCfg.entry[i].enable) {
                written = snprintf(ptr, spaceLeft, "%s{\"id\":%d,\"name\":\"%s\"}", isFirst ? "" : ",", i, gMbrtuCfg.entry[i].name);
                ptr += written;
                spaceLeft -= written;
                isFirst = false;
            }
        }

        written = snprintf(ptr, spaceLeft, "],\"inputCaptures\":[");
        ptr += written;
        spaceLeft -= written;
        isFirst = true;
        for (int i = 0; i < MAX_INPUT_CAPTURE; i++) {
            if (gInCaptureCfg.entry[i].enable) {
                written = snprintf(ptr, spaceLeft, "%s{\"id\":%d,\"name\":\"%s\"}", isFirst ? "" : ",", i, gInCaptureCfg.entry[i].name);
                ptr += written;
                spaceLeft -= written;
                isFirst = false;
            }
        }

        written = snprintf(ptr, spaceLeft, "],\"digitalInputs\":[],\"sensors\":[");
        ptr += written;
        spaceLeft -= written;

        TCPIP_HTTP_NET_DynamicWrite(vDcpt, pBuffer->data, (ptr - (char *) pBuffer->data), true);
        TCPIP_HTTP_NET_ConnectionUserDataSet(connHandle, (const void*) 1);
        return TCPIP_HTTP_DYN_PRINT_RES_PROCESS_AGAIN;
    }

    uint32_t idx = step - 1;

    while (idx < gSensorCfg.numSensor) {
        if (spaceLeft < 350)
            break;

        SENSOR_ENTRY_CONFIG* s = &gSensorCfg.entry[idx];
        const char* unitStr = "";

        if (s->type == SENSOR_ANALOG && s->indexOfType < MAX_ANALOG_CHANNEL) {
            unitStr = gAnalogCfg.entry[s->indexOfType].unit;
        } else if (s->type == SENSOR_MBRTU && s->indexOfType < MAX_MODBUS_TAG) {
            unitStr = gMbrtuCfg.entry[s->indexOfType].unit;
        } else if (s->type == SENSOR_INPUT_CAPTURE && s->indexOfType < MAX_INPUT_CAPTURE) {
            unitStr = gInCaptureCfg.entry[s->indexOfType].unit;
        }

        char sucId[16] = "\"\"", errId[16] = "\"\"", calId[16] = "\"\"";
        char sucAnd[16] = "\"\"", sucExp[16] = "\"\"", errAnd[16] = "\"\"", errExp[16] = "\"\"", calAnd[16] = "\"\"", calExp[16] = "\"\"";

        if (s->typeStatus == FROM_MBRTU || s->typeStatus == FROM_DIGITAL_INPUT) {
            snprintf(sucId, 16, "%u", s->indexOfTypeGood);
            snprintf(errId, 16, "%u", s->indexOfTypeErr);
            snprintf(calId, 16, "%u", s->indexOfTypeCalib);
        }
        if (s->typeStatus == FROM_MBRTU) {
            snprintf(sucAnd, 16, "%u", s->goodValueAND);
            snprintf(sucExp, 16, "%u", s->goodValueCompare);
            snprintf(errAnd, 16, "%u", s->errorValueAND);
            snprintf(errExp, 16, "%u", s->errorValueCompare);
            snprintf(calAnd, 16, "%u", s->calibValueAND);
            snprintf(calExp, 16, "%u", s->calibValueCompare);
        }

        written = snprintf(ptr, spaceLeft,
                "%s{\"id\": %lu, \"type\": %d, \"sensorId\": %u, \"unit\": \"%s\", \"sourceType\": %d, "
                "\"successId\": %s, \"errorId\": %s, \"calibId\": %s, \"successAndValue\": %s, "
                "\"successExpectedValue\": %s, \"errorAndValue\": %s, \"errorExpectedValue\": %s, "
                "\"calibAndValue\": %s, \"calibExpectedValue\": %s, \"active\": %s, \"calib\": %s}",
                (idx == 0) ? "" : ",",
                (unsigned long) (idx + 1),
                s->type,
                s->indexOfType,
                unitStr,
                s->typeStatus,
                sucId, errId, calId,
                sucAnd, sucExp, errAnd, errExp, calAnd, calExp,
                s->enable ? "true" : "false",
                s->calibrate ? "true" : "false"
                );

        ptr += written;
        spaceLeft -= written;
        idx++;
    }

    if (idx >= gSensorCfg.numSensor) {
        if (spaceLeft >= 3) {
            written = snprintf(ptr, spaceLeft, "]}");
            ptr += written;
        }
        TCPIP_HTTP_NET_DynamicWrite(vDcpt, pBuffer->data, (ptr - (char *) pBuffer->data), true);
        TCPIP_HTTP_NET_ConnectionUserDataSet(connHandle, (const void*) 0);
        return TCPIP_HTTP_DYN_PRINT_RES_DONE;
    }

    TCPIP_HTTP_NET_DynamicWrite(vDcpt, pBuffer->data, (ptr - (char *) pBuffer->data), true);
    step = idx + 1;
    TCPIP_HTTP_NET_ConnectionUserDataSet(connHandle, (const void*) (uintptr_t) step);

    return TCPIP_HTTP_DYN_PRINT_RES_PROCESS_AGAIN;
}

TCPIP_HTTP_DYN_PRINT_RES TCPIP_HTTP_Print_exportSettings(TCPIP_HTTP_NET_CONN_HANDLE connHandle, const TCPIP_HTTP_DYN_VAR_DCPT *vDcpt) {
    HTTP_APP_DYNVAR_BUFFER *pBuffer = HTTP_APP_GetDynamicBuffer();
    if (pBuffer == 0) {
        return TCPIP_HTTP_DYN_PRINT_RES_AGAIN;
    }

    char *ptr = (char *) pBuffer->data;
    size_t spaceLeft = pBuffer->bufferSize;
    int written = 0;

    written = snprintf(ptr, spaceLeft,
            "{"
            "\"logInterval\": %u,"
            "\"dataFormat\": %d,"
            "\"fileName\": \"%s\","
            "\"fileType\": %d,"
            "\"compress\": %s,"
            "\"ftpEnable\": %s,"
            "\"mqttEnable\": %s,"
            "\"sdEnable\": %s"
            "}",
            (unsigned int) gSensorCfg.logInterval,
            (int) gSensorCfg.formatFile,
            gSensorCfg.filenameTemplate,
            (int) gSensorCfg.typefile,
            gSensorCfg.compressed ? "true" : "false",
            gSensorCfg.uploadFtp ? "true" : "false",
            gSensorCfg.uploadMqtt ? "true" : "false",
            gSensorCfg.saveSdcard ? "true" : "false"
            );

    TCPIP_HTTP_NET_DynamicWrite(vDcpt, pBuffer->data, written, true);
    return TCPIP_HTTP_DYN_PRINT_RES_DONE;
}

TCPIP_HTTP_DYN_PRINT_RES TCPIP_HTTP_Print_ftpconfig(TCPIP_HTTP_NET_CONN_HANDLE connHandle, const TCPIP_HTTP_DYN_VAR_DCPT *vDcpt) {
    HTTP_APP_DYNVAR_BUFFER *pBuffer = HTTP_APP_GetDynamicBuffer();
    if (pBuffer == 0) {
        return TCPIP_HTTP_DYN_PRINT_RES_AGAIN;
    }

    char *ptr = (char *) pBuffer->data;
    size_t spaceLeft = pBuffer->bufferSize;
    int written = 0;

    written = snprintf(ptr, spaceLeft, "{");
    ptr += written;
    spaceLeft -= written;

    for (int i = 0; i < MAX_FTP_SERVER; i++) {
        FTP_SERVER_CONFIG *ftp = &gAppCfg.ftpServer[i];

        written = snprintf(ptr, spaceLeft,
                "%s\"%d\": {"
                "\"enable\": %s,"
                "\"protocol\": 0,"
                "\"host\": \"%s\","
                "\"port\": %u,"
                "\"username\": \"%s\","
                "\"remotePath\": \"%s\","
                "\"folderStructure\": %d,"
                "\"passive\": 0,"
                "\"timeout\": 10,"
                "\"status\": 0"
                "}",
                (i == 0) ? "" : ",",
                i + 1,
                ftp->enable ? "true" : "false",
                ftp->hostname,
                (unsigned int) ftp->port,
                ftp->username,
                ftp->dirPath,
                (int) ftp->makeFolder
                );

        ptr += written;
        spaceLeft -= written;

        if (spaceLeft <= 0) break;
    }

    if (spaceLeft > 0) {
        written = snprintf(ptr, spaceLeft, "}");
        ptr += written;
    }

    TCPIP_HTTP_NET_DynamicWrite(vDcpt, pBuffer->data, (ptr - (char *) pBuffer->data), true);
    return TCPIP_HTTP_DYN_PRINT_RES_DONE;
}

TCPIP_HTTP_DYN_PRINT_RES TCPIP_HTTP_Print_network(TCPIP_HTTP_NET_CONN_HANDLE connHandle, const TCPIP_HTTP_DYN_VAR_DCPT *vDcpt) {
    HTTP_APP_DYNVAR_BUFFER *pBuffer = HTTP_APP_GetDynamicBuffer();
    if (pBuffer == 0) {
        return TCPIP_HTTP_DYN_PRINT_RES_AGAIN;
    }

    char *ptr = (char *) pBuffer->data;
    size_t spaceLeft = pBuffer->bufferSize;
    int written = 0;

    TCPIP_NET_HANDLE netH = TCPIP_STACK_IndexToNet(0);
    const uint8_t* mac = (const uint8_t*) TCPIP_STACK_NetAddressMac(netH);

    IPV4_ADDR *ip = (IPV4_ADDR *) & gAppCfg.network.ipAddr;
    IPV4_ADDR *mask = (IPV4_ADDR *) & gAppCfg.network.ipMask;
    IPV4_ADDR *gw = (IPV4_ADDR *) & gAppCfg.network.gateway;
    IPV4_ADDR *dns1 = (IPV4_ADDR *) & gAppCfg.network.primaryDNS;
    IPV4_ADDR *dns2 = (IPV4_ADDR *) & gAppCfg.network.secondDNS;

    written = snprintf(ptr, spaceLeft,
            "{"
            "\"dhcp\": %d,"
            "\"ipAddress\": \"%d.%d.%d.%d\","
            "\"subnetMask\": \"%d.%d.%d.%d\","
            "\"gateway\": \"%d.%d.%d.%d\","
            "\"dns1\": \"%d.%d.%d.%d\","
            "\"dns2\": \"%d.%d.%d.%d\","
            "\"hostname\": \"%s\","
            "\"macAddress\": \"%02X:%02X:%02X:%02X:%02X:%02X\","
            "\"uplinkMode\": %d,"
            "\"interface\": \"%s\","
            "\"username\": \"%s\","
            "\"password\": \"%s\""
            "}",
            gAppCfg.network.isDHCPEn ? 1 : 0,
            ip->v[0], ip->v[1], ip->v[2], ip->v[3],
            mask->v[0], mask->v[1], mask->v[2], mask->v[3],
            gw->v[0], gw->v[1], gw->v[2], gw->v[3],
            dns1->v[0], dns1->v[1], dns1->v[2], dns1->v[3],
            dns2->v[0], dns2->v[1], dns2->v[2], dns2->v[3],
            gAppCfg.network.netBIOSName,
            mac ? mac[0] : 0, mac ? mac[1] : 0, mac ? mac[2] : 0, mac ? mac[3] : 0, mac ? mac[4] : 0, mac ? mac[5] : 0,
            (int) gAppCfg.network.uplink,
            "eth0",
            gAppCfg.network.deviceUsername,
            "......"
            );

    ptr += written;

    TCPIP_HTTP_NET_DynamicWrite(vDcpt, pBuffer->data, (ptr - (char *) pBuffer->data), true);
    return TCPIP_HTTP_DYN_PRINT_RES_DONE;
}

TCPIP_HTTP_DYN_PRINT_RES TCPIP_HTTP_Print_sim(TCPIP_HTTP_NET_CONN_HANDLE connHandle, const TCPIP_HTTP_DYN_VAR_DCPT *vDcpt) {
    HTTP_APP_DYNVAR_BUFFER *pBuffer = HTTP_APP_GetDynamicBuffer();
    if (pBuffer == 0) {
        return TCPIP_HTTP_DYN_PRINT_RES_AGAIN;
    }

    char *ptr = (char *) pBuffer->data;
    size_t spaceLeft = pBuffer->bufferSize;
    int written = 0;

    SIM_BASIC_INFO* simInfo = SIMBasic_GetInfo();
    if (simInfo != NULL) {
        int bars = 0;
        if (simInfo->rssi < 0 && simInfo->rssi >= -70) bars = 4;
        else if (simInfo->rssi >= -85) bars = 3;
        else if (simInfo->rssi >= -100) bars = 2;
        else if (simInfo->rssi >= -110) bars = 1;
        else bars = 0;

        written = snprintf(ptr, spaceLeft,
                "{"
                "\"simCards\": ["
                "{"
                "\"simSlot\": 0,"
                "\"name\": \"%s\","
                "\"network\": \"%s\","
                "\"status\": %d,"
                "\"technology\": 4,"
                "\"rssi\": %d,"
                "\"imei\": \"%s\","
                "\"iccid\": \"%s\","
                "\"apn\": \"%s\","
                "\"username\": \"%s\","
                "\"password\": \"%s\","
                "\"bars\": %d,"
                "\"active\": %d"
                "}"
                "]"
                "}",
                simInfo->networkName,
                simInfo->networkName,
                simInfo->inserted ? 1 : 0,
                simInfo->rssi,
                simInfo->imei,
                simInfo->ccid,
                gAppCfg.gsm.APN,
                gAppCfg.gsm.usernameAPN,
                "......",
                bars,
                1
                );
    } else {
        written = snprintf(ptr, spaceLeft, "{\"simCards\":[]}");
    }

    ptr += written;

    TCPIP_HTTP_NET_DynamicWrite(vDcpt, pBuffer->data, (ptr - (char *) pBuffer->data), true);

    return TCPIP_HTTP_DYN_PRINT_RES_DONE;
}

TCPIP_HTTP_DYN_PRINT_RES TCPIP_HTTP_Print_hmi(TCPIP_HTTP_NET_CONN_HANDLE connHandle, const TCPIP_HTTP_DYN_VAR_DCPT *vDcpt) {
    HTTP_APP_DYNVAR_BUFFER *pBuffer = HTTP_APP_GetDynamicBuffer();
    if (pBuffer == 0) {
        return TCPIP_HTTP_DYN_PRINT_RES_AGAIN;
    }

    char *ptr = (char *) pBuffer->data;
    size_t spaceLeft = pBuffer->bufferSize;
    int written = 0;

    uint32_t step = (uint32_t) (uintptr_t) TCPIP_HTTP_NET_ConnectionUserDataGet(connHandle);
    if (step == 0) {
        written = snprintf(ptr, spaceLeft, "{\"allSensors\":[");
        ptr += written;
        spaceLeft -= written;
        step = 1;
    }

    uint32_t idx = step - 1;
    while (idx < MAX_SENSOR) {
        if (spaceLeft < 150) {
            break;
        }

        SENSOR_ENTRY_CONFIG* s = &gSensorCfg.entry[idx];

        if (s->enable) {
            const char* nameStr = "";
            if (s->type == SENSOR_ANALOG && s->indexOfType < MAX_ANALOG_CHANNEL) {
                nameStr = gAnalogCfg.entry[s->indexOfType].name;
            } else if (s->type == SENSOR_MBRTU && s->indexOfType < MAX_MODBUS_TAG) {
                nameStr = gMbrtuCfg.entry[s->indexOfType].name;
            } else if (s->type == SENSOR_INPUT_CAPTURE && s->indexOfType < MAX_INPUT_CAPTURE) {
                nameStr = gInCaptureCfg.entry[s->indexOfType].name;
            }

            bool isFirst = true;
            for (int i = 0; i < idx; i++) {
                if (gSensorCfg.entry[i].enable) {
                    isFirst = false;
                    break;
                }
            }

            written = snprintf(ptr, spaceLeft, "%s{\"id\": %d, \"name\": \"%s\", \"type\": %d}",
                    isFirst ? "" : ",",
                    idx + 1,
                    nameStr,
                    (int) s->type
                    );
            ptr += written;
            spaceLeft -= written;
        }
        idx++;
    }

    if (idx >= MAX_SENSOR) {
        if (spaceLeft >= 100) {
            written = snprintf(ptr, spaceLeft, "],\"hmiLayout\": [");
            ptr += written;
            spaceLeft -= written;

            for (int i = 0; i < gAppCfg.hmi.numEntry && i < MAX_HMI_PARA; i++) {
                written = snprintf(ptr, spaceLeft, "%s%d",
                        (i == 0) ? "" : ", ",
                        gAppCfg.hmi.sensorIdx[i]
                        );
                ptr += written;
                spaceLeft -= written;
            }

            written = snprintf(ptr, spaceLeft, "]}");
            ptr += written;

            TCPIP_HTTP_NET_DynamicWrite(vDcpt, pBuffer->data, (ptr - (char *) pBuffer->data), true);
            TCPIP_HTTP_NET_ConnectionUserDataSet(connHandle, (const void*) 0);
            return TCPIP_HTTP_DYN_PRINT_RES_DONE;
        }
    }

    TCPIP_HTTP_NET_DynamicWrite(vDcpt, pBuffer->data, (ptr - (char *) pBuffer->data), true);

    step = idx + 1;
    TCPIP_HTTP_NET_ConnectionUserDataSet(connHandle, (const void*) (uintptr_t) step);

    return TCPIP_HTTP_DYN_PRINT_RES_PROCESS_AGAIN;
}