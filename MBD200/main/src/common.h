/* 
 * File:   common.h
 * Author: Syxn
 *
 * Created on March 28, 2024, 8:19 AM
 */


#ifndef COMMON_H
#define	COMMON_H

#include "config/default/library/tcpip/tcpip.h"

#define   STACK_USE_SD_CARD          
#define   STACK_USE_HMI          
#define   STACK_USE_NTP_GSM
#define   STACK_USE_GSM_FTP_CLIENT
#define   STACK_USE_ETH_FTP_CLIENT
#define   STACK_USE_ADC 
#define   STACK_USE_COUNTER_PULSE
#define   STACK_USE_RTC
#define   STACK_USE_GSM
#define   STACK_USE_MODBUS_TCP_SERVER  
#define   STACK_USE_GENERIC_TCP_SERVER

//#define     DEBUG_MODULE_ALL
#define     DEBUG_MODULE_BOOT_CFG
//#define     DEBUG_MODULE_ADC
//#define     DEBUG_MODULE_COUNTER
//#define     DEBUG_MODULE_HTTP
//#define     DEBUG_MODULE_ETH_FTP
//#define     DEBUG_MODULE_GSM
//#define     DEBUG_MODULE_MBRTU
//#define     DEBUG_MODULE_MBTCP
//#define     DEBUG_MODULE_SDCARD
#define     DEBUG_MODULE_MAIN
//#define     DEBUG_MODULE_DEE
//#define     DEBUG_MODULE_EXTEND

#define MANUFACTURER_SIZE (32u)
#define FW_CODE_SIZE      (16u)
#define HW_CODE_SIZE      (16u)
#define DATE_SIZE         (32u)
#define MODEL_SIZE        (32u)
#define SERIAL_SIZE       (32u)
#define DESCRIBE_SIZE     (32u)

#define MAX_HMI_PARA            (20u)
#define MAX_ANALOG_CHANNEL      (8u)
#define MAX_BUFFER_TAG          (14u)
#define MAX_COUNTER             (2u)
#define MAX_SENSOR              (20u)
#define NUM_ROW_PER_PAGE        (10u)
#define NUM_FTP_SERVER          2
#define MAX_NUM_FILE            2
#define FILE_QUEUE_SIZE              10
#define MAX_DESCRIPTION_LENGHT       (30u)

#define MAX_POSITION_SIZE               MAX_COUNTER * 2 + MAX_ANALOG_CHANNEL + MAX_BUFFER_TAG
#define OFFSET_POSITION_COUNTER         0        
#define OFFSET_POSITION_ANALOG          4   
#define OFFSET_POSITION_MBRTU           12

#define MAX_DIGITAL_INPUT           16
#define MAX_OUTPUT                  2

#define EXTEND_MAX_INPUT_OUTPUT     12
#define EXTEND_MAX_INPUT_CAPTURE    16
#define EXTEND_MAX_POSITION_SIZE    32

