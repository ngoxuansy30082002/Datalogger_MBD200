/* 
 * File:   generic_types.h
 * Author: Syxn
 *
 * Created on March 28, 2024, 8:19 AM
 */


#ifndef GENERIC_TYPES_H
#define	GENERIC_TYPES_H

#include "config/default/library/tcpip/tcpip.h"

#define   STACK_USE_GSM
#define   STACK_USE_GSM_NTP
#define   STACK_USE_GSM_FTP
#define   STACK_USE_ETH_NTP
#define   STACK_USE_ETH_FTP
#define   STACK_USE_SD_CARD          
#define   STACK_USE_HMI          
#define   STACK_USE_ANALOG 
#define   STACK_USE_INPUT_CAPTURE
#define   STACK_USE_RTC
#define   STACK_USE_MODBUS_TCP  
#define   STACK_USE_GENERIC_TCP

//#define     DEBUG_MODULE_ALL
//#define     DEBUG_MODULE_BOOT_CFG
//#define     DEBUG_MODULE_ADC
//#define     DEBUG_MODULE_COUNTER
//#define     DEBUG_MODULE_HTTP
//#define     DEBUG_MODULE_ETH_FTP
//#define     DEBUG_MODULE_GSM
//#define     DEBUG_MODULE_MBRTU
//#define     DEBUG_MODULE_MBTCP
//#define     DEBUG_MODULE_SDCARD
//#define     DEBUG_MODULE_MAIN
//#define     DEBUG_MODULE_DEE

#define MANUFACTURER_LEN                32
#define FW_CODE_LEN                     16
#define HW_CODE_LEN                     16
#define DATE_LEN                        32
#define MODEL_LEN                       32
#define SERIAL_LEN                      32
#define SENSOR_NAME_LEN                 24
#define SENSOR_UNIT_LEN                 16
#define USERNAME_LEN                    48
#define PASSWORD_LEN                    48
#define FILE_NAME_LEN                   64
#define FILE_NAME_PREFIX_LEN            32
#define DIR_PATH_LEN                    256
#define URL_LEN                         128
#define APN_LEN                         24
#define BIOS_NAME_LEN                   24

#define MAX_DIGITAL_OUTPUT              2
#define MAX_HMI_PARA                    20
#define MAX_ANALOG_CHANNEL              4
#define MAX_BUFFER_TAG                  14
#define MAX_INPUT_CAPTURE               4
#define MAX_SENSOR                      20
#define MAX_ROW_PER_PAGE                10
#define MAX_FTP_SERVER                  2
#define MAX_POSITION_SIZE               MAX_INPUT_CAPTURE + MAX_ANALOG_CHANNEL + MAX_BUFFER_TAG
#define OFFSET_POSITION_COUNTER         0        
#define OFFSET_POSITION_ANALOG          4   
#define OFFSET_POSITION_MBRTU           8


