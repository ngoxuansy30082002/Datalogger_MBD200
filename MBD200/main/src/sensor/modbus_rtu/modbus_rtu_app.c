#include "modbus_rtu_app.h"

static bool _finish = false;
static MODBUS_STATES _state;
static MODBUS_TRANSFER_DATA _transferData;

void modbus_app_init(MODBUS_UART_INTERFACE interface) {
    modbus_phy_init(interface);
    _state = MODBUS_IDLE;
}

bool modbus_app_kbhit() {
    modbus_phy_check_timeout();
    MODBUS_SERIAL * serial = modbus_phy_get_serial_data();
    volatile MODBUS_RX_PACKED * packed = modbus_phy_get_rx_packed();

    if (!serial->new) {
        return false;
    } else if (packed->func & 0x80) //did we receive an error?
    {
        packed->exception = packed->data[0]; //if so grab the error and return true
        packed->len = 1;
    }
    _finish = true;
    serial->new = false;

    return true;
}

MODBUS_TRANSFER_DATA * modbus_app_get_transfer_pointer() {
    return &_transferData;
};

void modbus_app_push_request(MODBUS_STATES state) {
    _finish = false;
    _state = state;
}

bool modbus_app_transfer_done() {
    if (_state == MODBUS_IDLE)
        return _finish;
    return false;
}

// use modbus master. master recevier.

/*MODBUS Master Functions*/

/********************************************************************
The following structs are used for read/write_sub_request.  These
functions take in one of these structs.
Please refer to the MODBUS protocol specification if you do not
understand the members of the structure.
 ********************************************************************/

/********************************************************************
The following functions are defined in the MODBUS protocol.  Please
refer to http://www.modbus.org for the purpose of each of these.
All functions take the slaves address as their first parameter.
Each function returns the exception code received from the response.
The function will return 0 if there were no errors in transmission.
 ********************************************************************/

/*
read_coils
Input:     int8       address            Slave Address
           int16      start_address      Address to start reading from
           int16      quantity           Amount of addresses to read
Output:    exception                     0 if no error, else the exception
 

 */
MODBUS_EXCEPTION modbus_app_read_coils(uint8_t address, uint16_t start_address, uint16_t quantity) {
    modbus_phy_send_start(address, FUNC_READ_COILS);

    modbus_phy_putc((uint8_t) (start_address >> 8));
    modbus_phy_putc((uint8_t) (start_address));

    modbus_phy_putc((uint8_t) (quantity >> 8));
    modbus_phy_putc((uint8_t) (quantity));

    modbus_phy_send_stop();

    return 0;
}

/*
read_discrete_input
Input:     int8       address            Slave Address
           int16      start_address      Address to start reading from
           int16      quantity           Amount of addresses to read
Output:    exception                     0 if no error, else the exception
 */
MODBUS_EXCEPTION modbus_app_read_discrete_input(uint8_t address, uint16_t start_address, uint16_t quantity) {
    modbus_phy_send_start(address, FUNC_READ_DISCRETE_INPUT);

    modbus_phy_putc((uint8_t) (start_address >> 8));
    modbus_phy_putc((uint8_t) (start_address));

    modbus_phy_putc((uint8_t) (quantity >> 8));
    modbus_phy_putc((uint8_t) (quantity));

    modbus_phy_send_stop();

    return 0;
}

/*
read_holding_registers
Input:     int8       address            Slave Address
           int16      start_address      Address to start reading from
           int16      quantity           Amount of addresses to read
Output:    exception                     0 if no error, else the exception
 */
MODBUS_EXCEPTION modbus_app_read_holding_registers(uint8_t address, uint16_t start_address, uint16_t quantity) {

    modbus_phy_send_start(address, FUNC_READ_HOLDING_REGISTERS);
    modbus_phy_putc((uint8_t) (start_address >> 8));
    modbus_phy_putc((uint8_t) (start_address));

    modbus_phy_putc((uint8_t) (quantity >> 8));
    modbus_phy_putc((uint8_t) (quantity));

    modbus_phy_send_stop();

    return 0;
}

/*
read_input_registers
Input:     int8       address            Slave Address
           int16      start_address      Address to start reading from
           int16      quantity           Amount of addresses to read
Output:    exception                     0 if no error, else the exception
 */
MODBUS_EXCEPTION modbus_app_read_input_registers(uint8_t address, uint16_t start_address, uint16_t quantity) {
    modbus_phy_send_start(address, FUNC_READ_INPUT_REGISTERS);

    modbus_phy_putc((uint8_t) (start_address >> 8));
    modbus_phy_putc((uint8_t) (start_address));

    modbus_phy_putc((uint8_t) (quantity >> 8));
    modbus_phy_putc((uint8_t) (quantity));

    modbus_phy_send_stop();

    return 0;
}

/*
write_single_coil
Input:     int8       address            Slave Address
           int16      output_address     Address to write into
           int1       on                 true for on, false for off
Output:    exception                     0 if no error, else the exception
 */
