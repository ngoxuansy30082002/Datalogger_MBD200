#include "master.h"
#include "modbus_rtu_app.h"

MBRTU_MASTER_DATA mbrtuMasterDt;

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

static void _MBRTU_gotoNewRow(void) {
    //    if (mbCurrentRow < glbAppRtu.total_row - 1)
    //        mbCurrentRow++;
    //    else
    //        mbCurrentRow = 0;
}

static uint64_t _scaleUint(uint64_t input, SENSOR_SCALE_TYPE scaleType, float scaleValue) {
    switch (scaleType) {
        case SCALE_NONE:
            return input;
        case SCALE_LINEAR:
            return input * scaleValue;
        case SCALE_SQRT:
            return (uint64_t) (sqrt(input));
    }
    return input; // default
}

static int64_t _scaleInt(int64_t input, SENSOR_SCALE_TYPE scaleType, float scaleValue) {
    switch (scaleType) {
        case SCALE_NONE:
            return input;
        case SCALE_LINEAR:
            return input * scaleValue;
        case SCALE_SQRT:
            return (int64_t) (sqrt(input));
    }
    return input; // default
}

static float _scaleFloat(float input, SENSOR_SCALE_TYPE scaleType, float scaleValue) {
    switch (scaleType) {
        case SCALE_NONE:
            return input;
        case SCALE_LINEAR:
            return input * scaleValue;
        case SCALE_SQRT:
            return sqrt(input);
    }
    return input; // default
}

static uint64_t _calOffsetUint(uint8_t i) {
    uint64_t preOffsetValue = mbrtuMasterDt.value[i].uintVal;

    if (gMbrtuCfg.entry[i].offSetPreOperator == OPERATOR_NONE) {
        if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_ADDITION) {
            preOffsetValue = (uint64_t) (preOffsetValue + gMbrtuCfg.entry[i].offsetSubVal);
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_SUBTRACTION) {
            preOffsetValue = (uint64_t) (preOffsetValue - gMbrtuCfg.entry[i].offsetSubVal);
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_MULTIPLICATION) {
            preOffsetValue = (uint64_t) ((float) preOffsetValue * gMbrtuCfg.entry[i].offsetSubVal);
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_DIVISION && gMbrtuCfg.entry[i].offsetSubVal != 0) {
            preOffsetValue = (uint64_t) ((float) preOffsetValue / gMbrtuCfg.entry[i].offsetSubVal);
        }
    }

    if (gMbrtuCfg.entry[i].offSetPreOperator == OPERATOR_ADDITION) {
        if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_NONE) {
            preOffsetValue = (uint64_t) (gMbrtuCfg.entry[i].offsetPreVal + preOffsetValue);
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_ADDITION) {
            preOffsetValue = (uint64_t) (gMbrtuCfg.entry[i].offsetPreVal + preOffsetValue + gMbrtuCfg.entry[i].offsetSubVal);
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_SUBTRACTION) {
            preOffsetValue = (uint64_t) (gMbrtuCfg.entry[i].offsetPreVal + preOffsetValue - gMbrtuCfg.entry[i].offsetSubVal);
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_MULTIPLICATION) {
            preOffsetValue = (uint64_t) (gMbrtuCfg.entry[i].offsetPreVal + ((float) preOffsetValue * gMbrtuCfg.entry[i].offsetSubVal));
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_DIVISION && gMbrtuCfg.entry[i].offsetSubVal != 0) {
            preOffsetValue = (uint64_t) (gMbrtuCfg.entry[i].offsetPreVal + ((float) preOffsetValue / gMbrtuCfg.entry[i].offsetSubVal));
        }
    }

    if (gMbrtuCfg.entry[i].offSetPreOperator == OPERATOR_SUBTRACTION) {
        if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_NONE) {
            preOffsetValue = (uint64_t) (gMbrtuCfg.entry[i].offsetPreVal - preOffsetValue);
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_ADDITION) {
            preOffsetValue = (uint64_t) (gMbrtuCfg.entry[i].offsetPreVal - preOffsetValue + gMbrtuCfg.entry[i].offsetSubVal);
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_SUBTRACTION) {
            preOffsetValue = (uint64_t) (gMbrtuCfg.entry[i].offsetPreVal - preOffsetValue - gMbrtuCfg.entry[i].offsetSubVal);
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_MULTIPLICATION) {
            preOffsetValue = (uint64_t) (gMbrtuCfg.entry[i].offsetPreVal - ((float) preOffsetValue * gMbrtuCfg.entry[i].offsetSubVal));
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_DIVISION && gMbrtuCfg.entry[i].offsetSubVal != 0) {
            preOffsetValue = (uint64_t) (gMbrtuCfg.entry[i].offsetPreVal - ((float) preOffsetValue / gMbrtuCfg.entry[i].offsetSubVal));
        }
    }

    if (gMbrtuCfg.entry[i].offSetPreOperator == OPERATOR_MULTIPLICATION) {
        if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_NONE) {
            preOffsetValue = (uint64_t) (gMbrtuCfg.entry[i].offsetPreVal * preOffsetValue);
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_ADDITION) {
            preOffsetValue = (uint64_t) ((gMbrtuCfg.entry[i].offsetPreVal * (float) preOffsetValue) + gMbrtuCfg.entry[i].offsetSubVal);
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_SUBTRACTION) {
            preOffsetValue = (uint64_t) ((gMbrtuCfg.entry[i].offsetPreVal * (float) preOffsetValue) - gMbrtuCfg.entry[i].offsetSubVal);
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_MULTIPLICATION) {
            preOffsetValue = (uint64_t) (gMbrtuCfg.entry[i].offsetPreVal * (float) preOffsetValue * gMbrtuCfg.entry[i].offsetSubVal);
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_DIVISION && gMbrtuCfg.entry[i].offsetSubVal != 0) {
            preOffsetValue = (uint64_t) ((gMbrtuCfg.entry[i].offsetPreVal * (float) preOffsetValue) / gMbrtuCfg.entry[i].offsetSubVal);
        }
    }

    if (gMbrtuCfg.entry[i].offSetPreOperator == OPERATOR_DIVISION) {
        if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_NONE && preOffsetValue != 0) {
            preOffsetValue = (uint64_t) (gMbrtuCfg.entry[i].offsetPreVal / (float) preOffsetValue);
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_ADDITION && preOffsetValue != 0) {
            preOffsetValue = (uint64_t) ((gMbrtuCfg.entry[i].offsetPreVal / (float) preOffsetValue) + gMbrtuCfg.entry[i].offsetSubVal);
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_SUBTRACTION && preOffsetValue != 0) {
            preOffsetValue = (uint64_t) ((gMbrtuCfg.entry[i].offsetPreVal / (float) preOffsetValue) - gMbrtuCfg.entry[i].offsetSubVal);
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_MULTIPLICATION && preOffsetValue != 0) {
            preOffsetValue = (uint64_t) ((gMbrtuCfg.entry[i].offsetPreVal / (float) preOffsetValue) * gMbrtuCfg.entry[i].offsetSubVal);
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_DIVISION && gMbrtuCfg.entry[i].offsetSubVal != 0 && preOffsetValue != 0) {
            preOffsetValue = (uint64_t) ((gMbrtuCfg.entry[i].offsetPreVal / (float) preOffsetValue) / gMbrtuCfg.entry[i].offsetSubVal);
        }
    }
    return preOffsetValue;
}

