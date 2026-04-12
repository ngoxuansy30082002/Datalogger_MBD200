/* 
 * File:   drv_fm25cl.h
 * Author: LENOVO
 *
 * Created on December 29, 2025, 2:25 PM
 */

#ifndef DRV_FM25CL_H
#define	DRV_FM25CL_H

#include <stdio.h>
#include <string.h>
#include "definitions.h"

#define DRV_FM25CL_CHIP_SELECT_PIN      GPIO_PIN_RD12
#define DRV_FM25CL_RESET_PIN            GPIO_PIN_RC14

#define DRV_FM25CL_HEADER_LEN           3 // 1byte opcode + 2byte address

#define DRV_FM25CL_OPCODE_WREN          0x06  
#define DRV_FM25CL_OPCODE_WRDI          0x04
#define DRV_FM25CL_OPCODE_RDSR          0x05
#define DRV_FM25CL_OPCODE_WRSR          0x01
#define DRV_FM25CL_OPCODE_READ          0x03
#define DRV_FM25CL_OPCODE_WRITE         0x02

#ifdef	__cplusplus
extern "C" {
#endif

    typedef enum {
        /* Transfer is being processed */
        DRV_FM25CL_TRANSFER_BUSY,
        /* Transfer is successfully completed*/
        DRV_FM25CL_TRANSFER_COMPLETED,
        /* Transfer had error*/
        DRV_FM25CL_TRANSFER_ERROR_UNKNOWN,
    } DRV_FM25CL_TRANSFER_STATUS;

    typedef void (* DRV_FM25CL_PLIB_CALLBACK)(uintptr_t context);
    typedef bool (* DRV_FM25CL_PLIB_WRITE_READ)(void* pTransmitData, size_t txSize, void *pReceiveData, size_t rxSize);
    typedef bool (* DRV_FM25CL_PLIB_WRITE)(void* pTransmitData, size_t txSize);
    typedef bool (* DRV_FM25CL_PLIB_READ)(void* pReceiveData, size_t rxSize);
    typedef bool (* DRV_FM25CL_PLIB_IS_BUSY)(void);
    typedef bool (*DRV_FM25CL_PLIB_IS_TX_BUSY) (void);
    typedef void (* DRV_FM25CL_PLIB_CALLBACK_REGISTER)(DRV_FM25CL_PLIB_CALLBACK callback, uintptr_t context);
    typedef void (*DRV_FM25CL_EVENT_HANDLER) (DRV_FM25CL_TRANSFER_STATUS event, uintptr_t context);

    typedef struct {
        DRV_FM25CL_PLIB_WRITE_READ writeRead;
        DRV_FM25CL_PLIB_WRITE write_t;
        DRV_FM25CL_PLIB_READ read_t;
        DRV_FM25CL_PLIB_IS_BUSY isBusy;
        DRV_FM25CL_PLIB_IS_TX_BUSY isTransmitterBusy;
        DRV_FM25CL_PLIB_CALLBACK_REGISTER callbackRegister;
        GPIO_PIN chipSelectPin;
        GPIO_PIN resetPin;
    } DRV_FM25CL_PLIB_INTERFACE;

    typedef enum {
        DRV_FM25CL_OPEN_READ_METADATA = 0,
        DRV_FM25CL_OPEN_WRITE_METADATA,
        DRV_FM25CL_OPEN_VERIFY_METADATA,
        DRV_FM25CL_OPEN_DONE,
        DRV_FM25CL_OPEN_WAIT_FOR_TRANSFER
    } DRV_FM25CL_OPEN_STATE;

    typedef enum {
        DRV_FM25CL_STATE_READ = 0,
        DRV_FM25CL_STATE_WRITE_ENABLE,
        DRV_FM25CL_STATE_WRITE,
    } DRV_FM25CL_STATE;

    typedef struct {
        /* Pointer to the receive data */
        void* pReceiveData;

        void* pOutData;
        /* Pointer to the transmit data */
        void* pTransmitData;

        /* Number of bytes to be written */
        size_t txSize;

        /* Number of bytes to be read */
        size_t rxSize;

    } DRV_FM25CL_TRANSFER_OBJ;

    /**************************************
     * AMC131 Driver Hardware Instance Object
     **************************************/
    typedef struct {
        /* Flag to indicate state  */
        DRV_FM25CL_STATE state;

        DRV_FM25CL_OPEN_STATE openState;

        /* Flag to indicate status of transfer */
        volatile DRV_FM25CL_TRANSFER_STATUS transferStatus;

        SYS_STATUS status;

        uint16_t registerAddr;

        /* Application event handler */
        DRV_FM25CL_EVENT_HANDLER eventHandler;

        /* Application context */
        uintptr_t context;

        bool isOpcodeCommand;

        DRV_FM25CL_TRANSFER_OBJ transferDataObj;

        DRV_FM25CL_TRANSFER_OBJ transferCmdObj;

    } DRV_FM25CL_OBJECT;

    //****************************************************************************
    //
    // Function prototypes
    //
    //****************************************************************************

    bool DrvFM25CL_Initialize();
    SYS_STATUS DrvFM25CL_Open();
    bool DrvFM25CL_readMultiRegister(uint16_t address, uint8_t *outdata, uint16_t readLen);
    bool DrvFM25CL_writeMultiRegister(uint16_t address, const uint8_t *data, uint16_t writeLen);
    DRV_FM25CL_TRANSFER_STATUS DrvFM25CL_GetTransferStatus();

#ifdef	__cplusplus
}
#endif

#endif	/* DRV_FM25CL_H */