MODBUS_EXCEPTION modbus_app_write_single_coil(uint8_t address, uint16_t output_address, uint16_t output_value) {
    modbus_phy_send_start(address, FUNC_WRITE_SINGLE_COIL);

    modbus_phy_putc((uint8_t) (output_address >> 8));
    modbus_phy_putc((uint8_t) (output_address));

    modbus_phy_putc((uint8_t) (output_value >> 8));
    modbus_phy_putc((uint8_t) (output_value));

    modbus_phy_send_stop();

    return 0;
}

/*
write_single_register
Input:     int8       address            Slave Address
           int16      reg_address        Address to write into
           int16      reg_value          Value to write
Output:    exception                     0 if no error, else the exception
 */
MODBUS_EXCEPTION modbus_app_write_single_register(uint8_t address, uint16_t reg_address, uint16_t reg_value) {
    modbus_phy_send_start(address, FUNC_WRITE_SINGLE_REGISTER);

    modbus_phy_putc((uint8_t) (reg_address >> 8));
    modbus_phy_putc((uint8_t) (reg_address));

    modbus_phy_putc((uint8_t) (reg_value >> 8));
    modbus_phy_putc((uint8_t) (reg_value));

    modbus_phy_send_stop();

    return 0;
}

/*
read_exception_status
Input:     int8       address            Slave Address
Output:    exception                     0 if no error, else the exception
 */
MODBUS_EXCEPTION modbus_app_read_exception_status(uint8_t address) {
    modbus_phy_send_start(address, FUNC_READ_EXCEPTION_STATUS);
    modbus_phy_send_stop();

    return 0;
}

/*
diagnostics
Input:     int8       address            Slave Address
           int16      sub_func           Subfunction to send
           int16      data               Data to send, changes based on subfunction
Output:    exception                     0 if no error, else the exception
 */
MODBUS_EXCEPTION modbus_app_diagnostics(uint8_t address, uint16_t sub_func, uint16_t data) {
    modbus_phy_send_start(address, FUNC_DIAGNOSTICS);

    modbus_phy_putc(Helpers_Make8(sub_func, 1));
    modbus_phy_putc(Helpers_Make8(sub_func, 0));

    modbus_phy_putc(Helpers_Make8(data, 1));
    modbus_phy_putc(Helpers_Make8(data, 0));

    modbus_phy_send_stop();

    return 0;
}

/*
get_comm_event_couter
Input:     int8       address            Slave Address
Output:    exception                     0 if no error, else the exception
 */
MODBUS_EXCEPTION modbus_app_get_comm_event_counter(uint8_t address) {
    modbus_phy_send_start(address, FUNC_GET_COMM_EVENT_COUNTER);
    modbus_phy_send_stop();

    return 0;
}

/*
get_comm_event_log
Input:     int8       address            Slave Address
Output:    exception                     0 if no error, else the exception
 */
MODBUS_EXCEPTION modbus_app_get_comm_event_log(uint8_t address) {
    modbus_phy_send_start(address, FUNC_GET_COMM_EVENT_LOG);
    modbus_phy_send_stop();

    return 0;
}

/*
write_multiple_coils
Input:     int8       address            Slave Address
           int16      start_address      Address to start at
           int16      quantity           Amount of coils to write to
           int1*      values             A pointer to an array holding the values to write
Output:    exception                     0 if no error, else the exception
 */
MODBUS_EXCEPTION modbus_app_write_multiple_coils(uint8_t address, uint16_t start_address, uint16_t quantity,
        uint8_t * values) {
    uint8_t i, count;

    count = (uint8_t) ((quantity / 8));

    if (quantity % 8)
        count++;

    modbus_phy_send_start(address, FUNC_WRITE_MULTIPLE_COILS);

    modbus_phy_putc((uint8_t) (start_address >> 8));
    modbus_phy_putc((uint8_t) (start_address));

    modbus_phy_putc((uint8_t) (quantity >> 8));
    modbus_phy_putc((uint8_t) (quantity));

    modbus_phy_putc(count);

    for (i = 0; i < count; i++) {
        modbus_phy_putc(*values);
        values++;
    }

    modbus_phy_send_stop();

    return 0;
}

/*
write_multiple_registers
Input:     int8       address            Slave Address
           int16      start_address      Address to start at
           int16      quantity           Amount of coils to write to
           int16*     values             A pointer to an array holding the data to write
Output:    exception                     0 if no error, else the exception
 */
MODBUS_EXCEPTION modbus_app_write_multiple_registers(uint8_t address, uint16_t start_address, uint16_t quantity,
        uint8_t * values) {
    uint8_t i, count;

    count = quantity * 2;

    modbus_phy_send_start(address, FUNC_WRITE_MULTIPLE_REGISTERS);

    modbus_phy_putc((uint8_t) (start_address >> 8));
    modbus_phy_putc((uint8_t) (start_address));

    modbus_phy_putc((uint8_t) (quantity >> 8));
    modbus_phy_putc((uint8_t) (quantity));

    modbus_phy_putc(count);

    for (i = 0; i < count; i++) {
        modbus_phy_putc(*values);
        values++;
    }

    modbus_phy_send_stop();

    return 0;
}

