/* * File:   hmi_dwin.h
 * Author: LENOVO
 *
 * Created on December 30, 2025, 9:03 PM
 */

#ifndef HMI_DWIN_H
#define	HMI_DWIN_H

#include <stdio.h>
#include <string.h>
#include "definitions.h"
#include "generic_types.h"

#define HMI_FRAME_HEADER1               0x5A
#define HMI_FRAME_HEADER2               0xA5
#define HMI_INSTRUCTION_WRITE           0x82
#define HMI_INSTRUCTION_READ            0x83
#define HMI_HEADER_LEN                  6

#define HMI_ADDR_PIC_SET                0x0084
#define HMI_ADDR_DATETIME               0x5025 // 
#define HMI_ADDR_SIGNAL1                0x5015 
#define HMI_ADDR_NETWORK1               0x5016 
//#define HMI_ADDR_SIGNAL2                0x501D 
//#define HMI_ADDR_NETWORK2               0x501E
//#define HMI_ADDR_SDCARD_STS             0x5050
//#define HMI_ADDR_FTP_STS                0x5060
//#define HMI_ADDR_SIM1_STS               0x5070
//#define HMI_ADDR_SIM2_STS               0x5080
#define HMI_ADDR_DEVICE_INFO            0x50A0

#define HMI_ADDR_ROW_NAME(row)          (0x1003u + ((uint16_t)(row) << 4))
#define HMI_ADDR_ROW_VALUE(row)         (0x2003u + ((uint16_t)(row) << 4))
#define HMI_ADDR_ROW_UNIT(row)          (0x3003u + ((uint16_t)(row) << 4))
#define HMI_ADDR_ROW_STATUS(row)        (0x4001u + ((uint16_t)(row) << 1))


#define HMI_QUEUE_SIZE                  50

#ifdef	__cplusplus
extern "C" {
#endif

    typedef enum {
        HMI_TAG_SWITCH_PAGE = 0,
        HMI_TAG_DATETIME,
        HMI_TAG_NETWORK_SIGNAL,
        HMI_TAG_DEVICE_STATUS,
        HMI_TAG_DEVICE_INFO,

        HMI_TAG_ROW_STATUS,
        HMI_TAG_ROW_VALUE,
        HMI_TAG_PAGE1_ROW_NAME,
        HMI_TAG_PAGE1_ROW_UNIT,
        HMI_TAG_PAGE2_ROW_NAME,
        HMI_TAG_PAGE2_ROW_UNIT,


        HMI_TAG_MAX_COUNT,
    } HMI_TAG_TYPE;

    typedef enum {
        HMI_DATA_INT = 0,
        HMI_DATA_UINT,
        HMI_DATA_FLOAT,
        HMI_DATA_NONE,
    } HMI_TAG_DATA_TYPE;

    typedef void (* HMI_UART_CALLBACK)(uintptr_t context);
    typedef bool (* HMI_UART_READ)(void* pReceiveData, size_t rxSize);
    typedef bool (* HMI_UART_WRITE)(void* pTransmitData, size_t txSize);
    typedef bool (* HMI_UART_READ_IS_BUSY)(void);
    typedef bool (* HMI_UART_WRITE_IS_BUSY)(void);
    typedef bool (* HMI_UART_READ_ABORT)(void);
    typedef void (* HMI_UART_WRITE_CALLBACK_REGISTER)(HMI_UART_CALLBACK callback, uintptr_t context);

    typedef struct {
        HMI_UART_READ read_t;
        HMI_UART_READ_IS_BUSY readIsBusy;
        HMI_UART_READ_ABORT readAbort;
        HMI_UART_WRITE write_t;
        HMI_UART_WRITE_IS_BUSY writeIsBusy;
        HMI_UART_WRITE_CALLBACK_REGISTER writeCallbackRegister;
    } HMI_UART_INTERFACE;

    typedef struct {
        uint16_t startAddress;
        uint8_t dataSize;
    } HMI_TAG_ENTRY;

    typedef struct {

        struct {
            bool isPending;
            char buffer[24];
        } entry[20];
//entry[MAX_HMI_PARA];
        int8_t numPending;
        bool lock;
    } HMI_PENDING;

    typedef struct {
        HMI_TAG_TYPE items[HMI_QUEUE_SIZE];
        int front;
        int rear;
        int size;
    } HMI_TAG_QUEUE;

    void HMIDwin_Initialize();
    void HMIDwin_Tasks();
    bool HMIDwin_TriggerSend(HMI_TAG_TYPE tagType);
    bool HMIDwin_TriggerSendStatus(uint8_t idxRow, uint8_t status);
//  bool HMIDwin_TriggerSendStatus(uint8_t idxRow, STATUS status);
    bool HMIDwin_TriggerSendValue(uint8_t idxRow, HMI_TAG_DATA_TYPE dataType, float data);

#ifdef	__cplusplus
}
#endif

#endif	/* HMI_DWIN_H */