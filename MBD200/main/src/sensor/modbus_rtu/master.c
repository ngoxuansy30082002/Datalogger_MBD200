#include "master.h"
#include "modbus_rtu_app.h"

MBRTU_MASTER_DATA mbrtuMasterDt;

static const char * __TAG__ = "MODBUS MASTER";
static const MODBUS_UART_INTERFACE _uartPlib = {
    .read_t = (MODBUS_UART_READ) UART2_Read,
    .readIsBusy = (MODBUS_UART_READ_IS_BUSY) UART2_ReadIsBusy,
    .readAbort = (MODBUS_UART_READ_ABORT) UART2_ReadAbort,
    .write_t = (MODBUS_UART_WRITE) UART2_Write,
    .writeIsBusy = (MODBUS_UART_WRITE_IS_BUSY) UART2_WriteIsBusy,
    .writeCallbackRegister = (MODBUS_UART_WRITE_CALLBACK_REGISTER) UART2_WriteCallbackRegister,
    .redePin = GPIO_PIN_RD13
};
static uint8_t _rowIndex = 0;

static uint8_t _gotoNextRow(uint8_t currentRow) {
    uint8_t nextRow = (currentRow + 1) % gMbrtuCfg.numTag;
    return nextRow;
}

static double _scaleValue(double input, SENSOR_SCALE_TYPE scaleType, float scaleValue) {
    switch (scaleType) {
        case SCALE_NONE:
            return input;
        case SCALE_LINEAR:
            return input * scaleValue;
        case SCALE_SQRT:
            if (input < 0) return -sqrt(-input);
            return sqrt(input);
    }
    return input;
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

static void _calculatorAdvancedScaling(uint8_t i) {
    const MODBUSRTU_TAG_ENTRY * thisCfg = &gMbrtuCfg.entry[i];
    MBRTU_RAW_VALUE rawValue = mbrtuMasterDt.entry[i].rawValue;

    double calcVal = 0.0;

    switch (thisCfg->rawDataType) {
        case DATA_UINT:
            //            LOG_DEBUG("reuint: %u --------\r\n", rawValue.reuint);
            calcVal = (double) rawValue.reuint;
            break;

        case DATA_INT:
            //            LOG_DEBUG("reint: %d --------\r\n", rawValue.reint);
            calcVal = (double) rawValue.reint;
            break;

        case DATA_FLOAT:
            //            LOG_DEBUG("refloat: %f --------\r\n", (double) rawValue.refloat.f);
            calcVal = (double) rawValue.refloat.f;
            break;

        default:
            return;
    }

    /* Conversion*/
    if (thisCfg->conversion) {
        if (thisCfg->inputMax != thisCfg->inputMin)
            calcVal = (calcVal - thisCfg->inputMin) * (thisCfg->outputMax - thisCfg->outputMin) /
            (thisCfg->inputMax - thisCfg->inputMin) + thisCfg->outputMin;
    }

    if (thisCfg->scaleType != SCALE_NONE) {
        /* Scale */
        calcVal = _scaleValue(calcVal, thisCfg->scaleType, thisCfg->scaleValue);

        /* Offset */
        calcVal = _applyOperator(thisCfg->offsetPreVal, calcVal, thisCfg->offSetPreOperator);
        calcVal = _applyOperator(calcVal, thisCfg->offsetSubVal, thisCfg->offsetSubOperator);
    }

    SENSOR_DATA_TYPE targetDataType;
    if (thisCfg->scaleType == SCALE_NONE)
        targetDataType = thisCfg->rawDataType;
    else
        targetDataType = thisCfg->scaleDataType;

    mbrtuMasterDt.entry[i].dataType = targetDataType;

    switch (targetDataType) {
        case DATA_UINT:
            mbrtuMasterDt.entry[i].value.uintVal = (uint64_t) calcVal;
            //            LOG_DEBUG("u_val222: %llu \r\n", mbrtuMasterDt.entry[i].value.uintVal);
            break;

        case DATA_INT:
            mbrtuMasterDt.entry[i].value.intVal = (int64_t) calcVal;
            //            LOG_DEBUG("i_val222: %lld \r\n", mbrtuMasterDt.entry[i].value.intVal);
            break;

        case DATA_FLOAT:
            mbrtuMasterDt.entry[i].value.floatVal = (float) calcVal;
            //            LOG_DEBUG("f_val222: %.5f \r\n", mbrtuMasterDt.entry[i].value.floatVal);
            break;

        default:
            break;
    }
}

static void _printToHMI(uint8_t thisTag) {
    for (uint8_t i = 0; i < gAppCfg.hmi.numEntry; i++) {
        uint8_t idxHMI = gAppCfg.hmi.sensorIdx[i] - 1;
        SENSOR_ENTRY_CONFIG *sEntry = &gSensorCfg.entry[idxHMI];

        if (!sEntry->enable)
            continue;

        if (sEntry->type == SENSOR_MBRTU &&
                sEntry->indexOfType == thisTag) {
            uint8_t status = 0;

            int8_t stt = SensorGeneral_calculateSensorStatusInput(idxHMI);
            if (stt == 0) status = 0;
            else if (stt == 1) status = 1;
            else if (stt == 2) status = 2;
            else {
                if (sEntry->calibrate)
                    status = 1;
                else {
                    stt = SensorGeneral_calculateSensorStatusAuto(sEntry->type, thisTag);
                    if (stt == -1)
                        continue;

                    status = stt;
                }
            }

            HMIDwin_TriggerSendStatus(i, status);

            if (mbrtuMasterDt.entry[thisTag].dataType == DATA_UINT) {
                float val = (float) mbrtuMasterDt.entry[thisTag].value.uintVal;
                //                LOG_DEBUG("%s - %s\t Status:%u Val(raw):%u Val(float):%f",
                //                        __TAG__, __func__, status, (unsigned int) mbrtuMasterDt.entry[thisTag].value.uintVal, val);
                HMIDwin_TriggerSendValue(i, HMI_DATA_UINT, val);
            } else if (mbrtuMasterDt.entry[thisTag].dataType == DATA_INT) {
                float val = (float) mbrtuMasterDt.entry[thisTag].value.intVal;
                //                LOG_DEBUG("%s - %s\t Status:%u Val(raw):%d Val(float):%f",
                //                        __TAG__, __func__, status, (int) mbrtuMasterDt.entry[thisTag].value.intVal, val);
                HMIDwin_TriggerSendValue(i, HMI_DATA_INT, val);
            } else if (mbrtuMasterDt.entry[thisTag].dataType == DATA_FLOAT) {
                float val = mbrtuMasterDt.entry[thisTag].value.floatVal;
                //                LOG_DEBUG("%s - %s\t Status:%u Val(float):%f",
                //                        __TAG__, __func__, status, val);
                HMIDwin_TriggerSendValue(i, HMI_DATA_FLOAT, val);
            }
        }
    };
}

void MbrtuMaster_Initialize(void) {
    memset(&mbrtuMasterDt, 0, sizeof (MBRTU_MASTER_DATA));
    for (uint8_t i = 0; i < gMbrtuCfg.numTag; i++) {
        mbrtuMasterDt.entry[i].status = STATUS_IDENTIFYING;
    }

    modbus_app_init(_uartPlib);
}

void MbrtuMaster_Tasks(void) {
    static uint32_t pollTick = 0;
    static uint32_t timeoutTick = 0;
    static uint8_t retry = 0;
    static MBRTU_MASTER_STATES states;

    const MODBUSRTU_PHY_CONFIG * phyCfg = &gAppCfg.modbusRtu;
    MODBUSRTU_TAG_ENTRY * thisTag = &gMbrtuCfg.entry[_rowIndex];

#define NEXT_STATE(nextState)  \
    do { states = (nextState); } while(0)

    switch (states) {
        case MBRTU_MASTER_INIT:
        {
            UART_SERIAL_SETUP setup = {
                .baudRate = phyCfg->baudRate,
                .parity = phyCfg->parity,
                .dataWidth = UART_DATA_8_BIT,
                .stopBits = phyCfg->stopbits,
            };
            UART2_SerialSetup(&setup, 0);

            NEXT_STATE(MBRTU_MASTER_IDLE);
            break;
        }

        case MBRTU_MASTER_IDLE:
        {
            if (mbrtuMasterDt.reInit) {
                UART2_ReadAbort();
                if (UART2_ReadIsBusy() || UART2_WriteIsBusy())
                    break;
                mbrtuMasterDt.reInit = false;
                NEXT_STATE(MBRTU_MASTER_INIT);
            }

            if (TIME_IS_EXPIRED(pollTick, phyCfg->pollInterval)) {
                pollTick = TICK_NOW();
                //                LOG_DEBUG("%s - %s:\t MBRTU_MASTER_IDLE numtag = %u", __TAG__, __func__, gMbrtuCfg.numTag);
                if (gMbrtuCfg.numTag > 0) {
                    if (thisTag->enable || retry > 0) {
                        LedIndicate_SetMode(LED_ID_RTU, LED_MODE_BLINK_N_STOP, 1);
                        NEXT_STATE(MBRTU_MASTER_REQUEST);
                    } else {
                        if (!thisTag->enable)
                            mbrtuMasterDt.entry[_rowIndex].status = STATUS_DISABLE;
                        _rowIndex = _gotoNextRow(_rowIndex);
                    }
                }
            }
            break;
        }

        case MBRTU_MASTER_REQUEST:
        {
            //            LOG_DEBUG("%s - %s:\t MBRTU_MASTER_REQUEST", __TAG__, __func__);
            MODBUS_TRANSFER_DATA * mbTransfer = modbus_app_get_transfer_pointer();
            MODBUS_STATES mbState = MODBUS_IDLE;

            switch (thisTag->function) {
                case FUNC_READ_COILS:
                {
                    mbTransfer->start_address = thisTag->regAddress;
                    mbTransfer->quantity = thisTag->quantity;
                    mbState = TASK_FUNC_READ_COILS;
                    break;
                }

                case FUNC_READ_DISCRETE_INPUT:
                {
                    mbTransfer->start_address = thisTag->regAddress;
                    mbTransfer->quantity = thisTag->quantity;
                    mbState = TASK_FUNC_READ_DISCRETE_INPUT;
                    break;
                }

                case FUNC_READ_HOLDING_REGISTERS:
                {
                    mbTransfer->start_address = thisTag->regAddress;
                    mbTransfer->quantity = thisTag->quantity;
                    mbState = TASK_FUNC_READ_HOLDING_REGISTERS;
                    break;
                }

                case FUNC_READ_INPUT_REGISTERS:
                {
                    mbTransfer->start_address = thisTag->regAddress;
                    mbTransfer->quantity = thisTag->quantity;
                    mbState = TASK_FUNC_READ_INPUT_REGISTERS;
                    break;
                }

                default:
                    NEXT_STATE(MBRTU_MASTER_COMPLETE);
                    break;
            }

            mbTransfer->slave_address = thisTag->slaveAddress;
            timeoutTick = TICK_NOW();
            modbus_app_push_request(mbState);
            NEXT_STATE(MBRTU_MASTER_RESPONSE);
            break;
        }

        case MBRTU_MASTER_RESPONSE:
        {
            modbus_app_task();

            if (modbus_app_transfer_done()) {
                //                LOG_INFO("%s - %s:\t MBRTU_MASTER_RESPONSE success", __TAG__, __func__);
                retry = 0;
                LedIndicate_SetMode(LED_ID_RTU, LED_MODE_BLINK_N_STOP, 2);
                NEXT_STATE(MBRTU_MASTER_PARSE);
            } else if (TIME_IS_EXPIRED(timeoutTick, gAppCfg.modbusRtu.timeout)) {
                //                LOG_WARN("%s - %s:\t MBRTU_MASTER_RESPONSE timeout", __TAG__, __func__);
                retry++;
                if (retry >= gAppCfg.modbusRtu.retries) {
                    retry = 0;
                    mbrtuMasterDt.entry[_rowIndex].status = STATUS_BAD;
                    memset(&mbrtuMasterDt.entry[_rowIndex].value, 0, sizeof (MBRTU_PARSED_VALUE));
                    NEXT_STATE(MBRTU_MASTER_COMPLETE);
                    break;
                }
                pollTick = TICK_NOW();
                NEXT_STATE(MBRTU_MASTER_IDLE);
            }
            break;
        }

        case MBRTU_MASTER_PARSE:
        {
            _calculatorAdvancedScaling(_rowIndex);
            NEXT_STATE(MBRTU_MASTER_COMPLETE);
            break;
        }

        case MBRTU_MASTER_COMPLETE:
        {
            _printToHMI(_rowIndex);
            _rowIndex = _gotoNextRow(_rowIndex);
            pollTick = TICK_NOW();
            NEXT_STATE(MBRTU_MASTER_IDLE);
            break;
        }

        default:
            NEXT_STATE(MBRTU_MASTER_IDLE);
            break;
    }
}

MODBUSRTU_TAG_ENTRY * MbRtuMaster_GetCurrentTagConfig(void) {
    return &gMbrtuCfg.entry[_rowIndex];
}

void MbRtuMaster_SetCurrentTagData(MBRTU_RAW_VALUE raw, bool statusIsGood) {
    mbrtuMasterDt.entry[_rowIndex].status = (statusIsGood) ? STATUS_GOOD : STATUS_BAD;
    memcpy(&mbrtuMasterDt.entry[_rowIndex].rawValue, &raw, sizeof (MBRTU_RAW_VALUE));
}

uint16_t MbRtu_getValueFromIndex(uint8_t idxMb) {
    if (mbrtuMasterDt.entry[idxMb].dataType == DATA_UINT)
        return (uint16_t) (mbrtuMasterDt.entry[idxMb].value.uintVal);

    else if (mbrtuMasterDt.entry[idxMb].dataType == DATA_FLOAT)
        return (uint16_t) (mbrtuMasterDt.entry[idxMb].value.floatVal);

    else if (mbrtuMasterDt.entry[idxMb].dataType == DATA_INT)
        return (uint16_t) (mbrtuMasterDt.entry[idxMb].value.intVal);

    return 0;
}
