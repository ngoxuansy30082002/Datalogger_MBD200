/* 
 * File:   sim_driver.h
 * Author: LENOVO
 *
 * Created on March 31, 2026, 8:18 PM
 */

#ifndef SIM_DRIVER_H
#define	SIM_DRIVER_H

#include <stdio.h>
#include <string.h>
#include "definitions.h"

#ifdef	__cplusplus
extern "C" {
#endif

    typedef size_t(* SIM_UART_WRITE)(uint8_t* pWrBuffer, const size_t size);
    typedef size_t(* SIM_UART_WRITE_PENDING_BYTE)(void);
    typedef size_t(* SIM_UART_WRITE_FREE_BYTE)(void);
    typedef bool (* SIM_UART_TRANSMIT_COMPLETE)(void);
    typedef size_t(* SIM_UART_READ)(uint8_t* pRdBuffer, const size_t size);
    typedef size_t(* SIM_UART_READ_PENDING_BYTE)(void);
    typedef size_t(* SIM_UART_READ_FREE_BYTE)(void);
    typedef size_t(* SIM_UART_READ_THRESHOLD_SET)(uint32_t nBytesThreshold);
    typedef size_t(* SIM_UART_READ_CALLBACK_REGISTER)(UART_RING_BUFFER_CALLBACK callback, uintptr_t context);
    typedef size_t(* SIM_UART_READ_NOTIFY_ENABLE)(bool isEnabled, bool isPersistent);

    typedef struct {
        SIM_UART_WRITE write;
        SIM_UART_WRITE_PENDING_BYTE writePendingBytes;
        SIM_UART_WRITE_FREE_BYTE writeFreeBytes;
        SIM_UART_TRANSMIT_COMPLETE transmitComplete;
        SIM_UART_READ read;
        SIM_UART_READ_PENDING_BYTE readPendingBytes;
        SIM_UART_READ_FREE_BYTE readFreeBytes;
        SIM_UART_READ_THRESHOLD_SET readThresholdSet;
        SIM_UART_READ_CALLBACK_REGISTER readCallbackRegister;
        SIM_UART_READ_NOTIFY_ENABLE readNotifyEnable;

        GPIO_PIN resetPin;
        GPIO_PIN pwrPin;
        GPIO_PIN statusPin;
        GPIO_PIN netStatusPin;
    } SIM_UART_PLIB;

    typedef enum {
        SIM_DRV_IDLE = 0,
        SIM_DRV_TX_BUSY,
        SIM_DRV_RX_BUSY
    } SIM_DRV_STATE;

    typedef enum {
        SIM_DRV_STATUS_IDLE,
        SIM_DRV_STATUS_WAIT_RESP, // Data is being sent or waiting for response
        SIM_DRV_STATUS_RECV_RESP, // Response received, data is in buffer
        SIM_DRV_STATUS_TIMEOUT // Time is up, no response
    } SIM_DRV_STATUS;

#ifdef	__cplusplus
}
#endif

#endif	/* SIM_DRIVER_H */

