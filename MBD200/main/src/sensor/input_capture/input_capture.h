/* 
 * File:   input_capture.h
 * Author: LENOVO
 *
 * Created on June 1, 2026, 7:45 PM
 */

#ifndef INPUT_CAPTURE_H
#define	INPUT_CAPTURE_H

#include <stdio.h>
#include <string.h>
#include "definitions.h"

#define INPUT_CAPTURE_HW_CHANNEL       2
#define INPUT_CAPTURE_FREQ_SAMPLES  3

#ifdef	__cplusplus
extern "C" {
#endif

    typedef void (* DRV_IC_CALLBACK)(uintptr_t context);
    typedef void (* DRV_IC_DISABLE)(void);
    typedef void (* DRV_IC_ENABLE)(void);
    typedef uint16_t(* DRV_IC_BUFFER_READ)(void);
    typedef void (* DRV_IC_CALLBACK_REGISTER)(DRV_IC_CALLBACK callback, uintptr_t context);
    typedef void (* DRV_IC_TMR_START)(void);
    typedef void (* DRV_IC_TMR_STOP)(void);
    typedef uint16_t(* DRV_IC_TMR_PERIOD_GET)(void);
    typedef uint32_t(* DRV_IC_TMR_FREQ_GET)(void);

    typedef struct {

        struct {
            DRV_IC_ENABLE enable;
            DRV_IC_DISABLE disable;
            DRV_IC_BUFFER_READ bufferRead;
            DRV_IC_CALLBACK_REGISTER callbackRegister;
        } channel[INPUT_CAPTURE_HW_CHANNEL];

        DRV_IC_TMR_START tmrStart;
        DRV_IC_TMR_STOP tmrStop;
        DRV_IC_TMR_PERIOD_GET periodGet;
        DRV_IC_TMR_FREQ_GET freqGet;
    } DRV_IC_PLIB;

    typedef enum {
        IC_INIT = 0,
        IC_RESTORE,
        IC_RUNNING,
        IC_ERROR
    } IC_STATE;

    typedef struct {
        uint16_t lastCapture;
        uint64_t pulseCount;
        uint32_t deltaBuffer[INPUT_CAPTURE_FREQ_SAMPLES];
        uint8_t bufIdx;
        uint8_t sampleCount;
        bool newDataReady;

        uint32_t lastUpdateTick;
    } IC_HW_STATE;

    typedef struct {
        bool reInit;

        struct {
            bool pinState;
            uint64_t counter;
            SENSOR_STATUS status;
            float freq;
            double value;
        } entry[MAX_INPUT_CAPTURE];
    } INPUT_CAPTURE_DATA;

    void InputCapture_Initialize(void);
    void InputCapture_Task(void);

    extern INPUT_CAPTURE_DATA inputCaptureDt;

#ifdef	__cplusplus
}
#endif

#endif	/* INPUT_CAPTURE_H */