/*
report_slave_id
Input:     int8       address            Slave Address
Output:    exception                     0 if no error, else the exception
 */
MODBUS_EXCEPTION modbus_app_report_slave_id(uint8_t address) {
    modbus_phy_send_start(address, FUNC_REPORT_SLAVE_ID);
    modbus_phy_send_stop();

    return 0;
}

/*
read_file_record
Input:     int8                address            Slave Address
           int8                byte_count         Number of bytes to read
           read_sub_request*   request            Structure holding record information
Output:    exception                              0 if no error, else the exception
 */
MODBUS_EXCEPTION modbus_app_read_file_record(uint8_t address, uint8_t byte_count,
        modbus_read_sub_request * request) {
    uint8_t i;

    modbus_phy_send_start(address, FUNC_READ_FILE_RECORD);

    modbus_phy_putc(byte_count);

    for (i = 0; i < (byte_count / 7); i += 7) {
        modbus_phy_putc(request->reference_type);
        modbus_phy_putc(Helpers_Make8(request->file_number, 1));
        modbus_phy_putc(Helpers_Make8(request->file_number, 0));
        modbus_phy_putc(Helpers_Make8(request->record_number, 1));
        modbus_phy_putc(Helpers_Make8(request->record_number, 0));
        modbus_phy_putc(Helpers_Make8(request->record_length, 1));
        modbus_phy_putc(Helpers_Make8(request->record_length, 0));
        request++;
    }

    modbus_phy_send_stop();

    return 0;
}

/*
write_file_record
Input:     int8                address            Slave Address
           int8                byte_count         Number of bytes to read
           read_sub_request*   request            Structure holding record/data information
Output:    exception                              0 if no error, else the exception
 */
MODBUS_EXCEPTION modbus_app_write_file_record(uint8_t address, uint8_t byte_count,
        modbus_write_sub_request * request) {
    uint8_t i, j = 0;

    modbus_phy_send_start(address, FUNC_WRITE_FILE_RECORD);

    modbus_phy_putc(byte_count);

    for (i = 0; i < byte_count; i += (7 + (j * 2))) {
        modbus_phy_putc(request->reference_type);
        modbus_phy_putc(Helpers_Make8(request->file_number, 1));
        modbus_phy_putc(Helpers_Make8(request->file_number, 0));
        modbus_phy_putc(Helpers_Make8(request->record_number, 1));
        modbus_phy_putc(Helpers_Make8(request->record_number, 0));
        modbus_phy_putc(Helpers_Make8(request->record_length, 1));
        modbus_phy_putc(Helpers_Make8(request->record_length, 0));

        for (j = 0; (j < request->record_length) &&
                (j < MODBUS_SERIAL_RX_BUFFER_SIZE - 8);
                j += 2) {
            modbus_phy_putc(Helpers_Make8(request->data[j], 1));
            modbus_phy_putc(Helpers_Make8(request->data[j], 0));
        }
        request++;
    }

    modbus_phy_send_stop();

    return 0;
}

/*
mask_write_register
Input:     int8       address            Slave Address
           int16      reference_address  Address to mask
           int16      AND_mask           A mask to AND with the data at reference_address
           int16      OR_mask            A mask to OR with the data at reference_address
Output:    exception                              0 if no error, else the exception
 */
MODBUS_EXCEPTION modbus_app_mask_write_register(uint8_t address, uint16_t reference_address,
        uint16_t AND_mask, uint16_t OR_mask) {
    modbus_phy_send_start(address, FUNC_MASK_WRITE_REGISTER);

    modbus_phy_putc(Helpers_Make8(reference_address, 1));
    modbus_phy_putc(Helpers_Make8(reference_address, 0));

    modbus_phy_putc(Helpers_Make8(AND_mask, 1));
    modbus_phy_putc(Helpers_Make8(AND_mask, 0));

    modbus_phy_putc(Helpers_Make8(OR_mask, 1));
    modbus_phy_putc(Helpers_Make8(OR_mask, 0));

    modbus_phy_send_stop();

    return 0;
}

/*
read_write_multiple_registers
Input:     int8       address                Slave Address
           int16      read_start             Address to start reading
           int16      read_quantity          Amount of registers to read
           int16      write_start            Address to start writing
           int16      write_quantity         Amount of registers to write
           int16*     write_registers_value  Pointer to an aray us to write
Output:    exception                         0 if no error, else the exception
 */
MODBUS_EXCEPTION modbus_app_read_write_multiple_registers(uint8_t address, uint16_t read_start,
        uint16_t read_quantity, uint16_t write_start,
        uint16_t write_quantity,
        uint16_t * write_registers_value) {
    uint8_t i;

    modbus_phy_send_start(address, FUNC_READ_WRITE_MULTIPLE_REGISTERS);

    modbus_phy_putc(Helpers_Make8(read_start, 1));
    modbus_phy_putc(Helpers_Make8(read_start, 0));

    modbus_phy_putc(Helpers_Make8(read_quantity, 1));
    modbus_phy_putc(Helpers_Make8(read_quantity, 0));

    modbus_phy_putc(Helpers_Make8(write_start, 1));
    modbus_phy_putc(Helpers_Make8(write_start, 0));

    modbus_phy_putc(Helpers_Make8(write_quantity, 1));
    modbus_phy_putc(Helpers_Make8(write_quantity, 0));

    modbus_phy_putc((uint8_t) (2 * write_quantity));

    for (i = 0; i < write_quantity; i += 2) {
        modbus_phy_putc(Helpers_Make8(write_registers_value[i], 1));
        modbus_phy_putc(Helpers_Make8(write_registers_value[i + 1], 0));
    }

    modbus_phy_send_stop();

    return 0;
}

