#include "adc.h"
#include "ad411x_regs.h"

ADC_DATA adcDt;

float _calculator_analog_float(float scale_float, uint8_t index);
static void _ADC_PrintHMI(uint8_t adcIdxCh, bool noGood);


static const char * __TAG__ = "ADC";
static ADC_DEVICE _devices[NUM_ADC_DEVICE] = {0};
static ADC_CHANNEL_BUFFER _chnBuffer[MAX_ANALOG_CHANNEL] = {0};

static ad717x_init_param ad717xInit = {
    .regs = ad4111_regs,
    .num_regs = sizeof (ad4111_regs) / sizeof (ad4111_regs[0]),
    .active_device = ID_AD4111,
    .mode = CONTINUOUS,
    .num_channels = 4,
    .chan_map[0].analog_inputs.analog_input_pairs = IIN0P_IIN0M,
    .chan_map[0].setup_sel = 0,
    .chan_map[1].analog_inputs.analog_input_pairs = IIN1P_IIN1M,
    .chan_map[1].setup_sel = 0,
    .chan_map[2].analog_inputs.analog_input_pairs = IIN2P_IIN2M,
    .chan_map[2].setup_sel = 0,
    .chan_map[3].analog_inputs.analog_input_pairs = IIN3P_IIN3M,
    .chan_map[3].setup_sel = 0,
    .num_setups = 1,
    .setups[0].bi_unipolar = false,
    .setups[0].input_buff = false,
    .setups[0].ref_buff = false,
    .setups[0].ref_source = INTERNAL_REF,
};

static uint8_t _numChannelEnable(uint8_t idxAdcDevice) {
    uint8_t startAdcChannel = idxAdcDevice * NUM_CHANNEL_PER_DEVICE;
    uint8_t endAdcChannel = startAdcChannel + NUM_CHANNEL_PER_DEVICE;

    uint8_t numChannelEnable = 0;
    for (uint8_t i = startAdcChannel; i < endAdcChannel; i++) {
        if (gAnalogCfg.entry[i].enable == true)
            numChannelEnable++;
        else {
            adcDt.entry[i].status = STATUS_DISABLE;
            adcDt.entry[i].rawValue = 0;
            adcDt.entry[i].value = 0;
        }
    }

    return numChannelEnable;
}

static bool _adcInit(uint8_t idxAdcDevice) {
    uint8_t startAdcChannel = idxAdcDevice * NUM_CHANNEL_PER_DEVICE;
    uint8_t endAdcChannel = startAdcChannel + NUM_CHANNEL_PER_DEVICE;
    ADC_DEVICE * dev = &_devices[idxAdcDevice];

    AD717X_remove(dev->devDcpt);

    for (uint8_t i = startAdcChannel, j = 0; i < endAdcChannel, j < NUM_CHANNEL_PER_DEVICE; i++, j++)
        ad717xInit.chan_map[j].channel_enable = gAnalogCfg.entry[i].enable;

    ad717xInit.idxAdcDevice = idxAdcDevice;
    int32_t res = AD717X_Init(&dev->devDcpt, ad717xInit);
    if (res >= 0) {
        LOG_SUCCESS("%s - %s:\t Device %u INIT SUCCESS with ID: 0x%X \r\n", __TAG__, __func__, idxAdcDevice, res);
        return true;
    } else {
        LOG_ERROR("%s - %s:\t Device %u INIT FAIL \r\n", __TAG__, __func__, idxAdcDevice);
        return false;
    }
}

static double _updateChannelValue(int channel, float newValue) {
    ADC_CHANNEL_BUFFER *buf = &_chnBuffer[channel];
    buf->buffer[buf->index] = newValue;
    buf->index = (buf->index + 1) % ADC_WINDOW_SIZE;

    float a = buf->buffer[0];
    float b = buf->buffer[1];
    float c = buf->buffer[2];
    double selectedValue;

    if (fabs(a - b) < fabs(a - c) && fabs(a - b) < fabs(b - c)) {
        selectedValue = (a + b) / 2;
    } else if (fabs(a - c) < fabs(a - b) && fabs(a - c) < fabs(b - c)) {
        selectedValue = (a + c) / 2;
    } else {
        selectedValue = (b + c) / 2;
    }

    return selectedValue;
}