static int64_t _calOffsetInt(uint8_t i) {
    int64_t preOffsetValue = mbrtuMasterDt.value[i].intVal;
    if (gMbrtuCfg.entry[i].offSetPreOperator == OPERATOR_NONE) {
        if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_ADDITION) {
            preOffsetValue = (int64_t) (preOffsetValue + gMbrtuCfg.entry[i].offsetSubVal);
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_SUBTRACTION) {
            preOffsetValue = (int64_t) (preOffsetValue - gMbrtuCfg.entry[i].offsetSubVal);
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_MULTIPLICATION) {
            preOffsetValue = (int64_t) ((float) preOffsetValue * gMbrtuCfg.entry[i].offsetSubVal);
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_DIVISION && gMbrtuCfg.entry[i].offsetSubVal != 0) {
            preOffsetValue = (int64_t) ((float) preOffsetValue / gMbrtuCfg.entry[i].offsetSubVal);
        }
    }

    if (gMbrtuCfg.entry[i].offSetPreOperator == OPERATOR_ADDITION) {
        if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_NONE) {
            preOffsetValue = (int64_t) (gMbrtuCfg.entry[i].offsetPreVal + preOffsetValue);
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_ADDITION) {
            preOffsetValue = (int64_t) (gMbrtuCfg.entry[i].offsetPreVal + preOffsetValue + gMbrtuCfg.entry[i].offsetSubVal);
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_SUBTRACTION) {
            preOffsetValue = (int64_t) (gMbrtuCfg.entry[i].offsetPreVal + preOffsetValue - gMbrtuCfg.entry[i].offsetSubVal);
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_MULTIPLICATION) {
            preOffsetValue = (int64_t) (gMbrtuCfg.entry[i].offsetPreVal + ((float) preOffsetValue * gMbrtuCfg.entry[i].offsetSubVal));
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_DIVISION && gMbrtuCfg.entry[i].offsetSubVal != 0) {
            preOffsetValue = (int64_t) (gMbrtuCfg.entry[i].offsetPreVal + ((float) preOffsetValue / gMbrtuCfg.entry[i].offsetSubVal));
        }
    }

    if (gMbrtuCfg.entry[i].offSetPreOperator == OPERATOR_SUBTRACTION) {
        if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_NONE) {
            preOffsetValue = (int64_t) (gMbrtuCfg.entry[i].offsetPreVal - preOffsetValue);
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_ADDITION) {
            preOffsetValue = (int64_t) (gMbrtuCfg.entry[i].offsetPreVal - preOffsetValue + gMbrtuCfg.entry[i].offsetSubVal);
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_SUBTRACTION) {
            preOffsetValue = (int64_t) (gMbrtuCfg.entry[i].offsetPreVal - preOffsetValue - gMbrtuCfg.entry[i].offsetSubVal);
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_MULTIPLICATION) {
            preOffsetValue = (int64_t) (gMbrtuCfg.entry[i].offsetPreVal - ((float) preOffsetValue * gMbrtuCfg.entry[i].offsetSubVal));
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_DIVISION && gMbrtuCfg.entry[i].offsetSubVal != 0) {
            preOffsetValue = (int64_t) (gMbrtuCfg.entry[i].offsetPreVal - ((float) preOffsetValue / gMbrtuCfg.entry[i].offsetSubVal));
        }
    }

    if (gMbrtuCfg.entry[i].offSetPreOperator == OPERATOR_MULTIPLICATION) {
        if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_NONE) {
            preOffsetValue = (int64_t) (gMbrtuCfg.entry[i].offsetPreVal * preOffsetValue);
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_ADDITION) {
            preOffsetValue = (int64_t) ((gMbrtuCfg.entry[i].offsetPreVal * (float) preOffsetValue) + gMbrtuCfg.entry[i].offsetSubVal);
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_SUBTRACTION) {
            preOffsetValue = (int64_t) ((gMbrtuCfg.entry[i].offsetPreVal * (float) preOffsetValue) - gMbrtuCfg.entry[i].offsetSubVal);
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_MULTIPLICATION) {
            preOffsetValue = (int64_t) (gMbrtuCfg.entry[i].offsetPreVal * (float) preOffsetValue * gMbrtuCfg.entry[i].offsetSubVal);
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_DIVISION && gMbrtuCfg.entry[i].offsetSubVal != 0) {
            preOffsetValue = (int64_t) ((gMbrtuCfg.entry[i].offsetPreVal * (float) preOffsetValue) / gMbrtuCfg.entry[i].offsetSubVal);
        }
    }

    if (gMbrtuCfg.entry[i].offSetPreOperator == OPERATOR_DIVISION) {
        if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_NONE && preOffsetValue != 0) {
            preOffsetValue = (int64_t) (gMbrtuCfg.entry[i].offsetPreVal / (float) preOffsetValue);
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_ADDITION && preOffsetValue != 0) {
            preOffsetValue = (int64_t) ((gMbrtuCfg.entry[i].offsetPreVal / (float) preOffsetValue) + gMbrtuCfg.entry[i].offsetSubVal);
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_SUBTRACTION && preOffsetValue != 0) {
            preOffsetValue = (int64_t) ((gMbrtuCfg.entry[i].offsetPreVal / (float) preOffsetValue) - gMbrtuCfg.entry[i].offsetSubVal);
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_MULTIPLICATION && preOffsetValue != 0) {
            preOffsetValue = (int64_t) ((gMbrtuCfg.entry[i].offsetPreVal / (float) preOffsetValue) * gMbrtuCfg.entry[i].offsetSubVal);
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_DIVISION && gMbrtuCfg.entry[i].offsetSubVal != 0 && preOffsetValue != 0) {
            preOffsetValue = (int64_t) ((gMbrtuCfg.entry[i].offsetPreVal / (float) preOffsetValue) / gMbrtuCfg.entry[i].offsetSubVal);
        }
    }
    return preOffsetValue;
}