/*
read_FIFO_queue
Input:     int8       address           Slave Address
           int16      FIFO_address      FIFO address
Output:    exception                    0 if no error, else the exception
 */
MODBUS_EXCEPTION modbus_app_read_FIFO_queue(uint8_t address, uint16_t FIFO_address) {
    modbus_phy_send_start(address, FUNC_READ_FIFO_QUEUE);

    modbus_phy_putc(Helpers_Make8(FIFO_address, 1));
    modbus_phy_putc(Helpers_Make8(FIFO_address, 0));

    modbus_phy_send_stop();

    return 0;
}

static uint64_t ExtractModbusData(const volatile uint8_t *data, uint8_t byteCount, BYTE_ORDER_TYPE order) {
    if (byteCount == 1) {
        return data[1];
    } else if (byteCount == 2) {
        if (order == BIG_ENDIAN_ABCD || order == LITTLE_ENDIAN_CDAB)
            return ((uint16_t) data[1] << 8) | data[2];
        else
            return ((uint16_t) data[2] << 8) | data[1];
    } else if (byteCount == 4) {
        switch (order) {
            case BIG_ENDIAN_ABCD:
                return ((uint32_t) data[1] << 24) | ((uint32_t) data[2] << 16) | ((uint32_t) data[3] << 8) | data[4];
            case LITTLE_ENDIAN_CDAB:
                return ((uint32_t) data[3] << 24) | ((uint32_t) data[4] << 16) | ((uint32_t) data[1] << 8) | data[2];
            case BIG_ENDIAN_SWAP_BADC:
                return ((uint32_t) data[2] << 24) | ((uint32_t) data[1] << 16) | ((uint32_t) data[4] << 8) | data[3];
            case LITTLE_ENDIAN_SWAP_DCBA:
                return ((uint32_t) data[4] << 24) | ((uint32_t) data[3] << 16) | ((uint32_t) data[2] << 8) | data[1];
        }
    } else if (byteCount == 6) {
        switch (order) {
            case BIG_ENDIAN_ABCD:
                return ((uint64_t) data[1] << 40) | ((uint64_t) data[2] << 32) | ((uint64_t) data[3] << 24) | ((uint64_t) data[4] << 16) | ((uint64_t) data[5] << 8) | data[6];
            case LITTLE_ENDIAN_CDAB:
                return ((uint64_t) data[5] << 40) | ((uint64_t) data[6] << 32) | ((uint64_t) data[3] << 24) | ((uint64_t) data[4] << 16) | ((uint64_t) data[1] << 8) | data[2];
            case BIG_ENDIAN_SWAP_BADC:
                return ((uint64_t) data[2] << 40) | ((uint64_t) data[1] << 32) | ((uint64_t) data[4] << 24) | ((uint64_t) data[3] << 16) | ((uint64_t) data[6] << 8) | data[5];
            case LITTLE_ENDIAN_SWAP_DCBA:
                return ((uint64_t) data[6] << 40) | ((uint64_t) data[5] << 32) | ((uint64_t) data[4] << 24) | ((uint64_t) data[3] << 16) | ((uint64_t) data[2] << 8) | data[1];
        }
    } else if (byteCount == 8) {
        switch (order) {
            case BIG_ENDIAN_ABCD:
                return ((uint64_t) data[1] << 56) | ((uint64_t) data[2] << 48) | ((uint64_t) data[3] << 40) | ((uint64_t) data[4] << 32) | ((uint64_t) data[5] << 24) | ((uint64_t) data[6] << 16) | ((uint64_t) data[7] << 8) | data[8];
            case LITTLE_ENDIAN_CDAB:
                return ((uint64_t) data[7] << 56) | ((uint64_t) data[8] << 48) | ((uint64_t) data[5] << 40) | ((uint64_t) data[6] << 32) | ((uint64_t) data[3] << 24) | ((uint64_t) data[4] << 16) | ((uint64_t) data[1] << 8) | data[2];
            case BIG_ENDIAN_SWAP_BADC:
                return ((uint64_t) data[2] << 56) | ((uint64_t) data[1] << 48) | ((uint64_t) data[4] << 40) | ((uint64_t) data[3] << 32) | ((uint64_t) data[6] << 24) | ((uint64_t) data[5] << 16) | ((uint64_t) data[8] << 8) | data[7];
            case LITTLE_ENDIAN_SWAP_DCBA:
                return ((uint64_t) data[8] << 56) | ((uint64_t) data[7] << 48) | ((uint64_t) data[6] << 40) | ((uint64_t) data[5] << 32) | ((uint64_t) data[4] << 24) | ((uint64_t) data[3] << 16) | ((uint64_t) data[2] << 8) | data[1];
        }
    }
    return 0; // Fallback
}

