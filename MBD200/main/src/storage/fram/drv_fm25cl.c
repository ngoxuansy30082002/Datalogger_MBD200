#include "drv_fm25cl.h"

static DRV_FM25CL_OBJECT _gDrvFM25CLObj = {0};
static DRV_FM25CL_OBJECT *dObj = &_gDrvFM25CLObj;
static const char _metadata[11] = "BKLOGY JSC";
static CACHE_ALIGN uint8_t _txBuffer[2048] = {0};
static CACHE_ALIGN uint8_t _rxBuffer[2048] = {0};
static CACHE_ALIGN uint8_t _opcode = 0;

static const DRV_FM25CL_PLIB_INTERFACE _fm25clPlib = {
    .writeRead = (DRV_FM25CL_PLIB_WRITE_READ) SPI4_WriteRead,
    .write_t = (DRV_FM25CL_PLIB_WRITE) SPI4_Write,
    .read_t = (DRV_FM25CL_PLIB_READ) SPI4_Read,
    .isBusy = (DRV_FM25CL_PLIB_IS_BUSY) SPI4_IsBusy,
    .callbackRegister = (DRV_FM25CL_PLIB_CALLBACK_REGISTER) SPI4_CallbackRegister,
    .chipSelectPin = DRV_FM25CL_CHIP_SELECT_PIN,
    .resetPin = DRV_FM25CL_RESET_PIN,
};

bool _SPIWriteRead(DRV_FM25CL_OBJECT* dObj, DRV_FM25CL_TRANSFER_OBJ* transferObj) {
    GPIO_PinClear(_fm25clPlib.chipSelectPin);
    Nop();
    Nop();
    dObj->transferStatus = DRV_FM25CL_TRANSFER_BUSY;
    return _fm25clPlib.writeRead(transferObj->pTransmitData, transferObj->txSize, transferObj->pReceiveData, transferObj->rxSize);
}

static void _sendCommand(uint8_t opcode) {
    _opcode = opcode;

    dObj->transferCmdObj.pTransmitData = &_opcode;
    dObj->transferCmdObj.txSize = 1;
    dObj->transferCmdObj.pReceiveData = NULL;
    dObj->transferCmdObj.rxSize = 0;

    dObj->isOpcodeCommand = true;
    (void) _SPIWriteRead(dObj, &dObj->transferCmdObj);
}

bool DrvFM25CL_readMultiRegister(uint16_t address, uint8_t *outdata, uint16_t readLen) {
    if (readLen + DRV_FM25CL_HEADER_LEN > sizeof (_txBuffer)) return false;

    _txBuffer[0] = DRV_FM25CL_OPCODE_READ;
    _txBuffer[1] = (address >> 8) & 0xFF;
    _txBuffer[2] = (address >> 0) & 0xFF;

    dObj->state = DRV_FM25CL_STATE_READ;
    dObj->transferStatus = DRV_FM25CL_TRANSFER_BUSY;

    dObj->transferDataObj.pTransmitData = _txBuffer;
    dObj->transferDataObj.txSize = readLen + DRV_FM25CL_HEADER_LEN;
    dObj->transferDataObj.pReceiveData = _rxBuffer;
    dObj->transferDataObj.pOutData = outdata;
    dObj->transferDataObj.rxSize = readLen + DRV_FM25CL_HEADER_LEN;

    dObj->isOpcodeCommand = false;
    (void) _SPIWriteRead(dObj, &dObj->transferDataObj);
    return true;
}

bool DrvFM25CL_writeMultiRegister(uint16_t address, const uint8_t *data, uint16_t writeLen) {
    if (writeLen + DRV_FM25CL_HEADER_LEN > sizeof (_txBuffer)) return false;

    _txBuffer[0] = DRV_FM25CL_OPCODE_WRITE;
    _txBuffer[1] = (address >> 8) & 0xFF;
    _txBuffer[2] = (address >> 0) & 0xFF;
    memcpy(&_txBuffer[DRV_FM25CL_HEADER_LEN], data, writeLen);

    dObj->state = DRV_FM25CL_STATE_WRITE_ENABLE;
    dObj->transferStatus = DRV_FM25CL_TRANSFER_BUSY;

    dObj->transferDataObj.pTransmitData = _txBuffer;
    dObj->transferDataObj.txSize = writeLen + DRV_FM25CL_HEADER_LEN;
    dObj->transferDataObj.pReceiveData = NULL;
    dObj->transferDataObj.rxSize = 0;

    _sendCommand(DRV_FM25CL_OPCODE_WREN);
    return true;
}

