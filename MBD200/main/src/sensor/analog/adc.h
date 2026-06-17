/* 
 * File:   adc.h
 * Author: Syxn
 *
 * Created on May 7, 2024, 3:39 PM
 */

#ifndef ADC_H
#define	ADC_H

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "definitions.h"
#include "ad717x.h"

#define ADC_TIMEOUT                         4
#define ADC_WINDOW_SIZE                     3

#define NUM_ADC_DEVICE                      1
#define NUM_CHANNEL_PER_DEVICE              4

#ifdef	__cplusplus
extern "C" {
#endif

    typedef enum {
        ADC_IDLE = 0,
        ADC_REINIT,
        ADC_ERROR,
        ADC_READING_DATA,
        ADC_READ_DATA_COMPLETE
    } ADC_STATES;

    typedef struct {
        float buffer[ADC_WINDOW_SIZE];
        int index;
    } ADC_CHANNEL_BUFFER;

    typedef struct {
        ad717x_dev *devDcpt;
        bool reInit;
        ADC_STATES states;
        uint32_t pollTick;
        uint32_t reInitTick;
        uint32_t timeoutTick;
        uint8_t duplicateChannelCount;
        uint8_t curChannel;
        uint8_t preChannel;
        int32_t sampleData;
    }
    ADC_DEVICE;

    typedef struct {

        struct {
            double rawValue;
            double value;
            SENSOR_STATUS status;
        } entry[MAX_ANALOG_CHANNEL];
    } ADC_DATA;

    void Adc_Initialize(void);
    void Adc_Task(void);
    void Adc_TriggerReinit(void);

    extern ADC_DATA adcDt;

#ifdef	__cplusplus
}
#endif

#endif	/* ADC_H */

