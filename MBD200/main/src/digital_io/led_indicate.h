/* 
 * File:   led_indicate.h
 * Author: LENOVO
 *
 * Created on June 3, 2026, 8:11 PM
 */

#ifndef LED_INDICATE_H
#define	LED_INDICATE_H

#include <stdio.h>
#include <string.h>
#include "definitions.h"

#define LED_BLINK_ON_TIME_MS    100   
#define LED_BLINK_OFF_TIME_MS   100   
#define LED_PAUSE_TIME_MS       1500  

#ifdef	__cplusplus
extern "C" {
#endif

    typedef struct {
        GPIO_PIN power;
        GPIO_PIN status;
        GPIO_PIN rtu;
        GPIO_PIN gsm;
    } DRV_LED_PLIB;

    typedef enum {
        LED_ID_POWER = 0,
        LED_ID_STATUS,
        LED_ID_RTU,
        LED_ID_GSM,
        LED_NUM_MAX
    } LED_ID;

    typedef enum {
        LED_MODE_OFF = 0,
        LED_MODE_ON,
        LED_MODE_BLINK_INF,
        LED_MODE_BLINK_N_REPEAT,
        LED_MODE_BLINK_N_STOP
    } LED_MODE;

    typedef enum {
        LED_STATE_IDLE = 0,
        LED_STATE_ON,
        LED_STATE_OFF,
        LED_STATE_PAUSE
    } LED_STATE;

    typedef struct {
        uint32_t pin;
        LED_MODE mode;
        LED_STATE state;

        uint16_t targetCount;
        uint16_t currentCount;
        uint32_t lastTick;
    } LED_CONTEXT;

    void LedIndicate_Initialize(void);
    void LedIndicate_Task(void);
    void LedIndicate_SetMode(LED_ID id, LED_MODE mode, uint16_t count);
    

#ifdef	__cplusplus
}
#endif

#endif	/* LED_INDICATE_H */

