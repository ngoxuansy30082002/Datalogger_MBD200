#include "input_capture.h"

INPUT_CAPTURE_DATA inputCaptureDt;

static const char * __TAG__ = "INPUTCAPTURE";

static const DRV_IC_PLIB _icPlib = {
    .channel =
    {
        {
            .enable = (DRV_IC_ENABLE) ICAP4_Enable,
            .disable = (DRV_IC_DISABLE) ICAP4_Disable,
            .bufferRead = (DRV_IC_BUFFER_READ) ICAP4_CaptureBufferRead,
            .callbackRegister = (DRV_IC_CALLBACK_REGISTER) ICAP4_CallbackRegister
        },
        {
            .enable = (DRV_IC_ENABLE) ICAP6_Enable,
            .disable = (DRV_IC_DISABLE) ICAP6_Disable,
            .bufferRead = (DRV_IC_BUFFER_READ) ICAP6_CaptureBufferRead,
            .callbackRegister = (DRV_IC_CALLBACK_REGISTER) ICAP6_CallbackRegister
        }
    },

    .tmrStart = (DRV_IC_TMR_START) TMR3_Start,
    .tmrStop = (DRV_IC_TMR_STOP) TMR3_Stop,
    .periodGet = (DRV_IC_TMR_PERIOD_GET) TMR3_PeriodGet,
    .freqGet = (DRV_IC_TMR_FREQ_GET) TMR3_FrequencyGet
};

static IC_HW_STATE _hwState[INPUT_CAPTURE_HW_CHANNEL];
static IC_STATE _states = IC_INIT;
static uint32_t _timerFreq = 0;
static uint16_t _timerPeriod = 0xFFFF;

static void _captureCallbackHandler(uintptr_t context) {
    uint8_t hwIdx = (uint8_t) context; // 0: ICAP4, 1: ICAP6

    uint16_t currentCapture = _icPlib.channel[hwIdx].bufferRead();
    uint32_t delta = 0;

    if (currentCapture >= _hwState[hwIdx].lastCapture)
        delta = currentCapture - _hwState[hwIdx].lastCapture;
    else
        delta = (_timerPeriod - _hwState[hwIdx].lastCapture) + currentCapture;

    if (delta > 0) {
        _hwState[hwIdx].deltaBuffer[_hwState[hwIdx].bufIdx] = delta;
        _hwState[hwIdx].bufIdx = (_hwState[hwIdx].bufIdx + 1) % INPUT_CAPTURE_FREQ_SAMPLES;
        if (_hwState[hwIdx].sampleCount < INPUT_CAPTURE_FREQ_SAMPLES)
            _hwState[hwIdx].sampleCount++;
    }

    _hwState[hwIdx].pulseCount++;
    _hwState[hwIdx].lastCapture = currentCapture;
    _hwState[hwIdx].newDataReady = true;
}