static float _calOffsetFloat(uint8_t i) {
    float preOffsetValue = mbrtuMasterDt.value[i].floatVal;
    if (gMbrtuCfg.entry[i].offSetPreOperator == OPERATOR_NONE) {
        if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_ADDITION) {
            preOffsetValue = (float) (preOffsetValue + gMbrtuCfg.entry[i].offsetSubVal);
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_SUBTRACTION) {
            preOffsetValue = (float) (preOffsetValue - gMbrtuCfg.entry[i].offsetSubVal);
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_MULTIPLICATION) {
            preOffsetValue = (float) (preOffsetValue * gMbrtuCfg.entry[i].offsetSubVal);
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_DIVISION && gMbrtuCfg.entry[i].offsetSubVal != 0) {
            preOffsetValue = (float) (preOffsetValue / gMbrtuCfg.entry[i].offsetSubVal);
        }
    }

    if (gMbrtuCfg.entry[i].offSetPreOperator == OPERATOR_ADDITION) {
        if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_NONE) {
            preOffsetValue = (float) (gMbrtuCfg.entry[i].offsetPreVal + preOffsetValue);
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_ADDITION) {
            preOffsetValue = (float) (gMbrtuCfg.entry[i].offsetPreVal + preOffsetValue + gMbrtuCfg.entry[i].offsetSubVal);
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_SUBTRACTION) {
            preOffsetValue = (float) (gMbrtuCfg.entry[i].offsetPreVal + preOffsetValue - gMbrtuCfg.entry[i].offsetSubVal);
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_MULTIPLICATION) {
            preOffsetValue = (float) (gMbrtuCfg.entry[i].offsetPreVal + (preOffsetValue * gMbrtuCfg.entry[i].offsetSubVal));
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_DIVISION && gMbrtuCfg.entry[i].offsetSubVal != 0) {
            preOffsetValue = (float) (gMbrtuCfg.entry[i].offsetPreVal + (preOffsetValue / gMbrtuCfg.entry[i].offsetSubVal));
        }
    }

    if (gMbrtuCfg.entry[i].offSetPreOperator == OPERATOR_SUBTRACTION) {
        if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_NONE) {
            preOffsetValue = (float) (gMbrtuCfg.entry[i].offsetPreVal - preOffsetValue);
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_ADDITION) {
            preOffsetValue = (float) (gMbrtuCfg.entry[i].offsetPreVal - preOffsetValue + gMbrtuCfg.entry[i].offsetSubVal);
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_SUBTRACTION) {
            preOffsetValue = (float) (gMbrtuCfg.entry[i].offsetPreVal - preOffsetValue - gMbrtuCfg.entry[i].offsetSubVal);
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_MULTIPLICATION) {
            preOffsetValue = (float) (gMbrtuCfg.entry[i].offsetPreVal - (preOffsetValue * gMbrtuCfg.entry[i].offsetSubVal));
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_DIVISION && gMbrtuCfg.entry[i].offsetSubVal != 0) {
            preOffsetValue = (float) (gMbrtuCfg.entry[i].offsetPreVal - (preOffsetValue / gMbrtuCfg.entry[i].offsetSubVal));
        }
    }

    if (gMbrtuCfg.entry[i].offSetPreOperator == OPERATOR_MULTIPLICATION) {
        if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_NONE) {
            preOffsetValue = (float) (gMbrtuCfg.entry[i].offsetPreVal * preOffsetValue);
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_ADDITION) {
            preOffsetValue = (float) ((gMbrtuCfg.entry[i].offsetPreVal * preOffsetValue) + gMbrtuCfg.entry[i].offsetSubVal);
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_SUBTRACTION) {
            preOffsetValue = (float) ((gMbrtuCfg.entry[i].offsetPreVal * preOffsetValue) - gMbrtuCfg.entry[i].offsetSubVal);
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_MULTIPLICATION) {
            preOffsetValue = (float) (gMbrtuCfg.entry[i].offsetPreVal * preOffsetValue * gMbrtuCfg.entry[i].offsetSubVal);
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_DIVISION && gMbrtuCfg.entry[i].offsetSubVal != 0) {
            preOffsetValue = (float) ((gMbrtuCfg.entry[i].offsetPreVal * preOffsetValue) / gMbrtuCfg.entry[i].offsetSubVal);
        }
    }

    if (gMbrtuCfg.entry[i].offSetPreOperator == OPERATOR_DIVISION) {
        if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_NONE && preOffsetValue != 0) {
            preOffsetValue = (float) (gMbrtuCfg.entry[i].offsetPreVal / preOffsetValue);
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_ADDITION && preOffsetValue != 0) {
            preOffsetValue = (float) ((gMbrtuCfg.entry[i].offsetPreVal / preOffsetValue) + gMbrtuCfg.entry[i].offsetSubVal);
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_SUBTRACTION && preOffsetValue != 0) {
            preOffsetValue = (float) ((gMbrtuCfg.entry[i].offsetPreVal / preOffsetValue) - gMbrtuCfg.entry[i].offsetSubVal);
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_MULTIPLICATION && preOffsetValue != 0) {
            preOffsetValue = (float) ((gMbrtuCfg.entry[i].offsetPreVal / preOffsetValue) * gMbrtuCfg.entry[i].offsetSubVal);
        } else if (gMbrtuCfg.entry[i].offsetSubOperator == OPERATOR_DIVISION && gMbrtuCfg.entry[i].offsetSubVal != 0 && preOffsetValue != 0) {
            preOffsetValue = (float) ((gMbrtuCfg.entry[i].offsetPreVal / preOffsetValue) / gMbrtuCfg.entry[i].offsetSubVal);
        }
    }
    return preOffsetValue;
}

