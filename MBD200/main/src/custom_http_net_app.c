#include "system_config.h"
#include "system_definitions.h"
#include "http_net_print.h"
//#include "bootloader/bootloader_nvm_interface.h"


#include "net_pres/pres/net_pres_socketapi.h"
#include "system/sys_random_h2_adapter.h"
#include "system/sys_time_h2_adapter.h"
#include "tcpip/tcpip.h"
#include "tcpip/src/common/helpers.h"


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

        if (!strcmp((char *) httpDataBuff, (const char *) "logInterval")) {
            uint8_t isDHCPEnable = atoi((char *) (httpDataBuff + 32));
            LOG_INFO("isDHCPEnable %u", isDHCPEnable);
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

static TCPIP_HTTP_NET_IO_RESULT HTTPPostHmiDisplay(TCPIP_HTTP_NET_CONN_HANDLE connHandle) {
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

static TCPIP_HTTP_NET_IO_RESULT HTTPPostAnalog(TCPIP_HTTP_NET_CONN_HANDLE connHandle) {
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

static TCPIP_HTTP_NET_IO_RESULT HTTPPostModbus(TCPIP_HTTP_NET_CONN_HANDLE connHandle) {
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

static TCPIP_HTTP_NET_IO_RESULT HTTPPostInputCapture(TCPIP_HTTP_NET_CONN_HANDLE connHandle) {
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

static TCPIP_HTTP_NET_IO_RESULT HTTPPostOutput(TCPIP_HTTP_NET_CONN_HANDLE connHandle) {
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

static TCPIP_HTTP_NET_IO_RESULT HTTPPostSim(TCPIP_HTTP_NET_CONN_HANDLE connHandle) {
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

static TCPIP_HTTP_NET_IO_RESULT HTTPPostFtp(TCPIP_HTTP_NET_CONN_HANDLE connHandle) {
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