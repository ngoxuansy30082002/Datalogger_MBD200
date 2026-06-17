/* 
 * File:   digital_output.h
 * Author: LENOVO
 *
 * Created on June 3, 2026, 9:09 PM
 */

#ifndef DIGITAL_OUTPUT_H
#define	DIGITAL_OUTPUT_H

#include <stdio.h>
#include <string.h>
#include "definitions.h"

#ifdef	__cplusplus
extern "C" {
#endif

    typedef struct {
        GPIO_PIN relay1;
        GPIO_PIN relay2;
    } DRV_DO_PLIB;

    typedef enum {
        DO_STATE_IDLE = 0,
        DO_STATE_ON,
        DO_STATE_OFF
    } DO_STATE;

    typedef struct {
        uint32_t pin;
        DO_STATE state;
        uint16_t currentCount;
        uint32_t lastTick;
    } DO_CONTEXT;

    typedef struct {

        struct {
            bool state;
            bool level;
        } out[MAX_DIGITAL_OUTPUT];
    } DO_DATA;

    void DigitalOutput_Initialize(void);
    void DigitalOutput_Task(void);
    void DigitalOutput_Set(uint8_t id, bool enable);

    extern DO_DATA doDt;
#ifdef	__cplusplus
}
#endif

#endif	/* DIGITAL_OUTPUT_H */

