#include "app.h"
#include "http_net_print.h"

DEVICE_INFO gDeviceInfo;

static const char * __TAG__ = "APP";
static APP_STATES _state = 0;
static uint32_t _initDelayTick = 0;

/* ===================================================================
 * Apply network configuration (DHCP on/off, static IP, NetBIOS name)
 * =================================================================*/
static void _applyNetworkConfig(TCPIP_NET_HANDLE netH) {
    if (netH == 0) {
        LOG_ERROR("%s:\t ApplyNetCfg: invalid netH", __TAG__);
        return;
    }

    if (gAppCfg.network.netBIOSName[0] != '\0') {
        TCPIP_STACK_NetBiosNameSet(netH, gAppCfg.network.netBIOSName);
        LOG_INFO("%s:\t NetBIOS name set to '%s'",
                __TAG__, gAppCfg.network.netBIOSName);
    }

    if (gAppCfg.network.isDHCPEn) {
        /* ============================================================
         *  DHCP MODE
         * ============================================================*/
        if (!TCPIP_DHCP_IsEnabled(netH)) {
            TCPIP_DHCP_Enable(netH);
        }

        IPV4_ADDR lastIp;
        lastIp.Val = gAppCfg.network.ipAddr.Val;

        if (lastIp.Val != 0) {
            if (TCPIP_DHCP_Request(netH, lastIp)) {
                LOG_INFO("%s:\t DHCP enabled, requesting last IP %d.%d.%d.%d",
                        __TAG__, lastIp.v[0], lastIp.v[1], lastIp.v[2], lastIp.v[3]);
            } else {
                LOG_WARN("%s:\t DHCP enabled, hint last IP failed, fallback DISCOVER",
                        __TAG__);
            }
        } else {
            LOG_INFO("%s:\t DHCP enabled, no last IP hint", __TAG__);
        }
    } else {
        /* ============================================================
         *  STATIC MODE
         *  - Disable DHCP client
         *  - Set IP / Mask / Gateway / DNS
         * ============================================================*/
        if (TCPIP_DHCP_IsEnabled(netH)) {
            TCPIP_DHCP_Disable(netH);
            LOG_INFO("%s:\t DHCP disabled (static mode)", __TAG__);
        }

        IPV4_ADDR ip;
        ip.Val = gAppCfg.network.ipAddr.Val;
        IPV4_ADDR mask;
        mask.Val = gAppCfg.network.ipMask.Val;
        IPV4_ADDR gw;
        gw.Val = gAppCfg.network.gateway.Val;
        IPV4_ADDR dns1;
        dns1.Val = gAppCfg.network.primaryDNS.Val;
        IPV4_ADDR dns2;
        dns2.Val = gAppCfg.network.secondDNS.Val;

        if (!TCPIP_STACK_NetAddressSet(netH, &ip, &mask, true)) {
            LOG_ERROR("%s:\t Static IP set FAILED", __TAG__);
        }

        TCPIP_STACK_NetAddressGatewaySet(netH, &gw);
        TCPIP_STACK_NetDnsPrimarySet(netH, &dns1);
        TCPIP_STACK_NetDnsSecondSet(netH, &dns2);

        LOG_INFO("%s:\t Static IP   : %d.%d.%d.%d",
                __TAG__, ip.v[0], ip.v[1], ip.v[2], ip.v[3]);
        LOG_INFO("%s:\t Subnet Mask : %d.%d.%d.%d",
                __TAG__, mask.v[0], mask.v[1], mask.v[2], mask.v[3]);
        LOG_INFO("%s:\t Gateway     : %d.%d.%d.%d",
                __TAG__, gw.v[0], gw.v[1], gw.v[2], gw.v[3]);
        LOG_INFO("%s:\t DNS1 / DNS2 : %d.%d.%d.%d / %d.%d.%d.%d",
                __TAG__,
                dns1.v[0], dns1.v[1], dns1.v[2], dns1.v[3],
                dns2.v[0], dns2.v[1], dns2.v[2], dns2.v[3]);
    }
}

