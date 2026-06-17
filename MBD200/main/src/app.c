#include "app.h"
#include "http_net_print.h"

static const char * __TAG__ = "APP";
static APP_STATES _state = 0;
static DEVICE_INFO _deviceInfo = {0};
static uint32_t _initDelayTick = 0;

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
            //            bool ret = InFlash_LoadDeviceInfo((uint8_t *) & _deviceInfo, sizeof (DEVICE_INFO));
            //            LOG_DEBUG("%s:\t APP_LOAD_DEVICE_INFO ret=%u", __TAG__, ret);
            //
            //            uint32_t crc = Helpers_CRC32Calculate((uint8_t *) & _deviceInfo.manufacturer, sizeof (DEVICE_INFO) - sizeof (_deviceInfo.crc));
            //            if (crc != _deviceInfo.crc)
            _state = APP_SAVE_DEVICE_INFO;
            //            else
            //                _state = APP_TCPIP_INIT;
            //            break;
        }

        case APP_SAVE_DEVICE_INFO:
        {
            //            snprintf(_deviceInfo.manufacturer, MANUFACTURER_LEN, "%s", MANUFACTURER);
            //            snprintf(_deviceInfo.fwVer, FW_VER_LEN, "%s", FIRMWARE_VERSION);
            //            snprintf(_deviceInfo.hwVer, HW_VER_LEN, "%s", HARDWARE_VERSION);
            //            snprintf(_deviceInfo.dateTime, DATE_LEN, "%s", DATE_TIME);
            //            snprintf(_deviceInfo.model, MODEL_LEN, "%s", MODEL);
            //            snprintf(_deviceInfo.serial, SERIAL_LEN, "%s", SERIAL_NUMBER);
            //            _deviceInfo.crc = Helpers_CRC32Calculate((uint8_t *) & _deviceInfo.manufacturer, sizeof (DEVICE_INFO) - sizeof (_deviceInfo.crc));
            //
            //            bool ret = InFlash_SaveDeviceInfo((uint8_t *) & _deviceInfo, sizeof (DEVICE_INFO));
            //            LOG_DEBUG("%s:\t APP_SAVE_DEVICE_INFO ret=%u", __TAG__, ret);
            //            if (++attempCount > numAttemps)
            _state = APP_TCPIP_INIT;
            //            else
            //                _state = APP_LOAD_DEVICE_INFO;
            //            break;
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
                netName = TCPIP_STACK_NetNameGet(netH);
                netBiosName = TCPIP_STACK_NetBIOSName(netH);

                LOG_DEBUG("%s:\t TCP/IP stack\t Interface %s on host %s - NBNS disabled", __TAG__, netName, netBiosName);
            }

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
    }
}


/*******************************************************************************
 End of File
 */