void modbus_app_task() {
    switch (_state) {
        case MODBUS_IDLE:
            break;

        case TASK_FUNC_READ_COILS:
            modbus_app_read_coils(_transferData.slave_address, _transferData.start_address, _transferData.quantity);
            _state = TASK_RESPONSE;
            modbus_phy_enable_timeout(true);
            break;

        case TASK_FUNC_READ_DISCRETE_INPUT:
            modbus_app_read_discrete_input(_transferData.slave_address, _transferData.start_address, _transferData.quantity);
            _state = TASK_RESPONSE;
            modbus_phy_enable_timeout(true);
            break;

        case TASK_FUNC_READ_HOLDING_REGISTERS:
            modbus_app_read_holding_registers(_transferData.slave_address, _transferData.start_address, _transferData.quantity);
            _state = TASK_RESPONSE;
            modbus_phy_enable_timeout(true);
            break;

        case TASK_FUNC_READ_INPUT_REGISTERS:
            modbus_app_read_input_registers(_transferData.slave_address, _transferData.start_address, _transferData.quantity);
            _state = TASK_RESPONSE;
            modbus_phy_enable_timeout(true);
            break;

        case TASK_FUNC_WRITE_SINGLE_COIL:
            modbus_app_write_single_coil(_transferData.slave_address, _transferData.output_address, _transferData.output_value);
            _state = TASK_RESPONSE_WRITE;
            modbus_phy_enable_timeout(true);
            break;

        case TASK_FUNC_WRITE_SINGLE_REGISTER:
            modbus_app_write_single_register(_transferData.slave_address, _transferData.reg_address, _transferData.reg_value);
            _state = TASK_RESPONSE_WRITE;
            modbus_phy_enable_timeout(true);
            break;

        case TASK_FUNC_WRITE_MULTIPLE_COILS:
            modbus_app_write_multiple_coils(_transferData.slave_address, _transferData.start_address, _transferData.quantity,
                    _transferData.values);
            _state = TASK_RESPONSE_WRITE;
            modbus_phy_enable_timeout(true);
            break;

        case TASK_FUNC_WRITE_MULTIPLE_REGISTERS:
            modbus_app_write_multiple_registers(_transferData.slave_address, _transferData.start_address, _transferData.quantity,
                    _transferData.values);
            _state = TASK_RESPONSE_WRITE;
            modbus_phy_enable_timeout(true);
            break;

        case TASK_RESPONSE:
            if (modbus_app_kbhit()) {

                MBRTU_RAW_VALUE raw = {0};
                const MODBUSRTU_TAG_ENTRY *tagCfg = MbRtuMaster_GetCurrentTagConfig();
                volatile MODBUS_RX_PACKED *modbusRx = modbus_phy_get_rx_packed();
                bool isError = false;

                //                for (uint8_t i = 0; i < modbusRx->len; i++) {
                //                    LOG_DEBUG("databyte %u-th: %02X", i, modbusRx->data[i]);
                //                }

                if (_transferData.slave_address == modbusRx->address) {
                    uint8_t func = modbusRx->func;

                    if (func != FUNC_READ_COILS && func != FUNC_READ_DISCRETE_INPUT &&
                            func != FUNC_READ_HOLDING_REGISTERS && func != FUNC_READ_INPUT_REGISTERS) {
                        MbRtuMaster_SetCurrentTagData(raw, false);
                        _state = MODBUS_IDLE;
                        break;
                    }

                    uint8_t byteCount = modbusRx->data[0];

                    if (byteCount != 1 && byteCount != 2 && byteCount != 4 && byteCount != 6 && byteCount != 8) {
                        isError = true;
                    } else {
                        uint64_t extractedVal = ExtractModbusData(modbusRx->data, byteCount, tagCfg->byteOder);

                        switch (tagCfg->rawDataType) {
                            case DATA_UINT:
                                raw.reuint = extractedVal;
                                break;

                            case DATA_INT:
                                if (byteCount == 1) raw.reint = (int8_t) extractedVal;
                                else if (byteCount == 2) raw.reint = (int16_t) extractedVal;
                                else if (byteCount == 4) raw.reint = (int32_t) extractedVal;
                                else raw.reint = (int64_t) extractedVal;
                                break;

                            case DATA_FLOAT:
                                raw.refloat.c = (uint32_t) extractedVal;
                                break;

                            default:
                                isError = true;
                                break;
                        }
                    }

                    MbRtuMaster_SetCurrentTagData(raw, !isError);
                }

                modbusRx->len = 0;
                _state = MODBUS_IDLE;
                break;
            }

        case TASK_RESPONSE_WRITE:
            if (modbus_app_kbhit()) {
                //                SYS_CONSOLE_PRINT("bytecount: %x \r\n", modbus_rx.data[0]);
                //                SYS_CONSOLE_PRINT("data1: %x \r\n", modbus_rx.data[1]);
                //                SYS_CONSOLE_PRINT("data2: %x \r\n", modbus_rx.data[2]);
                //                SYS_CONSOLE_PRINT("data3: %x \r\n", modbus_rx.data[3]);
                //                SYS_CONSOLE_PRINT("data4: %x \r\n", modbus_rx.data[4]);
                _state = MODBUS_IDLE;
            }
            break;
        default:
            break;
    }
};