void DrvFM25CL_Handler(void) {
    switch (dObj->state) {
        case DRV_FM25CL_STATE_WRITE_ENABLE:
        {
            GPIO_PinSet(_fm25clPlib.chipSelectPin);
            Nop();
            Nop();
            Nop();
            Nop();
            Nop();
            Nop();
            Nop();
            dObj->state = DRV_FM25CL_STATE_WRITE;
            dObj->isOpcodeCommand = false;
            (void) _SPIWriteRead(dObj, &dObj->transferDataObj);
            break;
        }
        case DRV_FM25CL_STATE_READ:
        {
            GPIO_PinSet(_fm25clPlib.chipSelectPin);
            memmove(dObj->transferDataObj.pOutData, dObj->transferDataObj.pReceiveData + DRV_FM25CL_HEADER_LEN,
                    dObj->transferDataObj.rxSize - DRV_FM25CL_HEADER_LEN);

            dObj->transferStatus = DRV_FM25CL_TRANSFER_COMPLETED;
            break;
        }
        default:
        {
            GPIO_PinSet(_fm25clPlib.chipSelectPin);
            dObj->transferStatus = DRV_FM25CL_TRANSFER_COMPLETED;
            break;
        }
    }
    /* If transfer is complete, notify the application */
    if (dObj->transferStatus != DRV_FM25CL_TRANSFER_BUSY) {
        if (dObj->eventHandler != NULL) {
            dObj->eventHandler(dObj->transferStatus, dObj->context);
        }
    }
}

void _SPIPlibCallbackHandler(uintptr_t context) {
    DRV_FM25CL_OBJECT* dObj = (DRV_FM25CL_OBJECT*) context;

    DrvFM25CL_Handler();

    if (dObj->isOpcodeCommand) {
        dObj->transferCmdObj.txSize = dObj->transferCmdObj.rxSize = 0;
        dObj->transferCmdObj.pTransmitData = dObj->transferCmdObj.pReceiveData = NULL;
    } else {
        dObj->transferDataObj.txSize = dObj->transferDataObj.rxSize = 0;
        dObj->transferDataObj.pTransmitData = dObj->transferDataObj.pReceiveData = NULL;
    }
}

bool DrvFM25CL_Initialize() {
    /* Check if the instance has already been initialized. */
    dObj->status = SYS_STATUS_UNINITIALIZED;
    dObj->openState = DRV_FM25CL_OPEN_READ_METADATA;

    _fm25clPlib.callbackRegister(_SPIPlibCallbackHandler, (uintptr_t) dObj);
    /* De-assert Chip Select pin to begin with. */
    GPIO_PinSet(_fm25clPlib.chipSelectPin);
    if (_fm25clPlib.resetPin != GPIO_PIN_NONE)
        GPIO_PinClear(_fm25clPlib.resetPin);

    dObj->transferStatus = DRV_FM25CL_TRANSFER_COMPLETED;
    /* Return the driver index */
    return true;
}

SYS_STATUS DrvFM25CL_Open() {
    static DRV_FM25CL_OPEN_STATE nextState;
    static uint8_t initDataRx[32];
    static uint8_t numRetry = 0;

    if (_fm25clPlib.resetPin != GPIO_PIN_NONE)
        GPIO_PinSet(_fm25clPlib.resetPin);

    switch (dObj->openState) {
        case DRV_FM25CL_OPEN_READ_METADATA:
        {
            memset(initDataRx, 0, sizeof (initDataRx));
            DrvFM25CL_readMultiRegister(0, initDataRx, sizeof (_metadata));
            nextState = DRV_FM25CL_OPEN_VERIFY_METADATA;
            dObj->openState = DRV_FM25CL_OPEN_WAIT_FOR_TRANSFER;
            break;
        }
        case DRV_FM25CL_OPEN_WRITE_METADATA:
        {
            DrvFM25CL_writeMultiRegister(0, (const uint8_t *) _metadata, sizeof (_metadata));
            nextState = DRV_FM25CL_OPEN_READ_METADATA;
            dObj->openState = DRV_FM25CL_OPEN_WAIT_FOR_TRANSFER;
            break;
        }
        case DRV_FM25CL_OPEN_VERIFY_METADATA:
        {
            //            SYS_CONSOLE_PRINT("FRAMREAD: %x %x %x %x %x %x %x %x %x %x\r\n",
            //                    initDataRx[0], initDataRx[1], initDataRx[2], initDataRx[3], initDataRx[4],
            //                    initDataRx[5], initDataRx[6], initDataRx[7], initDataRx[8], initDataRx[9],
            //                    initDataRx[10]);

            bool valid = true;
            for (uint8_t i = 0; i < sizeof (_metadata); i++) {
                if (initDataRx[i] != _metadata[i]) {
                    valid = false;
                    break;
                }
            }

            if (valid) {
                dObj->openState = DRV_FM25CL_OPEN_DONE;
                dObj->status = SYS_STATUS_READY;
            } else {
                if (++numRetry > 1) {
                    dObj->openState = DRV_FM25CL_OPEN_DONE;
                    dObj->status = SYS_STATUS_ERROR;
                } else
                    dObj->openState = DRV_FM25CL_OPEN_WRITE_METADATA;
            }
            break;
        }
        case DRV_FM25CL_OPEN_WAIT_FOR_TRANSFER:
        {
            if (dObj->transferStatus != DRV_FM25CL_TRANSFER_COMPLETED) break;
            dObj->openState = nextState;
            break;
        }
        case DRV_FM25CL_OPEN_DONE:
        default: break;
    }

    return dObj->status;
}

DRV_FM25CL_TRANSFER_STATUS DrvFM25CL_GetTransferStatus() {
    return dObj->transferStatus;
}