static double _applyOperator(double val1, double val2, OPERATOR op) {
    switch (op) {
        case OPERATOR_ADDITION: return val1 + val2;
        case OPERATOR_SUBTRACTION: return val1 - val2;
        case OPERATOR_MULTIPLICATION: return val1 * val2;
        case OPERATOR_DIVISION: return (val2 != 0) ? (val1 / val2): 0;
        default: return val1;
    }
}

static double _calculatorAdvancedScaling(double value, uint8_t thisAdc) {
    const ANALOG_CHANNEL_CONFIG * thisCfg = &gAnalogCfg.entry[thisAdc];
    double outValue = value;

    /* Scale */
    if (thisCfg->scaleType == SCALE_LINEAR) {
        switch (thisCfg->scaleDataType) {
            case DATA_UINT:
                outValue = (uint64_t) (value * thisCfg->scaleValue);
                break;

            case DATA_INT:
                outValue = (int64_t) (value * thisCfg->scaleValue);
                break;

            case DATA_FLOAT:
                outValue = (value * thisCfg->scaleValue);
                break;

            default: break;
        }
    } else if (thisCfg->scaleType == SCALE_SQRT) {
        switch (thisCfg->scaleDataType) {
            case DATA_UINT:
                outValue = (uint64_t) sqrt(value);
                break;

            case DATA_INT:
                outValue = (int64_t) sqrt(value);
                break;

            case DATA_FLOAT:
                outValue = sqrt(value);
                break;

            default: break;
        }
    }

    /* Offset */
    if (thisCfg->scaleType != SCALE_NONE) {
        outValue = _applyOperator(thisCfg->offsetPreVal, outValue, thisCfg->offSetPreOperator);
        outValue = _applyOperator(outValue, thisCfg->offsetSubVal, thisCfg->offsetSubOperator);
    }

    return outValue;
}

void Adc_Initialize(void) {
    DrvSpiAdc_Initialize();

    for (uint8_t i = 0; i < NUM_ADC_DEVICE; i++) {
        ADC_DEVICE * dev = &_devices[i];
        dev->reInit = true;
        dev->states = ADC_IDLE;
    }
}