/********************************************************************
The following slave functions are defined in the MODBUS protocol.
Please refer to http://www.modbus.org for the purpose of each of
these.  All functions take the slaves address as their first
parameter.
 ********************************************************************/

/*
read_coils_rsp
Input:     int8       address            Slave Address
           int8       byte_count         Number of bytes being sent
           int8*      coil_data          Pointer to an array of data to send
Output:    void
 */
void modbus_app_read_coils_rsp(uint8_t address, uint8_t byte_count, uint8_t * coil_data) {
    uint8_t i;
    modbus_phy_send_start(address, FUNC_READ_COILS);

    modbus_phy_putc(byte_count);

    for (i = 0; i < byte_count; ++i) {

        modbus_phy_putc(*coil_data);
        coil_data++;
    }

    modbus_phy_send_stop();
}

/*
read_discrete_input_rsp
Input:     int8       address            Slave Address
           int8       byte_count         Number of bytes being sent
           int8*      input_data         Pointer to an array of data to send
Output:    void
 */
void modbus_app_read_discrete_input_rsp(uint8_t address, uint8_t byte_count,
        uint8_t * input_data) {
    uint8_t i;

    modbus_phy_send_start(address, FUNC_READ_DISCRETE_INPUT);

    modbus_phy_putc(byte_count);

    for (i = 0; i < byte_count; ++i) {

        modbus_phy_putc(*input_data);
        input_data++;
    }

    modbus_phy_send_stop();
}

/*
read_holding_registers_rsp
Input:     int8       address            Slave Address
           int8       byte_count         Number of bytes being sent
           int8*      reg_data           Pointer to an array of data to send
Output:    void
 */
void modbus_app_read_holding_registers_rsp(uint8_t address, uint8_t byte_count,
        uint16_t * reg_data) {
    uint8_t i;

    modbus_phy_send_start(address, FUNC_READ_HOLDING_REGISTERS);

    modbus_phy_putc(byte_count);

    for (i = 0; i < byte_count; i += 2) {

        modbus_phy_putc((uint8_t) (*reg_data >> 8));
        modbus_phy_putc((uint8_t) (*reg_data));
        reg_data++;
    }

    modbus_phy_send_stop();
}

/*
read_input_registers_rsp
Input:     int8       address            Slave Address
           int8       byte_count         Number of bytes being sent
           int8*      input_data         Pointer to an array of data to send
Output:    void
 */
void modbus_app_read_input_registers_rsp(uint8_t address, uint8_t byte_count,
        uint16_t * input_data) {
    uint8_t i;

    modbus_phy_send_start(address, FUNC_READ_INPUT_REGISTERS);

    modbus_phy_putc(byte_count);

    for (i = 0; i < byte_count; i += 2) {

        modbus_phy_putc((uint8_t) (*input_data >> 8));
        modbus_phy_putc((uint8_t) (*input_data));
        input_data++;
    }

    modbus_phy_send_stop();
}

/*
write_single_coil_rsp
Input:     int8       address            Slave Address
           int16      output_address     Echo of output address received
           int16      output_value       Echo of output value received
Output:    void
 */
void modbus_app_write_single_coil_rsp(uint8_t address, uint16_t output_address,
        uint16_t output_value) {

    modbus_phy_send_start(address, FUNC_WRITE_SINGLE_COIL);

    modbus_phy_putc((uint8_t) (output_address >> 8));
    modbus_phy_putc((uint8_t) (output_address));

    modbus_phy_putc((uint8_t) (output_value >> 8));
    modbus_phy_putc((uint8_t) (output_value));

    modbus_phy_send_stop();
}

/*
write_single_register_rsp
Input:     int8       address            Slave Address
           int16      reg_address        Echo of register address received
           int16      reg_value          Echo of register value received
Output:    void
 */
void modbus_app_write_single_register_rsp(uint8_t address, uint16_t reg_address,
        uint16_t reg_value) {

    modbus_phy_send_start(address, FUNC_WRITE_SINGLE_REGISTER);

    modbus_phy_putc((uint8_t) (reg_address >> 8));
    modbus_phy_putc((uint8_t) (reg_address));

    modbus_phy_putc((uint8_t) (reg_value >> 8));
    modbus_phy_putc((uint8_t) (reg_value));

    modbus_phy_send_stop();
}

/*
read_exception_status_rsp
Input:     int8       address            Slave Address
Output:    void
 */
void modbus_app_read_exception_status_rsp(uint8_t address, uint8_t data) {

    modbus_phy_send_start(address, FUNC_READ_EXCEPTION_STATUS);
    modbus_phy_send_stop();
}

