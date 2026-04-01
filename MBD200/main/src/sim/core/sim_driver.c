#include "sim/sim_config.h"
#include "sim_driver.h"

static const SIM_UART_PLIB _simPlib = {
    .write = (SIM_UART_WRITE) UART1_Write,
    .writePendingBytes = (SIM_UART_WRITE_PENDING_BYTE) UART1_WriteCountGet,
    .writeFreeBytes = (SIM_UART_WRITE_FREE_BYTE) UART1_WriteFreeBufferCountGet,
    .transmitComplete = (SIM_UART_TRANSMIT_COMPLETE) UART1_TransmitComplete,
    .read = (SIM_UART_READ) UART1_Read,
    .readPendingBytes = (SIM_UART_READ_PENDING_BYTE) UART1_ReadCountGet,
    .readFreeBytes = (SIM_UART_READ_FREE_BYTE) UART1_ReadFreeBufferCountGet,
    .readThresholdSet = (SIM_UART_READ_THRESHOLD_SET) UART1_ReadThresholdSet,
    .readCallbackRegister = (SIM_UART_READ_CALLBACK_REGISTER) UART1_ReadCallbackRegister,
    .readNotifyEnable = (SIM_UART_READ_NOTIFY_ENABLE) UART1_ReadNotificationEnable,

    .resetPin = GPIO_PIN_RG14,
    .pwrPin = GPIO_PIN_RG12,
    .statusPin = GPIO_PIN_RD9,
    .netStatusPin = GPIO_PIN_RA15
};

static SIM_DRV_STATE _bufferState = 0;
static uint8_t _transferBuffer[SIM_TRANSFER_BUFF_SIZE] = {0};
static SIM_DRV_STATUS _drvStatus = 0;
static uint32_t _lastByteTick = 0;
static uint32_t _timeoutTick = 0;
static uint32_t _timeoutMs = 0;

static void _uartByteIncoming(UART_EVENT event, uintptr_t context) {
    /* Mark the timestamp of the latest incoming byte */
    _lastByteTick = TICK_NOW();
}

void SIMDriver_Initialize(void) {
    _simPlib.readCallbackRegister(, _uartByteIncoming, (uintptr_t) NULL);
    _simPlib.readThresholdSet(1);
    _simPlib.readNotifyEnable(true, true);
}

void SIMDriver_Task(void) {
    size_t available = _simPlib.readPendingBytes();
    if (available > 0) {
        if (IS_TIMEOUT(_lastByteTick, SIM_TRANSFER_GAP_TIME)) {

            _drvStatus = SIM_DRV_STATUS_RECV_RESP;
            return;
        }
    }

    if (_drvStatus == SIM_DRV_STATUS_IDLE) return;
    if (IS_TIMEOUT(_timeoutTick, _timeoutMs))
        _drvStatus = SIM_DRV_STATUS_TIMEOUT;

}

/**
 * @brief Get pointer to the shared transfer buffer
 * @param state State to lock the buffer (TX or RX)
 * @return Pointer to buffer, or NULL if buffer is locked by another process
 */
uint8_t* SIMDriver_GetBuffer(SIM_DRV_STATE state) {
    if (_bufferState == SIM_DRV_IDLE) {
        memset(_transferBuffer, 0, SIM_TRANSFER_BUFF_SIZE);

        if (state == SIM_DRV_TX_BUSY)
            _bufferState = state;
        else if (state == SIM_DRV_RX_BUSY) {
            /* Ensure we only read if a response is actually ready */
            if (_drvStatus == SIM_DRV_STATUS_RECV_RESP) {
                size_t available = _simPlib.readPendingBytes();
                size_t toRead = (available > SIM_TRANSFER_BUFF_SIZE) ? SIM_TRANSFER_BUFF_SIZE : available;
                _simPlib.read(_transferBuffer, toRead);
                _drvStatus = SIM_DRV_STATUS_IDLE;
            } else
                return NULL;
        }

        return _transferBuffer;
    }
    return NULL; // Buffer is currently in use
}

/**
 * @brief Send command and start tracking timeout
 * @param size Data size in buffer
 * @param timeout Expected response time in ms
 */
bool SIMDriver_Execute(size_t txSize, uint32_t timeout) {
    if (_bufferState != SIM_DRV_TX_BUSY) return false;
    if (_simPlib.writePendingBytes() > 0 ||
            _simPlib.writeFreeBytes() < txSize ||
            _simPlib.transmitComplete == false)
        return false;

    if (_simPlib.write(_transferBuffer, txSize) == txSize) {
        _timeoutTick = TICK_NOW();
        _timeoutMs = timeout;
        _bufferState = SIM_DRV_IDLE;
        _drvStatus = SIM_DRV_STATUS_WAIT_RESP;
        return true;
    }
    return false;
}

/**
 * @brief Get current transaction status. 
 */
SIM_DRV_STATUS SIMDriver_GetStatus(void) {
    return _drvStatus;
}

/**
 * @brief Hard reset the SIM module using Reset Pin
 */
void SIMDriver_Reset(void) {

}

/**
 * @brief Power on/off the module using Power Key Pin
 */
void SIMDriver_SetPower(bool state) {

}