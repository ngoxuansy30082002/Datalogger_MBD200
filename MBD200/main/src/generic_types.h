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
#define HASHCODE_SIZE                    16

#define SENSOR_NAME_LEN                 24
#define SENSOR_UNIT_LEN                 16
#define USERNAME_LEN                    48
#define PASSWORD_LEN                    48
#define FILE_NAME_LEN                   64
#define FILE_MAX_SIZE                   2048
#define FILE_NAME_PREFIX_LEN            32
#define DIR_PATH_LEN                    256
#define URL_LEN                         128
#define APN_LEN                         24
#define BIOS_NAME_LEN                   24

#define MAX_DIGITAL_OUTPUT              2
#define MAX_HMI_PARA                    20
#define MAX_ANALOG_CHANNEL              4
#define MAX_MODBUS_TAG                  14
#define MAX_INPUT_CAPTURE               4
#define MAX_SENSOR                      20
#define MAX_ROW_PER_PAGE                10
#define MAX_FTP_SERVER                  2
#define MAX_POSITION_SIZE               MAX_INPUT_CAPTURE + MAX_ANALOG_CHANNEL + MAX_MODBUS_TAG
#define OFFSET_POSITION_COUNTER         0        
#define OFFSET_POSITION_ANALOG          4   
#define OFFSET_POSITION_MBRTU           8


#if (SYS_FS_AUTOMOUNT_ENABLE)
#define SYS_FS_SPIFLASH_VOL                  SYS_FS_MEDIA_IDX1_DEVICE_NAME_VOLUME_IDX0
#define SYS_FS_SPIFLASH_MOUNT_POINT          SYS_FS_MEDIA_IDX1_MOUNT_NAME_VOLUME_IDX0
#define SYS_FS_SPIFLASH_TYPE                 MPFS2
#define SYS_FS_SPIFLASH_TYPE_STRING          "MPFS"
#else
#define SYS_FS_SPIFLASH_VOL                  "/dev/mtda1"
#define SYS_FS_SPIFLASH_MOUNT_POINT          "/mnt/web"
#define SYS_FS_SPIFLASH_TYPE                 MPFS2
#define SYS_FS_SPIFLASH_TYPE_STRING          "MPFS"
#endif

#if (SYS_FS_AUTOMOUNT_ENABLE)
#define SYS_FS_SDCARD_VOL                  SYS_FS_MEDIA_IDX0_DEVICE_NAME_VOLUME_IDX0
#define SYS_FS_SDCARD_MOUNT_POINT          SYS_FS_MEDIA_IDX0_MOUNT_NAME_VOLUME_IDX0
#define SYS_FS_SDCARD_TYPE                 FAT
#define SYS_FS_SDCARD_TYPE_STRING          "FAT"
#else
#define SYS_FS_SDCARD_VOL                 "/dev/mmcblka1"
#define SYS_FS_SDCARD_MOUNT_POINT          "/mnt/SDCard"
#define SYS_FS_SDCARD_TYPE                 FAT
#define SYS_FS_SDCARD_TYPE_STRING          "FAT"
#endif


#define TICK_NOW() SYS_TMR_TickCountGet()

#define MS_TO_TICK(ms) \
    ((SYS_TMR_TickCounterFrequencyGet() / 1000ul) * (ms))

#define TIME_IS_EXPIRED(startTick, intervalMs) \
    ((TICK_NOW() - (startTick)) >= (MS_TO_TICK(intervalMs)))