/*
diagnostics_rsp
Input:     int8       address            Slave Address
           int16      sub_func           Echo of sub function received
           int16      data               Echo of data received
Output:    void
 */
void modbus_app_diagnostics_rsp(uint8_t address, uint16_t sub_func, uint16_t data) {

    modbus_phy_send_start(address, FUNC_DIAGNOSTICS);

    modbus_phy_putc((uint8_t) (sub_func >> 8));
    modbus_phy_putc((uint8_t) (sub_func));

    modbus_phy_putc((uint8_t) (data >> 8));
    modbus_phy_putc((uint8_t) (data));

    modbus_phy_send_stop();
}

/*
get_comm_event_counter_rsp
Input:     int8       address            Slave Address
           int16      status             Status, refer to MODBUS documentation
           int16      event_count        Count of events
Output:    void
 */
void modbus_app_get_comm_event_counter_rsp(uint8_t address, uint16_t status,
        uint16_t event_count) {

    modbus_phy_send_start(address, FUNC_GET_COMM_EVENT_COUNTER);

    modbus_phy_putc((uint8_t) (status >> 8));
    modbus_phy_putc((uint8_t) (status));

    modbus_phy_putc((uint8_t) (event_count >> 8));
    modbus_phy_putc((uint8_t) (event_count));

    modbus_phy_send_stop();
}

/*
get_comm_event_counter_rsp
Input:     int8       address            Slave Address
           int16      status             Status, refer to MODBUS documentation
           int16      event_count        Count of events
           int16      message_count      Count of messages
           int8*      events             Pointer to event data
           int8       events_len         Length of event data in bytes
Output:    void
 */
void modbus_app_get_comm_event_log_rsp(uint8_t address, uint16_t status,
        uint16_t event_count, uint16_t message_count,
        uint8_t *events, uint8_t events_len) {
    uint8_t i;

    modbus_phy_send_start(address, FUNC_GET_COMM_EVENT_LOG);

    modbus_phy_putc(events_len + 6);

    modbus_phy_putc((uint8_t) (status >> 8));
    modbus_phy_putc((uint8_t) (status));

    modbus_phy_putc((uint8_t) (event_count >> 8));
    modbus_phy_putc((uint8_t) (event_count));

    modbus_phy_putc((uint8_t) (message_count >> 8));
    modbus_phy_putc((uint8_t) (message_count));

    for (i = 0; i < events_len; ++i) {

        modbus_phy_putc(*events);
        events++;
    }

    modbus_phy_send_stop();
}

/*
write_multiple_coils_rsp
Input:     int8       address            Slave Address
           int16      start_address      Echo of address to start at
           int16      quantity           Echo of amount of coils written to
Output:    void
 */
void modbus_app_write_multiple_coils_rsp(uint8_t address, uint16_t start_address,
        uint16_t quantity) {

    modbus_phy_send_start(address, FUNC_WRITE_MULTIPLE_COILS);

    modbus_phy_putc((uint8_t) (start_address >> 8));
    modbus_phy_putc((uint8_t) (start_address));

    modbus_phy_putc((uint8_t) (quantity >> 8));
    modbus_phy_putc((uint8_t) (quantity));

    modbus_phy_send_stop();
}

/*
write_multiple_registers_rsp
Input:     int8       address            Slave Address
           int16      start_address      Echo of address to start at
           int16      quantity           Echo of amount of registers written to
Output:    void
 */
void modbus_app_write_multiple_registers_rsp(uint8_t address, uint16_t start_address,
        uint16_t quantity) {

    modbus_phy_send_start(address, FUNC_WRITE_MULTIPLE_REGISTERS);

    modbus_phy_putc((uint8_t) (start_address >> 8));
    modbus_phy_putc((uint8_t) (start_address));

    modbus_phy_putc((uint8_t) (quantity >> 8));
    modbus_phy_putc((uint8_t) (quantity));

    modbus_phy_send_stop();
}

/*
report_slave_id_rsp
Input:     int8       address            Slave Address
           int8       slave_id           Slave Address
           int8       run_status         Are we running?
           int8*      data               Pointer to an array holding the data
           int8       data_len           Length of data in bytes
Output:    void
 */
void modbus_app_report_slave_id_rsp(uint8_t address, uint8_t slave_id, bool run_status,
        uint8_t *data, uint8_t data_len) {
    uint8_t i;

    modbus_phy_send_start(address, FUNC_REPORT_SLAVE_ID);

    modbus_phy_putc(data_len + 2);
    modbus_phy_putc(slave_id);

    if (run_status)
        modbus_phy_putc(0xFF);
    else
        modbus_phy_putc(0x00);

    for (i = 0; i < data_len; ++i) {

        modbus_phy_putc(*data);
        data++;
    }

    modbus_phy_send_stop();
}

/*
read_file_record_rsp
Input:     int8                     address            Slave Address
           int8                     byte_count         Number of bytes to send
           read_sub_request_rsp*    request            Structure holding record/data information
Output:    void
 */
