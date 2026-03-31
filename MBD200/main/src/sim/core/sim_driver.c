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

    .resetPin = GPIO_PIN_RG14,
    .pwrPin = GPIO_PIN_RG12,
    .statusPin = GPIO_PIN_RD9,
    .netStatusPin = GPIO_PIN_RA15
};

static uint8_t _transferBuffer[SIM_TRANSFER_BUFF_SIZE] = {0};

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

/**
 * @brief Send raw data to SIM module
 * @param data Pointer to data buffer
 * @param length Number of bytes to send
 * @return Number of bytes actually sent
 */
size_t SIMDriver_Send(uint8_t* data, size_t length) {
    if (data == NULL || length == 0) return 0;
    return _simPlib.write(data, length);
}

/**
 * @brief Read available data from UART into a local buffer
 * @param buffer Pointer to destination buffer
 * @param maxLength Maximum bytes to read
 * @return Number of bytes read
 */
size_t SIMDriver_Receive(uint8_t* buffer, size_t maxLength) {
    size_t pending = _simPlib.readPendingBytes();
    if (pending == 0) return 0;

    size_t readSize = (pending > maxLength) ? maxLength : pending;
    return _simPlib.read(buffer, readSize);
}