#ifdef	__cplusplus
extern "C" {
#endif

    typedef enum {
        UPLINK_ALL,
        UPLINK_ETH,
        UPLINK_GSM
    } INTERNET_UPLINK;

    typedef enum {
        FORMAT_FILE_TT24 = 0,
        FORMAT_FILE_DNA,
    } FORMAT_FILE;

    typedef enum {
        FILE_TYPE_TXT = 0,
        FILE_TYPE_CSV,
    } FILE_TYPE;

    typedef enum {
        SENSOR_NONE = 0,
        SENSOR_MBRTU,
        SENSOR_ANALOG,
        SENSOR_INPUT_CAPTURE,
    } SENSOR_TYPE;

    typedef enum {
        ADC_4_20mA = 0,
        ADC_0_10V
    } ADC_TYPE;

    typedef enum {
        MODBUS_RTU = 0,
        MODBUS_TCP
    } MODBUS_TYPE;

    typedef enum {
        BIG_ENDIAN_ABCD = 0,
        LITTLE_ENDIAN_CDAB,
        BIG_ENDIAN_SWAP_BADC,
        LITTLE_ENDIAN_SWAP_DCBA,
    } BYTE_ORDER_TYPE;

    typedef enum {
        OPERATOR_ADDITION,
        OPERATOR_SUBTRACTION,
        OPERATOR_MULTIPLICATION,
        OPERATOR_DIVISION,
    } OPERATOR;

    typedef enum {
        FROM_AUTO = 0,
        FROM_MBRTU,
        FROM_DIGITAL_INPUT,
    } STATUS_SOURCE;

    typedef enum {
        MAKE_FOLDER_NONE = 0,
        MAKE_FOLDER_DAY,
        MAKE_FOLDER_MONTH,
    } MAKE_FOLDER;

    typedef enum {
        TIME_MODE_OCLOCK = 0,
        TIME_MODE_CYCLE,
    } TIME_MODE;

    typedef enum {
        OUT_HOLD = 0,
        OUT_PULSE,
    } CTRL_MODE_TYPE;

    typedef enum {
        SCALE_NONE = 0,
        SCALE_LINEAR,
        SCALE_SQRT
    } SENSOR_SCALE_TYPE;

    typedef enum {
        DATA_UINT = 0,
        DATA_INT,
        DATA_FLOAT,
    } SENSOR_DATA_TYPE;

    typedef enum {
        STATUS_DISABLE = 0,
        STATUS_IDENTIFYING,
        STATUS_GOOD,
        STATUS_BAD
    } SENSOR_STATUS;

    typedef struct {
        uint8_t hour;
        uint8_t minute;
        uint8_t second;
        uint8_t day;
        uint8_t month;
        uint16_t year;
        uint8_t dayOfWeek;
    } TIME;

    typedef struct {
        char hostname[URL_LEN];
        uint16_t port;
        char username[USERNAME_LEN];
        char password[PASSWORD_LEN];
        char dirPath[DIR_PATH_LEN];
        MAKE_FOLDER makeFolder;
        bool enable;
    }
    FTP_SERVER_CONFIG;

    typedef struct {
        uint8_t retentionMonths;
        uint8_t lastMonth;
    }
    SDCARD_CONFIG;

    typedef struct {
        uint16_t timeout;
        uint8_t retries;
        uint32_t baudRate;
        uint16_t pollInterval;
        uint8_t parity;
        uint8_t stopbits;
        uint16_t latency;
    }
    MODBUSRTU_PHY_CONFIG;

    typedef struct {
        char usernameAPN[USERNAME_LEN];
        char passwordAPN[PASSWORD_LEN];
        char APN[APN_LEN];
    }
    GSM_CONFIG;

    typedef struct {
        bool syncNtpEnable;
        char ntpServerPrimary[URL_LEN];
        char ntpServerBackup[URL_LEN];
        uint32_t syncInterval;
        uint16_t ntpPort;
        int8_t timeZone;
        uint8_t yearNumber;
    }
    DATETIME_CONFIG;

    typedef struct {
        IP_ADDR ipAddr;
        IP_ADDR ipMask;
        IP_ADDR gateway;
        IP_ADDR primaryDNS;
        IP_ADDR secondDNS;
        char netBIOSName[BIOS_NAME_LEN]; // NetBIOS name
        bool isDHCPEn;

        char deviceUsername[USERNAME_LEN];
        char devicePassword[PASSWORD_LEN];
        INTERNET_UPLINK uplink;
    }
    NETWORK_CONFIG;

    typedef struct {
        char name[SENSOR_NAME_LEN];
        char describe[SENSOR_NAME_LEN];
        CTRL_MODE_TYPE mode;
        uint16_t ontime;
        uint16_t offtime;
        uint16_t pulseCount;
    }
    DO_CHANNEL_CONFIG;

    typedef struct {
        DO_CHANNEL_CONFIG out[MAX_DIGITAL_OUTPUT];
    }
    IO_CONFIG;

    typedef struct {
        uint8_t sensorIdx[MAX_HMI_PARA];
        uint8_t numEntry;
    } HMI_CONFIG;

    typedef struct {
        bool enable;
        SENSOR_TYPE type;
        uint8_t indexOfType;

        bool calibrate;
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
    } SENSOR_ENTRY_CONFIG;

    typedef struct {
        SENSOR_ENTRY_CONFIG entry[MAX_SENSOR];
        uint8_t numSensor;

        FORMAT_FILE formatFile;
        FILE_TYPE typefile;
        uint16_t logInterval;
        char filenameTemplate[FILE_NAME_LEN];
        bool compressed;
        bool uploadFtp;
        bool uploadMqtt;
        bool saveSdcard;
    }
    SENSOR_CONFIG;

    typedef struct {
        NETWORK_CONFIG network;
        FTP_SERVER_CONFIG ftpServer[MAX_FTP_SERVER];
        MODBUSRTU_PHY_CONFIG modbusRtu;
        GSM_CONFIG gsm;
        DATETIME_CONFIG time;
        IO_CONFIG io;
        SDCARD_CONFIG sdCard;

        HMI_CONFIG hmi;
        uint16_t position[MAX_POSITION_SIZE];
    }
    APP_CONFIG;

    typedef struct {
        bool enable;
        char name[SENSOR_NAME_LEN];
        char unit[SENSOR_UNIT_LEN];
        MODBUS_TYPE type;

        IPV4_ADDR ipAddress;
        uint16_t port;

        uint8_t slaveAddress;
        uint8_t function;
        uint16_t regAddress;
        uint8_t quantity;
        SENSOR_DATA_TYPE rawDataType;
        BYTE_ORDER_TYPE byteOder;

        bool conversion;
        float inputMin;
        float inputMax;
        float outputMin;
        float outputMax;

        SENSOR_SCALE_TYPE scaleType;
        SENSOR_DATA_TYPE scaleDataType;
        float scaleValue;

        float offsetPreVal;
        float offsetSubVal;
        OPERATOR offSetPreOperator;
        OPERATOR offsetSubOperator;
    } MODBUSRTU_TAG_ENTRY;

    typedef struct {
        MODBUSRTU_TAG_ENTRY entry[MAX_MODBUS_TAG];
        uint8_t numTag;
    } MODBUSRTU_TAG_CONFIG;

    typedef struct {
        bool enable;
        char name[SENSOR_NAME_LEN];
        char unit[SENSOR_UNIT_LEN];
        ADC_TYPE adcType;

        float inputLow;
        float inputHigh;
        float outputLow;
        float outputHigh;

        SENSOR_SCALE_TYPE scaleType;
        SENSOR_DATA_TYPE scaleDataType;
        float scaleValue;

        float offsetPreVal;
        float offsetSubVal;
        OPERATOR offSetPreOperator;
        OPERATOR offsetSubOperator;
    } ANALOG_CHANNEL_CONFIG;

    typedef struct {
        ANALOG_CHANNEL_CONFIG entry[MAX_ANALOG_CHANNEL];
    } ANALOG_CONFIG;

    typedef struct {
        bool enable;
        char name[SENSOR_NAME_LEN];
        char unit[SENSOR_UNIT_LEN];

        float valPerPulse;
        float minFreq;

        SENSOR_SCALE_TYPE scaleType;
        SENSOR_DATA_TYPE scaleDataType;
        float scaleValue;

        float offsetPreVal;
        float offsetSubVal;
        OPERATOR offSetPreOperator;
        OPERATOR offsetSubOperator;
    }
    INPUT_CAPTURE_CHANNEL_CONFIG;

    typedef struct {
        INPUT_CAPTURE_CHANNEL_CONFIG entry[MAX_INPUT_CAPTURE];
    }
    INPUT_CAPTURE_CONFIG;

    typedef struct {
        char manufacturer[MANUFACTURER_LEN];
        char fwVer[FW_CODE_LEN];
        char hwVer[HW_CODE_LEN];
        char dateTime[DATE_LEN];
        char model[MODEL_LEN];
        char serial[SERIAL_LEN];
        uint8_t fwHashCode[HASHCODE_SIZE];
    }
    DEVICE_INFO;

    extern DEVICE_INFO gDeviceInfo;
    extern APP_CONFIG gAppCfg;
    extern SENSOR_CONFIG gSensorCfg;
    extern ANALOG_CONFIG gAnalogCfg;
    extern MODBUSRTU_TAG_CONFIG gMbrtuCfg;
    extern INPUT_CAPTURE_CONFIG gInCaptureCfg;

    extern int lenLog;
    extern char logs[100];


#ifdef	__cplusplus
}
#endif

#endif	/* GENERIC_TYPES_H */