static void _applyScale(uint8_t i) {
    SENSOR_SCALE_TYPE scaleType = gMbrtuCfg.entry[i].scaleType;
    SENSOR_DATA_TYPE scaleDataType = gMbrtuCfg.entry[i].scaleDataType;
    float scaleValue = gMbrtuCfg.entry[i].scaleValue;
    SENSOR_DATA_TYPE rawDataType = gMbrtuCfg.entry[i].rawDataType;
    MBRTU_RAW_VALUE rawValue = mbrtuMasterDt.rawValue[i];
    bool negativeValFlag = false;

    switch (rawDataType) {
        case DATA_RAW:
        {

            break;
        }
        case DATA_UINT:
        {
#if defined(DEBUG_MODULE_MBRTU) || defined(DEBUG_MODULE_ALL)
            SYS_CONSOLE_PRINT("reuint: %u --------", rawValue.reuint);
#endif
            if (scaleDataType == DATA_UINT) {
                mbrtuMasterDt.value[i].uintVal = (uint64_t) _scaleUint(rawValue.reuint, scaleType, scaleValue);
                //                SYS_CONSOLE_PRINT("u_val: %u \r\n", mbrtuMasterDt.value[i].uintVal);
                if (gMbrtuCfg.entry[i].adcType == ADC_4_20mA) {
                    float step = (gMbrtuCfg.entry[i].adcHigh - gMbrtuCfg.entry[i].adcLow) / 16;
                    mbrtuMasterDt.value[i].uintVal = ((float) mbrtuMasterDt.value[i].uintVal - 4) * step + gMbrtuCfg.entry[i].adcLow;
                }
                mbrtuMasterDt.value[i].uintVal = _calOffsetUint(i);

#if defined(DEBUG_MODULE_MBRTU) || defined(DEBUG_MODULE_ALL)
                SYS_CONSOLE_PRINT("u_val222: %u \r\n", mbrtuMasterDt.value[i].uintVal);
#endif
            }
            if (scaleDataType == DATA_INT) {
                mbrtuMasterDt.value[i].intVal = (int64_t) _scaleInt((int64_t) rawValue.reuint, scaleType, scaleValue);
                //                SYS_CONSOLE_PRINT("i_val: %d \r\n", mbrtuMasterDt.value[i].intVal);
                if (gMbrtuCfg.entry[i].adcType == ADC_4_20mA) //4-20mA
                {
                    float step = (gMbrtuCfg.entry[i].adcHigh - gMbrtuCfg.entry[i].adcLow) / 16;
                    mbrtuMasterDt.value[i].intVal = ((float) mbrtuMasterDt.value[i].intVal - 4) * step + gMbrtuCfg.entry[i].adcLow;
                }
                mbrtuMasterDt.value[i].intVal = _calOffsetInt(i);

#if defined(DEBUG_MODULE_MBRTU) || defined(DEBUG_MODULE_ALL)
                SYS_CONSOLE_PRINT("i_val222: %u \r\n", mbrtuMasterDt.value[i].intVal);
#endif

            }
            if (scaleDataType == DATA_FLOAT) {
                mbrtuMasterDt.value[i].floatVal = (float) _scaleFloat((float) rawValue.reuint, scaleType, scaleValue);
                //                SYS_CONSOLE_PRINT("f_val: %f \r\n", mbrtuMasterDt.value[i].floatVal);
                if (gMbrtuCfg.entry[i].adcType == ADC_4_20mA) //4-20mA
                {
                    float step = (gMbrtuCfg.entry[i].adcHigh - gMbrtuCfg.entry[i].adcLow) / 16;
                    mbrtuMasterDt.value[i].floatVal = ((float) mbrtuMasterDt.value[i].floatVal - 4) * step + gMbrtuCfg.entry[i].adcLow;
                }
                mbrtuMasterDt.value[i].floatVal = _calOffsetFloat(i);

#if defined(DEBUG_MODULE_MBRTU) || defined(DEBUG_MODULE_ALL)
                SYS_CONSOLE_PRINT("f_val222: %.5f \r\n", mbrtuMasterDt.value[i].floatVal);
#endif
            }
            break;
        }
        case DATA_INT:
        {
#if defined(DEBUG_MODULE_MBRTU) || defined(DEBUG_MODULE_ALL)
            SYS_CONSOLE_PRINT("reint: %d --------", rawValue.reint);
#endif
            if (rawValue.reint < 0) {
                rawValue.reint = abs(rawValue.reint);
                negativeValFlag = true;
            }
            if (scaleDataType == DATA_UINT) {
                mbrtuMasterDt.value[i].uintVal = (uint64_t) _scaleUint(rawValue.reint, scaleType, scaleValue);
                //                SYS_CONSOLE_PRINT("u_val: %u \r\n", mbrtuMasterDt.value[i].uintVal);
                if (gMbrtuCfg.entry[i].adcType == ADC_4_20mA) //4-20mA
                {
                    float step = (gMbrtuCfg.entry[i].adcHigh - gMbrtuCfg.entry[i].adcLow) / 16;
                    mbrtuMasterDt.value[i].uintVal = ((float) mbrtuMasterDt.value[i].uintVal - 4) * step + gMbrtuCfg.entry[i].adcLow;
                }
                mbrtuMasterDt.value[i].uintVal = _calOffsetUint(i);

#if defined(DEBUG_MODULE_MBRTU) || defined(DEBUG_MODULE_ALL)
                SYS_CONSOLE_PRINT("u_val222: %u \r\n", mbrtuMasterDt.value[i].uintVal);
#endif
            }
            if (scaleDataType == DATA_INT) {
                mbrtuMasterDt.value[i].intVal = (int64_t) _scaleInt((int64_t) rawValue.reint, scaleType, scaleValue);
                if (negativeValFlag) {
                    mbrtuMasterDt.value[i].intVal = -mbrtuMasterDt.value[i].intVal;
                    negativeValFlag = false;
                }
                //                SYS_CONSOLE_PRINT("i_val: %d \r\n", mbrtuMasterDt.value[i].intVal);
                if (gMbrtuCfg.entry[i].adcType == ADC_4_20mA) //4-20mA
                {
                    float step = (gMbrtuCfg.entry[i].adcHigh - gMbrtuCfg.entry[i].adcLow) / 16;
                    mbrtuMasterDt.value[i].intVal = ((float) mbrtuMasterDt.value[i].intVal - 4) * step + gMbrtuCfg.entry[i].adcLow;
                }
                mbrtuMasterDt.value[i].intVal = _calOffsetInt(i);

#if defined(DEBUG_MODULE_MBRTU) || defined(DEBUG_MODULE_ALL)
                SYS_CONSOLE_PRINT("i_val222: %d \r\n", mbrtuMasterDt.value[i].intVal);
#endif
            }
            if (scaleDataType == DATA_FLOAT) {
                mbrtuMasterDt.value[i].floatVal = (float) _scaleFloat((float) rawValue.reint, scaleType, scaleValue);
                if (negativeValFlag) {
                    mbrtuMasterDt.value[i].floatVal = -mbrtuMasterDt.value[i].floatVal;
                    negativeValFlag = false;
                }
                //                SYS_CONSOLE_PRINT("f_val: %f \r\n", mbrtuMasterDt.value[i].floatVal);
                if (gMbrtuCfg.entry[i].adcType == ADC_4_20mA) //4-20mA
                {
                    float step = (gMbrtuCfg.entry[i].adcHigh - gMbrtuCfg.entry[i].adcLow) / 16;
                    mbrtuMasterDt.value[i].floatVal = ((float) mbrtuMasterDt.value[i].floatVal - 4) * step + gMbrtuCfg.entry[i].adcLow;
                }
                mbrtuMasterDt.value[i].floatVal = _calOffsetFloat(i);

#if defined(DEBUG_MODULE_MBRTU) || defined(DEBUG_MODULE_ALL)
                SYS_CONSOLE_PRINT("f_val222: %f \r\n", mbrtuMasterDt.value[i].floatVal);
#endif
            }
            break;
        }
        case DATA_FLOAT:
        {
#if defined(DEBUG_MODULE_MBRTU) || defined(DEBUG_MODULE_ALL)
            SYS_CONSOLE_PRINT("refloat: %lf --------", rawValue.refloat.f);
#endif

            if (rawValue.refloat.f < 0) {
                rawValue.refloat.f = -rawValue.refloat.f;
                negativeValFlag = true;
            }
            if (scaleDataType == DATA_UINT) {
                mbrtuMasterDt.value[i].uintVal = (uint64_t) _scaleFloat(rawValue.refloat.f, scaleType, scaleValue);
                //                SYS_CONSOLE_PRINT("u_val: %u \r\n", mbrtuMasterDt.value[i].uintVal);
                if (gMbrtuCfg.entry[i].adcType == ADC_4_20mA) //4-20mA
                {
                    float step = (gMbrtuCfg.entry[i].adcHigh - gMbrtuCfg.entry[i].adcLow) / 16;
                    mbrtuMasterDt.value[i].uintVal = ((float) mbrtuMasterDt.value[i].uintVal - 4) * step + gMbrtuCfg.entry[i].adcLow;
                }
                mbrtuMasterDt.value[i].uintVal = _calOffsetUint(i);

#if defined(DEBUG_MODULE_MBRTU) || defined(DEBUG_MODULE_ALL)
                SYS_CONSOLE_PRINT("u_val222: %u \r\n", mbrtuMasterDt.value[i].uintVal);
#endif
            }
            if (scaleDataType == DATA_INT) {
                mbrtuMasterDt.value[i].intVal = (int64_t) _scaleFloat(rawValue.refloat.f, scaleType, scaleValue);
                if (negativeValFlag) {
                    mbrtuMasterDt.value[i].intVal = -mbrtuMasterDt.value[i].intVal;
                    negativeValFlag = false;
                }
                //                SYS_CONSOLE_PRINT("i_val: %d \r\n", mbrtuMasterDt.value[i].intVal);
                if (gMbrtuCfg.entry[i].adcType == ADC_4_20mA) //4-20mA
                {
                    float step = (gMbrtuCfg.entry[i].adcHigh - gMbrtuCfg.entry[i].adcLow) / 16;
                    mbrtuMasterDt.value[i].intVal = ((float) mbrtuMasterDt.value[i].intVal - 4) * step + gMbrtuCfg.entry[i].adcLow;
                }
                mbrtuMasterDt.value[i].intVal = _calOffsetInt(i);

#if defined(DEBUG_MODULE_MBRTU) || defined(DEBUG_MODULE_ALL)
                SYS_CONSOLE_PRINT("i_val222: %d \r\n", mbrtuMasterDt.value[i].intVal);
#endif
            }
            if (scaleDataType == DATA_FLOAT) {
                mbrtuMasterDt.value[i].floatVal = (float) _scaleFloat((float) rawValue.refloat.f, scaleType, scaleValue);
                if (negativeValFlag) {
                    mbrtuMasterDt.value[i].floatVal = -mbrtuMasterDt.value[i].floatVal;
                    negativeValFlag = false;
                }
                //                SYS_CONSOLE_PRINT("f_val: %f \r\n", mbrtuMasterDt.value[i].floatVal);
                if (gMbrtuCfg.entry[i].adcType == ADC_4_20mA) //4-20mA
                {
                    float step = (gMbrtuCfg.entry[i].adcHigh - gMbrtuCfg.entry[i].adcLow) / 16;
                    mbrtuMasterDt.value[i].floatVal = ((float) mbrtuMasterDt.value[i].floatVal - 4) * step + gMbrtuCfg.entry[i].adcLow;
                }
                mbrtuMasterDt.value[i].floatVal = _calOffsetFloat(i);

#if defined(DEBUG_MODULE_MBRTU) || defined(DEBUG_MODULE_ALL)
                SYS_CONSOLE_PRINT("f_val222: %f \r\n", mbrtuMasterDt.value[i].floatVal);
#endif
            }
            break;
        }
    }
}