void modbus_app_read_file_record_rsp(uint8_t address, uint8_t byte_count,
        modbus_read_sub_request_rsp * request) {
    uint8_t i = 0, j;

    modbus_phy_send_start(address, FUNC_READ_FILE_RECORD);

    modbus_phy_putc(byte_count);

    while (i < byte_count)
        ;
    {
        modbus_phy_putc(request->record_length);
        modbus_phy_putc(request->reference_type);

        for (j = 0; (j < request->record_length); j += 2) {

            modbus_phy_putc((uint8_t) (request->data[j] >> 8));
            modbus_phy_putc((uint8_t) (request->data[j]));
        }

        i += (request->record_length) + 1;
        request++;
    }

    modbus_phy_send_stop();
}

/*
write_file_record_rsp
Input:     int8                     address            Slave Address
           int8                     byte_count         Echo of number of bytes sent
           write_sub_request_rsp*   request            Echo of Structure holding record information
Output:    void
 */
void modbus_app_write_file_record_rsp(uint8_t address, uint8_t byte_count,
        modbus_write_sub_request_rsp * request) {
    uint8_t i, j = 0;

    modbus_phy_send_start(address, FUNC_WRITE_FILE_RECORD);

    modbus_phy_putc(byte_count);

    for (i = 0; i < byte_count; i += (7 + (j * 2))) {
        modbus_phy_putc(request->reference_type);
        modbus_phy_putc((uint8_t) (request->file_number >> 8));
        modbus_phy_putc((uint8_t) (request->file_number));
        modbus_phy_putc((uint8_t) (request->record_number >> 8));
        modbus_phy_putc((uint8_t) (request->record_number));
        modbus_phy_putc((uint8_t) (request->record_length >> 8));
        modbus_phy_putc((uint8_t) (request->record_length));

        for (j = 0; (j < request->record_length); j += 2) {

            modbus_phy_putc((uint8_t) (request->data[j] >> 8));
            modbus_phy_putc((uint8_t) (request->data[j]));
        }
    }

    modbus_phy_send_stop();
}

/*
mask_write_register_rsp
Input:     int8        address            Slave Address
           int16       reference_address  Echo of reference address
           int16       AND_mask           Echo of AND mask
           int16       OR_mask            Echo or OR mask
Output:    void
 */
void modbus_app_mask_write_register_rsp(uint8_t address, uint16_t reference_address,
        uint16_t AND_mask, uint16_t OR_mask) {

    modbus_phy_send_start(address, FUNC_MASK_WRITE_REGISTER);

    modbus_phy_putc((uint8_t) (reference_address >> 8));
    modbus_phy_putc((uint8_t) (reference_address));

    modbus_phy_putc((uint8_t) (AND_mask >> 8));
    modbus_phy_putc((uint8_t) (AND_mask));

    modbus_phy_putc((uint8_t) (OR_mask >> 8));
    modbus_phy_putc((uint8_t) (OR_mask));

    modbus_phy_send_stop();
}

/*
read_write_multiple_registers_rsp
Input:     int8        address            Slave Address
           int16*      data               Pointer to an array of data
           int8        data_len           Length of data in bytes
Output:    void
 */
void modbus_app_read_write_multiple_registers_rsp(uint8_t address, uint8_t data_len,
        uint16_t * data) {
    uint8_t i;

    modbus_phy_send_start(address, FUNC_READ_WRITE_MULTIPLE_REGISTERS);

    modbus_phy_putc(data_len * 2);

    for (i = 0; i < data_len * 2; i += 2) {

        modbus_phy_putc((uint8_t) (data[i] >> 8));
        modbus_phy_putc((uint8_t) (data[i]));
    }

    modbus_phy_send_stop();
}

/*
read_FIFO_queue_rsp
Input:     int8        address            Slave Address
           int16       FIFO_len           Length of FIFO in bytes
           int16*      data               Pointer to an array of data
Output:    void
 */
void modbus_app_read_FIFO_queue_rsp(uint8_t address, uint16_t FIFO_len, uint16_t * data) {
    uint8_t i;
    uint16_t byte_count;

    byte_count = ((FIFO_len * 2) + 2);

    modbus_phy_send_start(address, FUNC_READ_FIFO_QUEUE);

    modbus_phy_putc((uint8_t) (byte_count >> 8));
    modbus_phy_putc((uint8_t) (byte_count));

    modbus_phy_putc((uint8_t) (FIFO_len >> 8));
    modbus_phy_putc((uint8_t) (FIFO_len));

    for (i = 0; i < FIFO_len; i += 2) {

        modbus_phy_putc((uint8_t) (data[i] >> 8));
        modbus_phy_putc((uint8_t) (data[i]));
    }

    modbus_phy_send_stop();
}

/*
read_FIFO_queue_rsp
Input:     int8        address            Slave Address
           int16       func               function to respond to
           exception   error              exception response to send
Output:    void
 */
void modbus_app_exception_rsp(uint8_t address, uint16_t func, MODBUS_EXCEPTION error) {
    modbus_phy_send_start(address, func | 0x80);
    modbus_phy_putc(error);
    modbus_phy_send_stop();
}