void App_Initialize(void) {
    /* Place the App state machine in its initial state. */

    /* TODO: Initialize your application's state machine and other
     * parameters.
     */
}

void App_Tasks(void) {
    static const uint8_t numAttemps = 30;
    static uint8_t attempCount = 0;
    static IPV4_ADDR dwLastIP[2] = {
        {-1},
        {-1}
    };

    /* Check the application's current state. */
    switch (_state) {
        case APP_INIT_MODULE:
        {
            bool appInitialized = true;
            Rtc_Initialize();
            SIMMain_Initialize();
            SDcard_Initialize();
            BootConfig_Initialize();
            ExtFlash_Initialize();
            Fram_Initialize();
            Adc_Initialize();
            MbrtuMaster_Initialize();
            InputCapture_Initialize();
            DigitalOutput_Initialize();
            EthNtp_Initialize();
            SensorGeneral_Initialize();
            EthFtp_Initialize();
            HMIDwin_Initialize();
            EthMqtt_Initialize();
            InFlash_Initialize();
            EthHttp_Initialize();
            Fota_Initialize(NULL);

            if (appInitialized) {
                LOG_SUCCESS("%s:\t Module Init SUCCESS!", __TAG__);
                _state = APP_BOOT_CONFIG;
                _initDelayTick = TICK_NOW();
            }
            break;
        }

        case APP_BOOT_CONFIG:
        {
            ExtFlash_Task();
            if (BootConfig_Task())
                _state = APP_MOUNT_DISK;

            break;
        }

        case APP_MOUNT_DISK:
        {
            if (!TIME_IS_EXPIRED(_initDelayTick, 1000))
                break;

            LedIndicate_Initialize();
            if (SYS_FS_Mount(SYS_FS_SPIFLASH_VOL, SYS_FS_SPIFLASH_MOUNT_POINT, SYS_FS_SPIFLASH_TYPE, 0, NULL) == 0) {
                LOG_INFO("%s:\t Flash %s File System is mounted", __TAG__, SYS_FS_SPIFLASH_TYPE_STRING);
                _state = APP_LOAD_DEVICE_INFO;
            } else {
                if (++attempCount > numAttemps) {
                    LOG_ERROR("%s:\t Flash %s File System mount failed", __TAG__, SYS_FS_SPIFLASH_TYPE_STRING);
                    attempCount = 0;
                    _state = APP_LOAD_DEVICE_INFO;
                }
            }
            break;
        }

        case APP_LOAD_DEVICE_INFO:
        {
            bool ret = InFlash_LoadDeviceInfo((uint8_t *) & gDeviceInfo, sizeof (DEVICE_INFO));
            LOG_DEBUG("%s:\t APP_LOAD_DEVICE_INFO ret=%u", __TAG__, ret);

            uint32_t crc = Helpers_CRC32Calculate((uint8_t *) & gDeviceInfo.manufacturer, sizeof (DEVICE_INFO) - sizeof (gDeviceInfo.crc));
            if (crc != gDeviceInfo.crc)
                _state = APP_SAVE_DEVICE_INFO;
            else {
                LOG_INFO("%s:\t Device Info Loaded Successfully:", __TAG__);
                LOG_INFO("\tManufacturer : %s", gDeviceInfo.manufacturer);
                LOG_INFO("\tFW Version   : %s", gDeviceInfo.fwVer);
                LOG_INFO("\tHW Version   : %s", gDeviceInfo.hwVer);
                LOG_INFO("\tDate Time    : %s", gDeviceInfo.dateTime);
                LOG_INFO("\tModel        : %s", gDeviceInfo.model);
                LOG_INFO("\tSerial       : %s", gDeviceInfo.serial);
                LOG_INFO("\tCRC          : 0x%08X", gDeviceInfo.crc);
                _state = APP_TCPIP_INIT;
            }
            break;
        }

        case APP_SAVE_DEVICE_INFO:
        {
            snprintf(gDeviceInfo.manufacturer, MANUFACTURER_LEN, "%s", MANUFACTURER);
            snprintf(gDeviceInfo.fwVer, FW_VER_LEN, "%s", FIRMWARE_VERSION);
            snprintf(gDeviceInfo.hwVer, HW_VER_LEN, "%s", HARDWARE_VERSION);
            snprintf(gDeviceInfo.dateTime, DATE_LEN, "%s", DATE_TIME);
            snprintf(gDeviceInfo.model, MODEL_LEN, "%s", MODEL);
            snprintf(gDeviceInfo.serial, SERIAL_LEN, "%s", SERIAL_NUMBER);
            gDeviceInfo.crc = Helpers_CRC32Calculate((uint8_t *) & gDeviceInfo.manufacturer, sizeof (DEVICE_INFO) - sizeof (gDeviceInfo.crc));

            bool ret = InFlash_SaveDeviceInfo((uint8_t *) & gDeviceInfo, sizeof (DEVICE_INFO));
            LOG_DEBUG("%s:\t APP_SAVE_DEVICE_INFO ret=%u", __TAG__, ret);
            if (++attempCount > numAttemps)
                _state = APP_TCPIP_INIT;
            else
                _state = APP_LOAD_DEVICE_INFO;
            break;
        }

        case APP_TCPIP_INIT:
        {
            SYS_STATUS tcpipStat = TCPIP_STACK_Status(sysObj.tcpip);
            if (tcpipStat < 0) { // some error occurred
                LOG_ERROR("%s:\t TCP/IP stack\t initialization FAILED!", __TAG__);
                _state = APP_TCPIP_ERROR;
            }

            if (tcpipStat == SYS_STATUS_READY) {
                LOG_SUCCESS("%s:\t TCP/IP stack\t Init SUCCESS!", __TAG__);
                _state = APP_TCPIP_WAIT_INIT;
            }
            break;
        }

        case APP_TCPIP_WAIT_INIT:
        {
            int nNets = TCPIP_STACK_NumberOfNetworksGet();
            const char *netName, *netBiosName;
            for (int i = 0; i < nNets; i++) {
                TCPIP_NET_HANDLE netH = TCPIP_STACK_IndexToNet(i);
                _applyNetworkConfig(netH);

                netName = TCPIP_STACK_NetNameGet(netH);
                netBiosName = TCPIP_STACK_NetBIOSName(netH);

                LOG_DEBUG("%s:\t TCP/IP stack\t Interface %s on host %s - NBNS disabled", __TAG__, netName, netBiosName);
            }

            EthMqtt_Open();
            HTTP_APP_Initialize();
            _state = APP_TCPIP_TRANSACT;

            break;
        }

        case APP_TCPIP_TRANSACT:
        {
            int nNets = TCPIP_STACK_NumberOfNetworksGet();
            IPV4_ADDR ipAddr;

            for (int i = 0; i < nNets; i++) {
                TCPIP_NET_HANDLE netH = TCPIP_STACK_IndexToNet(i);
                ipAddr.Val = TCPIP_STACK_NetAddress(netH);
                if (dwLastIP[i].Val != ipAddr.Val) {
                    dwLastIP[i].Val = ipAddr.Val;
                    LOG_INFO("%s:\t TCP/IP stack \t %s IP Address: %d.%d.%d.%d "
                            , __TAG__, TCPIP_STACK_NetNameGet(netH), ipAddr.v[0], ipAddr.v[1], ipAddr.v[2], ipAddr.v[3]);
                }
            }
            break;
        }

        default: break;
    }

    if (_state > APP_BOOT_CONFIG) {
        Rtc_Task();
        SIMMain_Task();
        SDcard_Task();
        ExtFlash_Task();
        Fram_Task();
        Adc_Task();
        MbrtuMaster_Tasks();
        InputCapture_Task();
        LedIndicate_Task();
        DigitalOutput_Task();
        EthNtp_Task();
        SensorGeneral_Task();
        EthFtp_Task();
        HMIDwin_Tasks();
        EthMqtt_Task();
        EthHttp_Task();
        Fota_Task();
    }
}


/*******************************************************************************
 End of File
 */