void Adc_Task(void) {
    static uint8_t idxAdcDevice = 0;
    static uint8_t idxChannelActually[NUM_ADC_DEVICE] = {0};

    ADC_DEVICE * dev = &_devices[idxAdcDevice];

#define NEXT_STATE(nextState)  \
    do { dev->states = (nextState); } while(0)

    switch (dev->states) {

        case ADC_IDLE:
        {
            if (TIME_IS_EXPIRED(dev->pollTick, 200)) {
                dev->pollTick = TICK_NOW();
                if (dev->reInit)
                    NEXT_STATE(ADC_REINIT);
                else if (_numChannelEnable(idxAdcDevice) > 0) {
                    NEXT_STATE(ADC_READING_DATA);
                    dev->timeoutTick = TICK_NOW();
                }
            }
            break;
        }

        case ADC_REINIT:
        {
            if (TIME_IS_EXPIRED(dev->reInitTick, 2000)) {
                dev->reInitTick = TICK_NOW();
                if (_adcInit(idxAdcDevice)) {
                    dev->reInit = false;
                    NEXT_STATE(ADC_IDLE);
                } else
                    NEXT_STATE(ADC_ERROR);
            }
            break;
        }

        case ADC_READING_DATA:
        {
            if (TIME_IS_EXPIRED(dev->timeoutTick, 500)) {
                NEXT_STATE(ADC_ERROR);
                LOG_ERROR("%s - %s:\t Device %u Read RDY bit TIMEOUT\r\n", idxAdcDevice);
                break;
            }

            if (AD717X_WaitForReady(dev->devDcpt, ADC_TIMEOUT, &dev->curChannel) >= 0) {
                idxChannelActually[idxAdcDevice] = dev->curChannel + (idxAdcDevice * NUM_CHANNEL_PER_DEVICE);
                uint8_t thisChannel = idxChannelActually[idxAdcDevice];

                if (AD717X_ReadData(dev->devDcpt, &dev->sampleData) < 0) {
                    NEXT_STATE(ADC_ERROR);
                    LOG_ERROR("%s - %s:\t Device %u Read data ERROR \r\n", __TAG__, __func__, idxAdcDevice);
                } else {
                    if (dev->curChannel >= NUM_CHANNEL_PER_DEVICE || !gAnalogCfg.entry[thisChannel].enable) {
                        NEXT_STATE(ADC_ERROR);
                        LOG_ERROR("%s - %s:\t Device %u Channel %u invalid \r\n", __TAG__, __func__, idxAdcDevice, dev->curChannel);
                    } else
                        NEXT_STATE(ADC_READ_DATA_COMPLETE);
                }
            }
            break;
        }

        case ADC_READ_DATA_COMPLETE:
        {
            if (_numChannelEnable(idxAdcDevice) >= 2) {
                if (dev->curChannel == dev->preChannel)
                    dev->duplicateChannelCount++;
                else {
                    dev->duplicateChannelCount = 0;
                    dev->preChannel = dev->curChannel;
                }
            }

            /* Check if multi duplicate -> error */
            if (dev->duplicateChannelCount > 10) {
                dev->duplicateChannelCount = 0;
                NEXT_STATE(ADC_ERROR);
                LOG_ERROR("%s - %s:\t Device %u Channel %u multi duplicate \r\n", __TAG__, __func__, idxAdcDevice, dev->curChannel);
            } else {
                uint8_t thisAdc = idxChannelActually[idxAdcDevice];
                const ANALOG_CHANNEL_CONFIG * thisCfg = &gAnalogCfg.entry[thisAdc];

                double Iin = (double) (dev->sampleData * 2.5);
                Iin = (double) (Iin / 50.0);
                Iin = (double) (Iin * 1000 / 16777216.0);
                Iin = _updateChannelValue(thisAdc, Iin);
                adcDt.entry[thisAdc].rawValue = Iin;

                if (Iin < thisCfg->inputLow || Iin > thisCfg->inputHigh)
                    adcDt.entry[thisAdc].status = STATUS_BAD;
                else
                    adcDt.entry[thisAdc].status = STATUS_GOOD;

                /* Convert raw input -> Output value */

                double value = Iin;
                if (thisCfg->inputHigh != thisCfg->inputLow)
                    value = (Iin - thisCfg->inputLow) * (thisCfg->outputHigh - thisCfg->outputLow) /
                    (thisCfg->inputHigh - thisCfg->inputLow) + thisCfg->outputLow;

                /* Scale */
                adcDt.entry[thisAdc].value = _calculatorAdvancedScaling(value, thisAdc);

//                LOG_INFO("%s - %s:\t Device %u Channel %u, raw = %.5f, value = %.5f\r\n", __TAG__, __func__, idxAdcDevice, dev->curChannel, Iin, adcDt.entry[thisAdc].value);
                NEXT_STATE(ADC_IDLE);
            }
            break;
        }

        case ADC_ERROR:
        {
            dev->reInit = true;
            NEXT_STATE(ADC_IDLE);
            //            _ADC_PrintHMI(thisAdc, (adcDt.ADCChStatus[thisAdc] != GOOD));
            break;
        }

        default:
            NEXT_STATE(ADC_IDLE);

    }
}

void Adc_TriggerReinit(void) {
    for (uint8_t i = 0; i < NUM_ADC_DEVICE; i++) {
        ADC_DEVICE * dev = &_devices[i];
        dev->reInit = true;
    }
}

static void _ADC_PrintHMI(uint8_t adcChActually, bool noGood) {
    //    for (uint8_t i = 0; i < MAX_HMI_PARA; i++) {
    //        uint8_t idxHMI = glbAppCfg.tag_hmi[i];
    //        if (idxHMI == 30 || glbAppCfg.sensor.entry[idxHMI].enable == false)
    //            continue;
    //
    //        if (glbAppCfg.sensor.entry[idxHMI].type == SENSOR_ANALOG &&
    //                glbAppCfg.sensor.entry[idxHMI].idxInType == adcChActually) {
    //            uint8_t status = 2;
    //
    //            int8_t stt = APP_CalculateStatusSensor(idxHMI);
    //
    //            if (stt == (STATUS) BAD) status = 2;
    //            else if (stt == (STATUS) CALIBRATION) status = 1;
    //            else if (stt == (STATUS) GOOD) status = 0;
    //            else if (stt == (-1)) {
    //                if (noGood) status = 2;
    //                else status = 0;
    //            }
    //
    //            if (glbAppCfg.sensor.entry[idxHMI].calibrated) status = 1;
    //
    //            HMIDwin_TriggerSendStatus(i, status);
    //            HMIDwin_TriggerSendValue(i, HMI_DATA_FLOAT, (float) adcDt.valueAnalog[adcChActually]);
    //            //            (void) status;
    //        }
    //    };
}