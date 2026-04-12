/* 
 * File:   external_flash.h
 * Author: LENOVO
 *
 * Created on September 26, 2025, 3:01 PM
 */

#ifndef EXTERNAL_FLASH_H
#define	EXTERNAL_FLASH_H

#include <stdio.h>
#include <string.h>
#include "device_cache.h"
#include "definitions.h"

#define EXTFL_QUEUE_SIZE                16
#define EXTFL_PAGE_SIZE                 DRV_SST26_PAGE_SIZE
#define EXTFL_NUM_PARTITION             10
#define EXTFL_TIMEOUT_TRANSFER          500 //ms
#define EXTFL_MAGIC                     0xC840 // FLASH_GD25WQ128E

#ifdef	__cplusplus
extern "C" {
#endif

    typedef struct __attribute__((__packed__)) {
        uint16_t magic;
        uint16_t length;
        int type;
        uint32_t crc;
    }
    EXTFL_METADATA;

    typedef struct __attribute__((__packed__)) {
        char username[USERNAME_LEN];
        char password[PASSWORD_LEN];
        char dirPath[DIR_PATH_LEN];
        char hostname[URL_LEN];
        uint16_t port;
        char namePrefix[FILE_NAME_PREFIX_LEN];
        MAKE_FOLDER makeFolder;
        bool enable;
    }
    FTP_SERVER_PACKED;

    typedef struct __attribute__((__packed__)) {
        uint8_t retentionMonths;
        uint8_t lastMonth;
    }
    SDCARD_PACKED;

    typedef struct __attribute__((__packed__)) {
        INTERNET_UPLINK uplink;
        FORMAT_FILE formatFile;
        FILE_TYPE typefile;
        TIME_MODE timeMode;
        uint16_t sendInterval;
    }
    LOG_FILE_PACKED;

    typedef struct __attribute__((__packed__)) {
        uint16_t timeout;
        uint8_t retries;
        uint32_t baudRate;
        uint16_t pollInterval;
        uint8_t parity;
        uint8_t stopbits;
        uint16_t latency;
    }
    MODBUSRTU_PHY_PACKED;

    typedef struct __attribute__((__packed__)) {
        char usernameAPN[USERNAME_LEN];
        char passwordAPN[PASSWORD_LEN];
        char APN[APN_LEN];
    }
    GSM_PACKED;

    typedef struct __attribute__((__packed__)) {
        uint8_t indexNTP;
        uint8_t timeZone;
        bool timeAuto;
        uint8_t yearNumber;
    }
    DATETIME_PACKED;

    typedef struct __attribute__((__packed__)) {
        IP_ADDR ipAddr;
        IP_ADDR ipMask;
        IP_ADDR gateway;
        IP_ADDR primaryDNS;
        IP_ADDR secondDNS;
        char netBIOSName[BIOS_NAME_LEN]; // NetBIOS name
        bool isDHCPEn;

        char deviceUsername[USERNAME_LEN];
        char devicePassword[PASSWORD_LEN];
    }
    NETWORK_PACKED;

    typedef struct __attribute__((__packed__)) {

        struct __attribute__((__packed__)) {
            char describe[SENSOR_NAME_LEN];
            uint16_t time;
            CTRL_OUT_TYPE type;
        }
        out[MAX_DIGITAL_OUTPUT];
    }
    IO_PACKED;

    typedef struct __attribute__((__packed__)) {
        EXTFL_METADATA metadata;

        NETWORK_PACKED network;
        FTP_SERVER_PACKED ftpServer[MAX_FTP_SERVER];
        LOG_FILE_PACKED logFile;
        MODBUSRTU_PHY_PACKED modbusRtu;
        GSM_PACKED gsm;
        DATETIME_PACKED time;
        IO_PACKED io;
        SDCARD_PACKED sdCard;

        uint8_t hmi[MAX_HMI_PARA];
        uint16_t position[MAX_POSITION_SIZE];
    }
    APP_PACKED;

    typedef struct __attribute__((__packed__)) {
        EXTFL_METADATA metadata;

        struct __attribute__((__packed__)) {
            bool enable;
            SENSOR_TYPE type;
            uint8_t indexOfType;

            bool calibrated;
            STATUS_SOURCE typeStatus;

            STATUS_SOURCE typeGood;
            uint8_t indexOfTypeGood;
            STATUS_SOURCE typeCalib;
            uint8_t indexOfTypeCalib;
            STATUS_SOURCE typeErr;
            uint8_t indexOfTypeErr;

            uint16_t goodValueAND;
            uint16_t goodValueCompare;
            uint16_t calibValueAND;
            uint16_t calibValueCompare;
            uint16_t errorValueAND;
            uint16_t errorValueCompare;
        }
        entry[MAX_SENSOR];
        uint8_t numSensor;
    }
    SENSOR_PACKED;

    typedef struct __attribute__((__packed__)) {
        EXTFL_METADATA metadata;

        struct __attribute__((__packed__)) {
            bool enable;
            char name[SENSOR_NAME_LEN];
            char unit[SENSOR_UNIT_LEN];

            uint8_t slaveAddress;
            uint8_t function;
            uint16_t regAddress;
            uint8_t quantity;
            SENSOR_DATA_TYPE rawDataType;
            bool bigEndian;

            SENSOR_SCALE_TYPE scaleType;
            SENSOR_DATA_TYPE scaleDataType;
            float scaleValue;

            ADC_TYPE adcType;
            float adcLow;
            float adcHigh;
            float offsetPreVal;
            float offsetSubVal;
            OPERATOR offSetPreOperator;
            OPERATOR offsetSubOperator;
        }
        entry[MAX_MODBUS_TAG];

        uint8_t numTag;
    }
    MODBUSRTU_TAG_PACKED;

    typedef struct __attribute__((__packed__)) {
        EXTFL_METADATA metadata;

        struct __attribute__((__packed__)) {
            bool enable;
            char name[SENSOR_NAME_LEN];
            char unit[SENSOR_UNIT_LEN];

            SENSOR_SCALE_TYPE scaleType;
            SENSOR_DATA_TYPE scaleDataType;
            float scaleValue;

            ADC_TYPE adcType;
            float adcLow;
            float adcHigh;
            float offsetPreVal;
            float offsetSubVal;
            OPERATOR offSetPreOperator;
            OPERATOR offsetSubOperator;
        }
        entry[MAX_ANALOG_CHANNEL];
    }
    ANALOG_PACKED;

    typedef struct __attribute__((__packed__)) {
        EXTFL_METADATA metadata;

        struct __attribute__((__packed__)) {
            bool enable;
            char name[SENSOR_NAME_LEN];
            char unit[SENSOR_UNIT_LEN];

            float valPerPulse;
            float minFreq;
            float scale;
        }
        entry[MAX_INPUT_CAPTURE];
    }
    INPUT_CAPTURE_PACKED;

    typedef enum {
        EXTFL_DATA_APP_CFG = 0,
        EXTFL_DATA_SENSOR_CFG,
        EXTFL_DATA_ANALOG_CFG,
        EXTFL_DATA_MBRTU_CFG,
        EXTFL_DATA_INCAPTURE_CFG,
    } EXTFL_DATA_TYPE;

    typedef enum {
        EXTFL_SUCCESS = 0,
        EXTFL_TRANSFER_TIMEOUT,
        EXTFL_ERASE_FAIL,
        EXTFL_WRITE_FAIL,
        EXTFL_READ_FAIL,
        EXTFL_VERIFY_FAIL
    } EXTFL_RESULT;

    typedef enum {
        EXTFL_WRITE_IDLE = 0,
        EXTFL_WRITE_ERASE_SECTOR,
        EXTFL_WRITE_TRANSFER,
        EXTFL_WRITE_DONE,
    } EXTFL_WRITE_STATE;

    typedef enum {
        EXTFL_READ_IDLE = 0,
        EXTFL_READ_TRANSFER,
        EXTFL_READ_WAIT_TRANSFER,
        EXTFL_READ_VERIFY_DATA,
        EXTFL_READ_DONE
    } EXTFL_READ_STATE;

    typedef struct {
        EXTFL_DATA_TYPE type;
        uint32_t startAddress;
    } EXTFL_PARTITION;

    typedef struct {
        EXTFL_DATA_TYPE type;
        uint16_t size;
        void (*callback)(int type, int result);
    } EXTFL_QUEUE_ENTRY;


    void ExtFlash_Initialize();
    void ExtFlash_Task();
    bool ExtFlash_SaveConfig(EXTFL_DATA_TYPE dataType, void (*clb)(int type, int rlst));
    bool ExtFlash_LoadConfig(EXTFL_DATA_TYPE dataType, void (*clb)(int type, int rlst));

#ifdef	__cplusplus
}
#endif

#endif	/* EXTERNAL_FLASH_H */

