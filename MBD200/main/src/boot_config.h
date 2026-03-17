/* 
 * File:   boot_config.h
 * Author: Syxn
 *
 * Created on March 29, 2024, 5:26 PM
 */

#ifndef BOOT_CONFIG_H
#define	BOOT_CONFIG_H

#include <stdio.h>
#include "definitions.h"
//#include "config/default/library/tcpip/src/tcpip_helpers_private.h"

#define BOOT_CONFIG_START_ADDRESS       0x000000
#define BEGIN_ADDRESS_APPCOUNTER        0x003000
#define BEGIN_ADDRESS_APPRTU            0x004000
#define BEGIN_ADDRESS_ANALOG            0x005000
#define BEGIN_ADDRESS_GETSAMPLE         0x006000
#define BEGIN_ADDRESS_EXTEND            0x007000

#define FLASH_PAGE_SIZE                 DRV_SST26_PAGE_SIZE
#define MPFS_RESERVE_BLOCK              DRV_SST26_START_ADDRESS

#define MANUFACTURER                    "Bklogy JSC"
#define FW_CODE                         "v3.2.0"
#define HW_CODE                         "v2.2.0"
#define DATE                            "28-12-2024"
#define MODEL                           "MBD300C-DualSIM"
#define SERIAL                          "022500003"
#define DESCRIBE_DEVICE                 "Datalogger"

#define DEFAULT_USERNAME_DEVICE         "admin"
#define DEFAULT_PASSWORD_DEVICE         "admin"

#define MBRTU_BAUD_RATE                 9600
#define MBRTU_STOP_BITS                 0
#define MBRTU_PARITY                    0
#define MBRTU_TIMEOUT                   3000
#define MBRTU_RETRIES                   3
#define MBRTU_POLL_INTERVAL             1000
#define MBRTU_NUM_TIMEOUT               0

#define FTP_USER                        "admin"
#define FTP_PASS                        "admin"
#define FTP_PATH                        "/"
#define FTP_HOST                        "14.191.63.52"
#define FTP_PORT                        21

#define MY_APN                          "v-internet"
#define USERNAME_APN                    "admin"
#define PASSWORD_APN                    "admin"

#define SDCARD_TIME_REMOVE              3 //month

#define STRENGTH_BACK_LIGHT     50
#ifdef	__cplusplus
extern "C" {
#endif

    typedef enum {
        BOOT_CONFIG_INIT = 0,
        BOOT_CONFIG_BTN_HOLD,
        BOOT_CONFIG_WAIT,
        BOOT_CONFIG_WAIT_COMFIRM,
        BOOT_CONFIG_SAVE,
        BOOT_CONFIG_LOAD,
        BOOT_CONFIG_VALIDATE,
        BOOT_CONFIG_COMPLETE,
    } BOOT_CONFIG_STATES;

    typedef struct {
        BOOT_CONFIG_STATES state;
        DRV_HANDLE driverHandle;
        bool proactiveSaveFlag;
        uint32_t currentAddrFlash;
    } BOOT_CONFIG_DATA;

    typedef struct {
        unsigned short wConfigurationLength; // Number of bytes saved in EEPROM/Flash (sizeof(CONFIG))
        unsigned short wOriginalChecksum; // Checksum of the original AppConfig defaults as loaded from ROM (to detect when to wipe the EEPROM/Flash record of AppConfig due to a stack change, such as when switching from Ethernet to Wi-Fi)
        unsigned short wCurrentChecksum; // Checksum of the current EEPROM/Flash data.  This protects against using corrupt values if power failure occurs while writing them and helps detect coding errors in which some other task writes to the EEPROM in the AppConfig area.
    } NVM_VALIDATION_STRUCT;


    void BOOT_CONFIG_Initialize(void);
    bool BOOT_CONFIG_Tasks(void);
    void SaveAppConfig(bool forceSave);
    void SaveAppConfigCounter();
    void SaveAppConfigRtu();
    void SaveAppConfigAnalog();
    void SaveGetSample();
    void SaveExtendData();

#ifdef	__cplusplus
}
#endif

#endif	/* BOOT_CONFIG_H */

