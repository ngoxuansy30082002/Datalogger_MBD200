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

    typedef struct {
        SIM_UART_WRITE write;
        SIM_UART_WRITE_PENDING_BYTE writePendingBytes;
        SIM_UART_WRITE_FREE_BYTE writeFreeBytes;
        SIM_UART_TRANSMIT_COMPLETE transmitComplete;
        SIM_UART_READ read;
        SIM_UART_READ_PENDING_BYTE readPendingBytes;
        SIM_UART_READ_FREE_BYTE readFreeBytes;

        GPIO_PIN resetPin;
        GPIO_PIN pwrPin;
        GPIO_PIN statusPin;
        GPIO_PIN netStatusPin;
    } SIM_UART_PLIB;



#ifdef	__cplusplus
}
#endif

#endif	/* SIM_DRIVER_H */