void MbrtuMaster_Initialize(void) {
    mbrtuMasterDt.state = 0;
    memset(&mbrtuMasterDt, 0, sizeof (MBRTU_MASTER_DATA));
    for (uint8_t i = 0; i < gMbrtuCfg.numTag; i++) {
        mbrtuMasterDt.status[i] = UNKNOWN;
    }

    modbus_app_init(_uartPlib);
}

void MbrtuMaster_Tasks(void) {
    static uint32_t pollTick = 0;
    static uint32_t timeoutTick = 0;
    static uint8_t retry = 0;
    const MODBUSRTU_PHY_CONFIG * phyCfg = &gAppCfg.modbusRtu;

#define NEXT_STATE(nextState)  \
    do { mbrtuMasterDt.state = (nextState); } while(0)

    switch (mbrtuMasterDt.state) {
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
                if (gMbrtuCfg.numTag > 0) {
                    if (gMbrtuCfg.entry[_rowIndex].enable || retry > 0) {
                        NEXT_STATE(MBRTU_MASTER_REQUEST);
                        mbrtuMasterDt.led = 1;
                    } else {
                        if (!gMbrtuCfg.entry[_rowIndex].enable)
                            mbrtuMasterDt.status[_rowIndex] = UNKNOWN;
                        mbrtuMasterDt.led = 0;
                        _MBRTU_gotoNewRow();
                    }
                }
            }
            break;
        }
        case MBRTU_MASTER_REQUEST:
        {
            MODBUS_TRANSFER_DATA * mbTransfer = modbus_app_get_transfer_pointer();
            MODBUS_STATES mbState = MODBUS_IDLE;

            switch (gMbrtuCfg.entry[_rowIndex].function) {
                case FUNC_READ_COILS: // function read coils
                {
                    mbTransfer->start_address = gMbrtuCfg.entry[_rowIndex].regAddress;
                    mbTransfer->quantity = gMbrtuCfg.entry[_rowIndex].quantity;
                    mbState = TASK_FUNC_READ_COILS;
                    break;
                }
                case FUNC_READ_DISCRETE_INPUT: // function read input
                {
                    mbTransfer->start_address = gMbrtuCfg.entry[_rowIndex].regAddress;
                    mbTransfer->quantity = gMbrtuCfg.entry[_rowIndex].quantity;
                    mbState = TASK_FUNC_READ_DISCRETE_INPUT;
                    break;
                }
                case FUNC_READ_HOLDING_REGISTERS: // function read holding register
                {
                    mbTransfer->start_address = gMbrtuCfg.entry[_rowIndex].regAddress;
                    mbTransfer->quantity = gMbrtuCfg.entry[_rowIndex].quantity;
                    mbState = TASK_FUNC_READ_HOLDING_REGISTERS;
                    break;
                }
                case FUNC_READ_INPUT_REGISTERS: // function read input register
                {
                    mbTransfer->start_address = gMbrtuCfg.entry[_rowIndex].regAddress;
                    mbTransfer->quantity = gMbrtuCfg.entry[_rowIndex].quantity;
                    mbState = TASK_FUNC_READ_INPUT_REGISTERS;
                    break;
                }
            }
            mbTransfer->slave_address = gMbrtuCfg.entry[_rowIndex].slaveAddress;
            timeoutTick = TICK_NOW();
            modbus_app_push_request(mbState);
            NEXT_STATE(MBRTU_MASTER_RESPONSE);
            break;
        }
        case MBRTU_MASTER_RESPONSE:
        {
            modbus_app_task();

            if (modbus_app_transfer_done()) {
                mbrtuMasterDt.led = 0;
                retry = 0;
                NEXT_STATE(MBRTU_MASTER_PARSE);
            } else if (TIME_IS_EXPIRED(timeoutTick, gAppCfg.modbusRtu.timeout)) {
                retry++;
                mbrtuMasterDt.led = 0;
                if (retry >= gAppCfg.modbusRtu.retries) {
                    retry = 0;
                    mbrtuMasterDt.status[_rowIndex] = BAD;
                    memset(&mbrtuMasterDt.value[_rowIndex], 0, sizeof (MBRTU_PARSED_VALUE));
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
            _applyScale(_rowIndex);

            mbrtuMasterDt.state = MBRTU_MASTER_COMPLETE;
            break;
        }
        case MBRTU_MASTER_COMPLETE:
        {
            //            _MBRTU_PrintHMI(mbCurrentRow, (mbrtuMasterDt.mbRTUQuality[mbCurrentRow] != GOOD));
            //
            //            _MBRTU_gotoNewRow();
            //            _mbPollTick = SYS_TMR_TickCountGet();
            //            mbrtuMasterDt.state = MBRTU_MASTER_IDLE;
            break;
        }
        default:
            mbrtuMasterDt.state = MBRTU_MASTER_IDLE;
            break;
    }
}