//#define MAX_ASSETS              (4u)
//#define ASSET_NAME_LENGTH       (12u)
//#define SENSOR_NAME_LENGTH      (12u)
//#define ASSET_DES_LENGTH        (20u)
//#define MAX_SENSOR_PER_ASSETS   (04u)


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
    } FORMATDATA;

    typedef enum {
        TXT = 0,
        CSV,
    } TYPEFILE;

    typedef enum {
        SENSOR_NONE = 0,
        SENSOR_RTU,
        SENSOR_ANALOG,
        SENSOR_COUNTER,
        SENSOR_EXT_COUNTER
    } SENSORTYPE;

    typedef enum {
        FROM_RTU = 0,
        FROM_DIGITAL_INPUT,
        FROM_EXTEND_DO,
        FROM_EXTEND_DI,
        FROM_NONE,
    } GET_STATUS_TYPE;

    typedef enum {
        NONE_FOLDER = 0,
        BY_DAY,
        BY_MONTH,
    } MAKEFOLDER;

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
    } TIMEMODE;

    typedef enum {
        HOLD = 0,
        PULSE,
    } TYPE_CTRL_OUT;

    typedef enum {
        GOOD = 0,
        CALIBRATION,
        BAD
    } STATUS;

    typedef enum {
        GSM_SIM1 = 0,
        GSM_SIM2,
        GSM_DUAL_SIM,
    } GSM_SIM_MODE;

    typedef enum {
        EXTEND_IO_DISABLE = 0,
        EXTEND_IO_DIGITAL_INPUT,
        EXTEND_IO_DIGITAL_OUTPUT,
        EXTEND_IO_INPUT_CAPTURE
    } EXTEND_IO_MODE;

    typedef struct __attribute__((__packed__)) {
        char username[20];
        char password[40];
        char path[200];
        char hostname[50];
        uint8_t port;
        char namePrefix[50];
        MAKEFOLDER makeFolder;
        bool enable;
    }
    FTP_SERVER_CONFIG;

    typedef struct __attribute__((__packed__)) {
        UPLINK uplink;
        FORMATDATA formatData;
        TYPEFILE typefile;
        TIMEMODE timeMode;
        uint16_t SendInterval;
    }
    FTP_GENERAL_CONFIG;

    typedef struct __attribute__((__packed__)) {
        uint16_t timeout;
        uint8_t retries;
        uint32_t baudRate;
        uint16_t pollInterval;
        uint8_t parity;
        uint8_t stopbits;
        uint16_t latency;
    }
    MODBUSRTU_CONFIG;

    typedef struct __attribute__((__packed__)) {
        char usernameAPN[15];
        char passAPN[15];
        char APN[15];
        GSM_SIM_MODE mode;
    }
    GSM_CONFIG;

    typedef struct __attribute__((__packed__)) {
        char Timezone[10];
        uint8_t indexNTP;
        uint8_t Time_auto;
        uint8_t Year_number;
        char Timeset[50];
    }
    TIME_CONFIG;

    typedef struct __attribute__((__packed__)) {
        IP_ADDR ipAddr;
        IP_ADDR defaultIpAddr;
        IP_ADDR ipMask;
        IP_ADDR defaultIpMask;
        IP_ADDR gateway;
        IP_ADDR priDNS;
        IP_ADDR secondDNS;
        char NetBIOSName[16]; // NetBIOS name

        char app_username_device[24];
        char app_password_device[24];

        bool isDHCPEn;
    }
    NETWORK_CONFIG;

    typedef struct __attribute__((__packed__)) {
        char describeIN1[MAX_DESCRIPTION_LENGHT];
        char describeIN2[MAX_DESCRIPTION_LENGHT];
        char describeIN3[MAX_DESCRIPTION_LENGHT];
        char describeIN4[MAX_DESCRIPTION_LENGHT];
        char describeIN5[MAX_DESCRIPTION_LENGHT];
        char describeIN6[MAX_DESCRIPTION_LENGHT];
        char describeIN7[MAX_DESCRIPTION_LENGHT];
        char describeIN8[MAX_DESCRIPTION_LENGHT];
        char describeIN9[MAX_DESCRIPTION_LENGHT];
        char describeIN10[MAX_DESCRIPTION_LENGHT];
        char describeIN11[MAX_DESCRIPTION_LENGHT];
        char describeIN12[MAX_DESCRIPTION_LENGHT];
        char describeIN13[MAX_DESCRIPTION_LENGHT];
        char describeIN14[MAX_DESCRIPTION_LENGHT];
        char describeIN15[MAX_DESCRIPTION_LENGHT];
        char describeIN16[MAX_DESCRIPTION_LENGHT];

        char describeOUT1[MAX_DESCRIPTION_LENGHT];
        char describeOUT2[MAX_DESCRIPTION_LENGHT];
        uint16_t timeCtrlOut1;
        uint16_t timeCtrlOut2;
        TYPE_CTRL_OUT typeCtrlOut1;
        TYPE_CTRL_OUT typeCtrlOut2;
    }
    IO_CONFIG;

    typedef struct __attribute__((__packed__)) {
        uint8_t whichOut;
        uint8_t maxBottle;
        char remotePath[50];
        char remoteHost[50];
        uint16_t remotePort;
        char localPath[50];
        uint16_t localPort;
        uint8_t sampleTime;
        uint8_t bottleIdx;
    }
    SAMPLER_API_CONFIG;

    typedef struct __attribute__((__packed__)) {
        uint8_t timeRemove;
        uint8_t lastMonth;
    }
    SDCARD_CONFIG;
    // Application-dependent structure used to contain address information

    typedef struct __attribute__((__packed__)) {

        struct __attribute__((__packed__)) {
            SENSORTYPE type; //loai sensor (mb,adc,counter)
            uint8_t idxInType; // thu tu cua ss do trong nhom loai cua no (mb1,mb2, adc1,adc3...)

            bool calibrated;

            GET_STATUS_TYPE typeRun;
            uint8_t idxInTypeRun;
            GET_STATUS_TYPE typeCalib;
            uint8_t idxInTypeCalib;
            GET_STATUS_TYPE typeErr;
            uint8_t idxInTypeErr;

            GET_STATUS_TYPE typeStatus;
            uint16_t runvalueAND;
            uint16_t runvalueCompare;

            uint16_t calibvalueAND;
            uint16_t calibvalueCompare;

            uint16_t errorvalueAND;
            uint16_t errorvalueCompare;
            bool enable;
        }
        entry[MAX_SENSOR];
        uint8_t total_sensor;
    }
    SENSOR_CONFIG;

    typedef struct __attribute__((__packed__)) {
        NETWORK_CONFIG network;
        FTP_SERVER_CONFIG ftpServer[NUM_FTP_SERVER];
        FTP_GENERAL_CONFIG ftpGeneral;
        MODBUSRTU_CONFIG modbusRTU;
        GSM_CONFIG GSM;
        TIME_CONFIG time;
        IO_CONFIG io;
        SAMPLER_API_CONFIG sampleApi;
        SDCARD_CONFIG sdCard;
        SENSOR_CONFIG sensor;

        uint8_t strengthBackLight;
        uint8_t tag_hmi[MAX_HMI_PARA];

        uint16_t position[MAX_POSITION_SIZE]; //ModbusTCP server
    }
    APP_CONFIG;

    typedef struct __attribute__((__packed__)) {

        struct __attribute__((__packed__)) {
            uint8_t addr;
            uint8_t func;
            uint16_t addr_reg;
            uint8_t bytes;
            uint8_t type;
            bool enable;
            bool big_endian;
        }
        app_rtu_table[MAX_BUFFER_TAG];

        struct __attribute__((__packed__)) {
            char unit[11];
            char des[16];

            //scale
            uint8_t scale_type;
            uint8_t scaled_data_type;
            float scale_value;

            //ADC
            uint8_t ADCtype;
            float ADClow;
            float ADChigh;
            float ADCofset_pre;
            float ADCofset_sub;
            uint8_t ADCtypepre;
            uint8_t ADCtypesub;
        }
        analog_modbus[MAX_BUFFER_TAG];

        uint8_t total_row;
    }
    APP_RTU_TAG;

    typedef struct __attribute__((__packed__)) {

        struct __attribute__((__packed__)) {
            char unit[8];
            char des[15];
            bool enable;
            //scale
            uint8_t scale_type;
            uint8_t scaled_data_type;
            float scale_value;
            //ADC
            uint8_t ADCtype;
            float ADClow;
            float ADChigh;
            float ADCofset_pre;
            float ADCofset_sub;
            uint8_t ADCtypepre;
            uint8_t ADCtypesub;

        }
        entry[MAX_ANALOG_CHANNEL];
    }
    APP_ANALOG;

    typedef struct __attribute__((__packed__)) {
        char manufacturer[MANUFACTURER_SIZE]; //
        char fw_code[FW_CODE_SIZE]; //
        char hw_code[HW_CODE_SIZE]; // 
        char date[DATE_SIZE]; //
        char model[MODEL_SIZE]; //
        char serial[SERIAL_SIZE]; //
        char describe_device[DESCRIBE_SIZE];
    }
    APP_CONFIG_DEVICE;

    //    typedef struct __attribute__((__packed__)) {
    //
    //        struct __attribute__((__packed__)) {
    //            char name[ASSET_NAME_LENGTH];
    //            char des[ASSET_DES_LENGTH];
    //
    //            struct __attribute__((__packed__)) {
    //                char sensorname[SENSOR_NAME_LENGTH];
    //                SENSORTYPE sentype; //SENSOR TYPE
    //                uint8_t sid; //sensor ID
    //                uint8_t stateDaily;
    //                char dailyName[SENSOR_NAME_LENGTH];
    //                float delta;
    //                uint16_t time;
    //                float totalFlowNow;
    //            }
    //
    //            sensor[MAX_SENSOR_PER_ASSETS];
    //            uint8_t sensortotal;
    //            bool state;
    //        }
    //        assets[MAX_ASSETS];
    //        uint8_t assettotal;
    //    }
    //    APP_CONFIG_ASSETS;

    typedef struct __attribute__((__packed__)) {

        struct __attribute__((__packed__)) {
            char name[24];
            char unit[10];
            double pulse;
            float minFreq;
            bool enable;
            double scale;
        }
        counter[MAX_COUNTER * 2];
    }
    APP_COUNTER;

    typedef struct __attribute__((__packed__)) {
        bool isErr;
        uint8_t countErr;
        char newId[100];
        char iso_time[32];
        uint8_t bottleIdx;
    }
    GET_SAMPLE;

    typedef struct __attribute__((__packed__)) {
        bool enable;
        uint8_t address;
        uint16_t position[EXTEND_MAX_POSITION_SIZE]; //ModbusTCP server

        struct __attribute__((__packed__)) {
            char des[MAX_DESCRIPTION_LENGHT];
            EXTEND_IO_MODE mode;
        }
        inputOutput[EXTEND_MAX_INPUT_OUTPUT];

        struct __attribute__((__packed__)) {
            char des[MAX_DESCRIPTION_LENGHT];
            EXTEND_IO_MODE mode;

            struct __attribute__((__packed__)) {
                char name[24];
                char unit[10];
                float pulse;
                float minFreq;
                bool enable;
                float scale;
            }
            pulseRate;

            struct __attribute__((__packed__)) {
                char name[24];
                char unit[10];
                float pulse;
                bool enable;
            }
            counter;
        }
        inputCapture[EXTEND_MAX_INPUT_CAPTURE];
    }
    EXTEND_CONFIG;

    extern APP_CONFIG glbAppCfg;
    extern APP_ANALOG glbAppAnlg;
    extern APP_CONFIG_DEVICE glbAppDev;
    //    extern APP_CONFIG_ASSETS glbAppAsset;
    extern APP_RTU_TAG glbAppRtu;
    extern APP_COUNTER glbAppCnter;

    extern GET_SAMPLE glbGetSample;
    extern EXTEND_CONFIG glbExtend;

    extern int lenLog;
    extern char logs[100];

    void CreateTargetPath(char * targetPath, size_t pathLen, uint8_t idxFtpSrv, TIME time);

#ifdef	__cplusplus
}
#endif

#endif	/* COMMON_H */