void InputCapture_UpdateConfig() {
    bool useIC4 = gInCaptureCfg.entry[0].enable || gInCaptureCfg.entry[1].enable;
    bool useIC6 = gInCaptureCfg.entry[2].enable || gInCaptureCfg.entry[3].enable;

    if (useIC4) {
        if (_icPlib.channel[0].enable) _icPlib.channel[0].enable();
    } else {
        if (_icPlib.channel[0].disable) _icPlib.channel[0].disable();
    }

    if (useIC6) {
        if (_icPlib.channel[1].enable) _icPlib.channel[1].enable();
    } else {
        if (_icPlib.channel[1].disable) _icPlib.channel[1].disable();
    }

    if (useIC4 || useIC6) {
        if (_icPlib.tmrStart) _icPlib.tmrStart();
    } else {
        if (_icPlib.tmrStop) _icPlib.tmrStop();
    }
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

static double _calculatorAdvancedScaling(double value, uint8_t thisIc) {
    const INPUT_CAPTURE_CHANNEL_CONFIG * thisIcCfg = &gInCaptureCfg.entry[thisIc];
    double outValue = value;

    /* Scale */
    if (thisIcCfg->scaleType == SCALE_LINEAR) {
        switch (thisIcCfg->scaleDataType) {
            case DATA_UINT:
                outValue = (uint64_t) (value * thisIcCfg->scaleValue);
                break;

            case DATA_INT:
                outValue = (int64_t) (value * thisIcCfg->scaleValue);
                break;

            case DATA_FLOAT:
                outValue = (value * thisIcCfg->scaleValue);
                break;

            default: break;
        }
    } else if (thisIcCfg->scaleType == SCALE_SQRT) {
        switch (thisIcCfg->scaleDataType) {
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
    if (thisIcCfg->scaleType != SCALE_NONE) {
        outValue = _applyOperator(thisIcCfg->offsetPreVal, outValue, thisIcCfg->offSetPreOperator);
        outValue = _applyOperator(outValue, thisIcCfg->offsetSubVal, thisIcCfg->offsetSubOperator);
    }

    return outValue;
}

void InputCapture_Initialize(void) {
    memset(_hwState, 0, sizeof (_hwState));
    memset(&inputCaptureDt, 0, sizeof (INPUT_CAPTURE_DATA));

    if (_icPlib.freqGet != NULL) _timerFreq = _icPlib.freqGet();
    if (_icPlib.periodGet != NULL) _timerPeriod = _icPlib.periodGet();

    if (_icPlib.channel[0].callbackRegister != NULL) {
        _icPlib.channel[0].callbackRegister(_captureCallbackHandler, 0);
    }
    if (_icPlib.channel[1].callbackRegister != NULL) {
        _icPlib.channel[1].callbackRegister(_captureCallbackHandler, 1);
    }

    _states = IC_INIT;
}

void InputCapture_Task(void) {

#define NEXT_STATE(nextState)  \
    do { _states = (nextState); } while(0)

    switch (_states) {
        case IC_INIT:
        {
            InputCapture_UpdateConfig();
            NEXT_STATE(IC_RESTORE);
            break;
        }

        case IC_RESTORE:
        {
            // channelData[1].counter = Read_FRAM(ADDR_COUNTER_CH1);
            NEXT_STATE(IC_RUNNING);
            break;
        }

        case IC_RUNNING:
        {
            for (uint8_t i = 0; i < MAX_INPUT_CAPTURE; i++) {
                const INPUT_CAPTURE_CHANNEL_CONFIG * thisIcCfg = &gInCaptureCfg.entry[i];

                if (!thisIcCfg->enable) {
                    inputCaptureDt.entry[i].status = STATUS_DISABLE;
                    inputCaptureDt.entry[i].freq = 0.0f;
                    continue;
                } else {
                    if (inputCaptureDt.entry[i].status == STATUS_DISABLE)
                        inputCaptureDt.entry[i].status = STATUS_IDENTIFYING;
                }

                uint8_t hwIdx = (i == 0 || i == 1) ? 0 : 1; // 0,1 use IC4; 2,3 use IC6

                if (_hwState[hwIdx].newDataReady) {
                    LOG_DEBUG("%s - %s: IC new data", __TAG__, __func__);
                    if (i == 1 || i == 3) {
                        inputCaptureDt.entry[i].counter = _hwState[hwIdx].pulseCount;
                        float rawVal = (float) inputCaptureDt.entry[i].counter * thisIcCfg->valPerPulse;
                        inputCaptureDt.entry[i].value = _calculatorAdvancedScaling(rawVal, i);
                        inputCaptureDt.entry[i].status = STATUS_GOOD;
                    } else if (i == 0 || i == 2) {
                        if (_hwState[hwIdx].sampleCount > 0) {
                            uint32_t sumDelta = 0;
                            for (uint8_t k = 0; k < _hwState[hwIdx].sampleCount; k++) {
                                sumDelta += _hwState[hwIdx].deltaBuffer[k];
                            }

                            float avgDelta = (float) sumDelta / _hwState[hwIdx].sampleCount;
                            if (avgDelta > 0) {
                                inputCaptureDt.entry[i].freq = (float) _timerFreq / avgDelta;
                            }

                            float rawVal = inputCaptureDt.entry[i].freq * thisIcCfg->valPerPulse;
                            inputCaptureDt.entry[i].value = _calculatorAdvancedScaling(rawVal, i);

                            if (inputCaptureDt.entry[i].freq >= thisIcCfg->minFreq)
                                inputCaptureDt.entry[i].status = STATUS_GOOD;
                            else
                                inputCaptureDt.entry[i].status = STATUS_BAD;
                        } else
                            inputCaptureDt.entry[i].status = STATUS_IDENTIFYING;

                        _hwState[hwIdx].lastUpdateTick = TICK_NOW();
                    }
                } else {
                    if (i == 0 || i == 2) {
                        uint32_t timeoutThresholdMs = (thisIcCfg->minFreq > 0.0f) ? (uint32_t) (1000.0f / thisIcCfg->minFreq) : 1000;
                        timeoutThresholdMs += 1000;

                        if (TIME_IS_EXPIRED(_hwState[hwIdx].lastUpdateTick, timeoutThresholdMs)) {
                            inputCaptureDt.entry[i].freq = 0.0f;
                            inputCaptureDt.entry[i].value = 0.0f;
                            inputCaptureDt.entry[i].status = STATUS_BAD;

                            _hwState[hwIdx].sampleCount = 0;
                            _hwState[hwIdx].bufIdx = 0;
                        }
                    }
                }
            }

            _hwState[0].newDataReady = false;
            _hwState[1].newDataReady = false;

            if (inputCaptureDt.reInit) {
                inputCaptureDt.reInit = false;
                InputCapture_UpdateConfig();
            }

            break;
        }

        case IC_ERROR:
            break;
    }
}