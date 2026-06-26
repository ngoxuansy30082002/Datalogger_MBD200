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

#define FLASH_PAGE_SIZE                 DRV_SST26_PAGE_SIZE
#define MPFS_RESERVE_BLOCK              DRV_SST26_START_ADDRESS

#define MANUFACTURER                    "Bklogy JSC"
#define FIRMWARE_VERSION                "v3.0.0"
#define HARDWARE_VERSION                "v2.1.0"
#define DATE_TIME                       "25-06-2026"
#define MODEL                           "MBD200"
#define SERIAL_NUMBER                   "BKT-MBD-022500003"
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
#define FTP_NAME_PREFIX                 "LOGs"

#define MY_APN                          "v-internet"
#define USERNAME_APN                    "admin"
#define PASSWORD_APN                    "admin"

#define NTP_SERVER_PRIMARY              "pool.ntp.org"
#define NTP_SERVER_BACKUP               "time.google.com"

#define SDCARD_TIME_REMOVE              3 //month

#define STRENGTH_BACK_LIGHT     50
#ifdef	__cplusplus
extern "C" {
#endif

    typedef struct {
        GPIO_PIN button;
        GPIO_PIN led1;
        GPIO_PIN led2;
        GPIO_PIN led3;
        GPIO_PIN led4;
    } BOOT_CONFIG_PLIB;

    typedef enum {
        BOOT_CONFIG_INIT = 0,
        BOOT_CONFIG_BTN_HOLD,
        BOOT_CONFIG_WAIT,
        BOOT_CONFIG_WAIT_COMFIRM,
        BOOT_CONFIG_SAVE,
        BOOT_CONFIG_WAIT_SAVE,
        BOOT_CONFIG_LOAD,
        BOOT_CONFIG_WAIT_LOAD,
        BOOT_CONFIG_VALIDATE,
        BOOT_CONFIG_COMPLETE,
        BOOT_CONFIG_FAULT,
    } BOOT_CONFIG_STATES;

    typedef union {

        struct {
            uint16_t appCfg : 1;
            uint16_t sensorCfg : 1;
            uint16_t analogCfg : 1;
            uint16_t mbRtuCfg : 1;
            uint16_t inCaptureCfg : 1;
            uint16_t reserved : 11;
        } bits;

        uint16_t val;
    } BOOT_CONFIG_FLAG;

    void BootConfig_Initialize(void);
    bool BootConfig_Task(void);

#ifdef	__cplusplus
}
#endif

#endif	/* BOOT_CONFIG_H */