uint16_t MBRTU_getStatusFromMbRtu(uint8_t idxMb) {
    //    if (glbAppRtu.analog_modbus[idxMb].scaled_data_type == OPERATOR_ADDITION) //uint
    //        return (uint16_t) (mbrtuMasterDt.value[idxMb].value_uint);
    //
    //    else if (glbAppRtu.analog_modbus[idxMb].scaled_data_type == OPERATOR_SUBTRACTION) //float
    //        return (uint16_t) (mbrtuMasterDt.value[idxMb].value_float);
    //
    //    else if (glbAppRtu.analog_modbus[idxMb].scaled_data_type == OPERATOR_DIVISION) //int
    //        return (uint16_t) (mbrtuMasterDt.value[idxMb].value_int);
    //
    //    return 0;
}

static void _MBRTU_PrintHMI(uint8_t mbCurrentRow, bool noGood) {
    //    for (uint8_t i = 0; i < MAX_HMI_PARA; i++) {
    //        uint8_t idxHMI = glbAppCfg.tag_hmi[i];
    //        if (idxHMI == OPERATOR_MULTIPLICATION0 || glbAppCfg.sensor.entry[idxHMI].enable == false)
    //            continue;
    //
    //        if (glbAppCfg.sensor.entry[idxHMI].type == SENSOR_RTU &&
    //                glbAppCfg.sensor.entry[idxHMI].idxInType == mbCurrentRow) {
    //            uint8_t status = 2;
    //            int8_t stt = APP_CalculateStatusSensor(idxHMI);
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
    //
    //            if (glbAppRtu.analog_modbus[mbCurrentRow].scaled_data_type == OPERATOR_ADDITION) //uint
    //                HMIDwin_TriggerSendValue(i, HMI_DATA_UINT, (float) mbrtuMasterDt.value[mbCurrentRow].value_uint);
    //
    //            if (glbAppRtu.analog_modbus[mbCurrentRow].scaled_data_type == OPERATOR_SUBTRACTION) //float
    //                HMIDwin_TriggerSendValue(i, HMI_DATA_FLOAT, (float) mbrtuMasterDt.value[mbCurrentRow].value_float);
    //
    //            if (glbAppRtu.analog_modbus[mbCurrentRow].scaled_data_type == OPERATOR_DIVISION) //int
    //                HMIDwin_TriggerSendValue(i, HMI_DATA_INT, (float) mbrtuMasterDt.value[mbCurrentRow].value_int);
    //
    //            //            (void) status;
    //        }
    //    }
}


