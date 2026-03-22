#ifndef MODBUS_PHY_LAYER_H
#define	MODBUS_PHY_LAYER_H

#include <stdbool.h>
#include "definitions.h"

#define MODBUS_SERIAL_RX_BUFFER_SIZE  255 

#define MB_TRANSMIT_ENABLE(pin)             GPIO_PinSet(pin)
#define MB_RECEIVE_ENABLE(pin)              GPIO_PinClear(pin)

#ifdef	__cplusplus
extern "C" {
#endif

    typedef void (* MODBUS_UART_CALLBACK)(uintptr_t context);
    typedef bool (* MODBUS_UART_READ)(void* pReceiveData, size_t rxSize);
    typedef bool (* MODBUS_UART_WRITE)(void* pTransmitData, size_t txSize);
    typedef bool (* MODBUS_UART_READ_IS_BUSY)(void);
    typedef bool (* MODBUS_UART_WRITE_IS_BUSY)(void);
    typedef bool (* MODBUS_UART_READ_ABORT)(void);
    typedef void (* MODBUS_UART_WRITE_CALLBACK_REGISTER)(MODBUS_UART_CALLBACK callback, uintptr_t context);

    typedef struct {
        MODBUS_UART_READ read_t;
        MODBUS_UART_READ_IS_BUSY readIsBusy;
        MODBUS_UART_READ_ABORT readAbort;
        MODBUS_UART_WRITE write_t;
        MODBUS_UART_WRITE_IS_BUSY writeIsBusy;
        MODBUS_UART_WRITE_CALLBACK_REGISTER writeCallbackRegister;

        GPIO_PIN redePin;
    } MODBUS_UART_INTERFACE;

    typedef enum {
        FUNC_READ_COILS = 0x01, FUNC_READ_DISCRETE_INPUT = 0x02,
        FUNC_READ_HOLDING_REGISTERS = 0x03, FUNC_READ_INPUT_REGISTERS = 0x04,
        FUNC_WRITE_SINGLE_COIL = 0x05, FUNC_WRITE_SINGLE_REGISTER = 0x06,
        FUNC_READ_EXCEPTION_STATUS = 0x07, FUNC_DIAGNOSTICS = 0x08,
        FUNC_GET_COMM_EVENT_COUNTER = 0x0B, FUNC_GET_COMM_EVENT_LOG = 0x0C,
        FUNC_WRITE_MULTIPLE_COILS = 0x0F, FUNC_WRITE_MULTIPLE_REGISTERS = 0x10,
        FUNC_REPORT_SLAVE_ID = 0x11, FUNC_READ_FILE_RECORD = 0x14,
        FUNC_WRITE_FILE_RECORD = 0x15, FUNC_MASK_WRITE_REGISTER = 0x16,
        FUNC_READ_WRITE_MULTIPLE_REGISTERS = 0x17, FUNC_READ_FIFO_QUEUE = 0x18
    } MODBUS_FUNCTION;

    typedef enum {
        TASK_FUNC_READ_COILS = 0x01, TASK_FUNC_READ_DISCRETE_INPUT = 0x02,
        TASK_FUNC_READ_HOLDING_REGISTERS = 0x03, TASK_FUNC_READ_INPUT_REGISTERS = 0x04,
        TASK_FUNC_WRITE_SINGLE_COIL = 0x05, TASK_FUNC_WRITE_SINGLE_REGISTER = 0x06,
        TASK_FUNC_READ_EXCEPTION_STATUS = 0x07, TASK_FUNC_DIAGNOSTICS = 0x08,
        TASK_FUNC_GET_COMM_EVENT_COUNTER = 0x0B, TASK_FUNC_GET_COMM_EVENT_LOG = 0x0C,
        TASK_FUNC_WRITE_MULTIPLE_COILS = 0x0F, TASK_FUNC_WRITE_MULTIPLE_REGISTERS = 0x10,
        TASK_FUNC_REPORT_SLAVE_ID = 0x11, TASK_FUNC_READ_FILE_RECORD = 0x14,
        TASK_FUNC_WRITE_FILE_RECORD = 0x15, TASK_FUNC_MASK_WRITE_REGISTER = 0x16,
        TASK_FUNC_READ_WRITE_MULTIPLE_REGISTERS = 0x17, TASK_FUNC_READ_FIFO_QUEUE = 0x18,
        TASK_FUNC_RF_READ_HOLDING_REGISTERS = 0x20, TASK_FUNC_RF_READ_SINGLE_REGISTER = 0x21,
        TASK_RESPONSE, TASK_RESPONSE_WRITE,
        //TASK_FUNC_RF_WRITE_MULTI_REGISTER,TASK_FUNC_RF_WRITE_SINGLE_REGISTER
        TASK_FUNC_READ_COILS_RESPONSE, TASK_FUNC_READ_DISCRETE_INPUT_RESPONSE,
        TASK_FUNC_READ_HOLDING_REGISTERS_RESPONSE, TASK_FUNC_READ_INPUT_REGISTERS_RESPONSE,
        TASK_FUNC_WRITE_SINGLE_COIL_RESPONSE, TASK_FUNC_WRITE_SINGLE_REGISTER_RESPONSE,
        TASK_FUNC_READ_EXCEPTION_STATUS_RESPONSE, TASK_FUNC_DIAGNOSTICS_RESPONSE,
        TASK_FUNC_GET_COMM_EVENT_COUNTER_RESPONSE, TASK_FUNC_GET_COMM_EVENT_LOG_RESPONSE,
        TASK_FUNC_WRITE_MULTIPLE_COILS_RESPONSE, TASK_FUNC_WRITE_MULTIPLE_REGISTERS_RESPONSE,
        TASK_FUNC_WRITE_FILE_RECORD_RESPONSE, TASK_FUNC_MASK_WRITE_REGISTER_RESPONSE,
        TASK_FUNC_READ_WRITE_MULTIPLE_REGISTERS_RESPONSE, TASK_FUNC_READ_FIFO_QUEUE_RESPONSE,
        TASK_FUNC_REPORT_SLAVE_ID_RESPONSE, TASK_FUNC_READ_FILE_RECORD_RESPONSE,
        TASK_FUNC_RF_READ_HOLDING_REGISTERS_RESPONSE, TASK_FUNC_RF_READ_SINGLE_REGISTER_RESPONSE,
        TASK_FUNC_RF_WRITE_MULTI_REGISTER_RESPONSE, TASK_FUNC_RF_WRITE_SINGLE_REGISTER_RESPONSE, MODBUS_IDLE
    } MODBUS_STATES;

    typedef enum {
        ILLEGAL_FUNCTION = 1,
        ILLEGAL_DATA_ADDRESS = 2,
        ILLEGAL_DATA_VALUE = 3,
        SLAVE_DEVICE_FAILURE = 4,
        ACKNOWLEDGE = 5,
        SLAVE_DEVICE_BUSY = 6,
        MEMORY_PARITY_ERROR = 8,
        GATEWAY_PATH_UNAVAILABLE = 10,
        GATEWAY_TARGET_NO_RESPONSE = 11,
        TIMEOUT = 12
    } MODBUS_EXCEPTION;

    typedef struct {
        uint8_t address;
        uint8_t len; //number of bytes in the message received
        MODBUS_FUNCTION func; //the function of the message received
        MODBUS_EXCEPTION exception; //error recieved, if any
        uint8_t data[MODBUS_SERIAL_RX_BUFFER_SIZE]; //data of the message received
    } MODBUS_RX_PACKED;

    typedef union {
        uint8_t b[2];
        uint16_t d;
    } modbus_serial_crc;

    typedef struct {
        uint8_t slave_address;
        uint16_t start_address;
        uint16_t quantity;
        uint16_t output_address;
        uint16_t output_value;
        //        int on;
        uint16_t reg_address;
        uint16_t reg_value;
        uint8_t *values;
        uint16_t reference_address;
        uint16_t AND_mask;
        uint16_t OR_mask;
        uint16_t read_start;
        uint16_t read_quantity;
        uint16_t write_quantity;
        uint16_t byte_count;
        uint16_t write_start;
        uint16_t write_registers_value[200];
        uint8_t status_coils_8bit[MODBUS_SERIAL_RX_BUFFER_SIZE];
    } MODBUS_TRANSFER_DATA;

    typedef enum {
        MODBUS_GETADDY = 0,
        MODBUS_GETFUNC = 1,
        MODBUS_GETDATA = 2
    } modbus_serial_state;

    typedef struct {
        bool new;
        modbus_serial_state state;
        modbus_serial_crc crc;
    } MODBUS_SERIAL;

    void modbus_phy_send_start(unsigned char to, unsigned char func);
    void modbus_phy_send_stop(void);
    void modbus_phy_putc(unsigned char c);
    void modbus_phy_check_timeout(void);
    void modbus_phy_receive_on();
    void modbus_phy_calc_crc(unsigned char data);
    void modbus_phy_enable_timeout(bool enable);
    void modbus_phy_init(MODBUS_UART_INTERFACE interface);
    MODBUS_SERIAL * modbus_phy_get_serial_data();
    volatile MODBUS_RX_PACKED * modbus_phy_get_rx_packed();

#ifdef	__cplusplus
}
#endif

#endif	/* MODBUS_PHY_LAYER_H */