#ifdef	__cplusplus
extern "C" {
#endif

    typedef enum {
        NONE = 0,
        ALL,
        ETH,
        GSM
    } UPLINK;

    typedef enum {
        TT24 = 0,
        DNA,
    } FORMAT_DATA;

    typedef enum {
        TXT = 0,
        CSV,
    } FILE_TYPE;

    typedef enum {
        SENSOR_NONE = 0,
        SENSOR_MBRTU,
        SENSOR_ANALOG,
        SENSOR_INPUT_CAPTURE,
    } SENSOR_TYPE;

    typedef enum {
        FROM_NONE = 0,
        FROM_MBRTU,
        FROM_DIGITAL_INPUT,
    } STATUS_SOURCE;

    typedef enum {
        NONE_FOLDER = 0,
        BY_DAY,
        BY_MONTH,
    } MAKE_FOLDER;

    typedef struct {
        uint8_t hour;
        uint8_t minute;
        uint8_t second;
        uint8_t day;
        uint8_t month;
        uint16_t year;
        uint8_t dayOfWeek;
    } TIME;

    typedef enum {
        OCLOCK = 0,
        CYCLE,
    } TIME_MODE;

    typedef enum {
        HOLD = 0,
        PULSE,
    } CTRL_OUT_TYPE;

    typedef enum {
        GOOD = 0,
        CALIBRATION,
        BAD
    } SENSOR_STATUS;

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
    FTP_SERVER_CONFIG;

    typedef struct __attribute__((__packed__)) {
        uint8_t retentionMonths;
        uint8_t lastMonth;
    }
    SDCARD_CONFIG;

    typedef struct __attribute__((__packed__)) {
        UPLINK uplink;
        FORMAT_DATA formatData;
        FILE_TYPE typefile;
        TIME_MODE timeMode;
        uint16_t sendInterval;
    }
    LOG_FILE_CONFIG;

    typedef struct __attribute__((__packed__)) {
        uint16_t timeout;
        uint8_t retries;
        uint32_t baudRate;
        uint16_t pollInterval;
        uint8_t parity;
        uint8_t stopbits;
        uint16_t latency;
    }
    MODBUSRTU_PHY_CONFIG;

    typedef struct __attribute__((__packed__)) {
        char usernameAPN[USERNAME_LEN];
        char passwordAPN[PASSWORD_LEN];
        char APN[APN_LEN];
    }
    GSM_CONFIG;

    typedef struct __attribute__((__packed__)) {
        uint8_t indexNTP;
        bool timeAuto;
        uint8_t yearNumber;
    }
    TIME_CONFIG;

    typedef struct __attribute__((__packed__)) {
        IP_ADDR ipAddr;
        IP_ADDR defaultIpAddr;
        IP_ADDR ipMask;
        IP_ADDR defaultIpMask;
        IP_ADDR gateway;
        IP_ADDR primaryDNS;
        IP_ADDR secondDNS;
        char netBIOSName[BIOS_NAME_LEN]; // NetBIOS name
        bool isDHCPEn;

        char deviceUsername[USERNAME_LEN];
        char devicePassword[PASSWORD_LEN];
    }
    NETWORK_CONFIG;

    typedef struct __attribute__((__packed__)) {
        char describeOUT1[SENSOR_NAME_LEN];
        char describeOUT2[SENSOR_NAME_LEN];
        uint16_t timeCtrlOut1;
        uint16_t timeCtrlOut2;
        CTRL_OUT_TYPE typeCtrlOut1;
        CTRL_OUT_TYPE typeCtrlOut2;
    }
    IO_CONFIG;

    typedef struct __attribute__((__packed__)) {

        struct __attribute__((__packed__)) {
            bool enable;
            SENSOR_TYPE type; //loai sensor (mb,adc,counter)
            uint8_t indexOfType; // thu tu cua ss do trong nhom loai cua no (mb1,mb2, adc1,adc3...)

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
        uint8_t total_sensor;
    }
    SENSOR_CONFIG;

    typedef struct __attribute__((__packed__)) {
        NETWORK_CONFIG network;
        FTP_SERVER_CONFIG ftpServer[MAX_FTP_SERVER];
        LOG_FILE_CONFIG logFile;
        MODBUSRTU_PHY_CONFIG modbusRtu;
        GSM_CONFIG gsm;
        TIME_CONFIG time;
        IO_CONFIG io;
        SDCARD_CONFIG sdCard;
        SENSOR_CONFIG sensor;

        uint8_t hmi[MAX_HMI_PARA];
        uint16_t position[MAX_POSITION_SIZE];
    }
    APP_CONFIG;

    typedef struct __attribute__((__packed__)) {

        struct __attribute__((__packed__)) {
            bool enable;
            uint8_t slaveAddress;
            uint8_t function;
            uint16_t addrRegister;
            uint8_t quantity;
            uint8_t dataType;
            bool bigEndian;

            char name[SENSOR_NAME_LEN];
            char unit[SENSOR_UNIT_LEN];

            uint8_t scaleType;
            uint8_t scaledDataType;
            float scaleValue;

            uint8_t adcType;
            float adcLow;
            float adcHigh;
            float adcOffsetPre;
            float adcOffsetSub;
            uint8_t adcTypePre;
            uint8_t adcTypeSub;
        }
        entry[MAX_BUFFER_TAG];

        uint8_t numTag;
    }
    MODBUS_RTU_TAG;

    typedef struct __attribute__((__packed__)) {

        struct __attribute__((__packed__)) {
            bool enable;
            char name[SENSOR_NAME_LEN];
            char unit[SENSOR_UNIT_LEN];

            uint8_t scaleType;
            uint8_t scaledDataType;
            float scaleValue;

            uint8_t adcType;
            float adcLow;
            float adcHigh;
            float adcOffsetPre;
            float adcOffsetSub;
            uint8_t adcTypePre;
            uint8_t adcTypeSub;

        }
        entry[MAX_ANALOG_CHANNEL];
    }
    ANALOG;

    typedef struct __attribute__((__packed__)) {

        struct __attribute__((__packed__)) {
            bool enable;
            char name[SENSOR_NAME_LEN];
            char unit[SENSOR_UNIT_LEN];
            float pulse;
            float minFreq;
            float scale;
        }
        counter[MAX_INPUT_CAPTURE];
    }
    INPUT_CAPTURE;

    typedef struct __attribute__((__packed__)) {
        char manufacturer[MANUFACTURER_LEN];
        char fw_code[FW_CODE_LEN];
        char hw_code[HW_CODE_LEN];
        char date[DATE_LEN];
        char model[MODEL_LEN];
        char serial[SERIAL_LEN];
    }
    DEVICE_INFO;

    extern APP_CONFIG appCfg;
    extern ANALOG analogCfg;
    extern DEVICE_INFO deviceInfo;
    extern MODBUS_RTU_TAG mbrtuCfg;
    extern INPUT_CAPTURE inCaptureCfg;

    extern int lenLog;
    extern char logs[100];


#ifdef	__cplusplus
}
#endif

#endif	/* GENERIC_TYPES_H */