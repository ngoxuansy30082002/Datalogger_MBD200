#ifndef MODBUS_APP_LAYER_H
#define	MODBUS_APP_LAYER_H

#include "modbus_rtu_phy.h"

#ifdef	__cplusplus
extern "C" {
#endif

    typedef struct _modbus_read_sub_request_rsp {
        uint8_t record_length;
        uint8_t reference_type;
        uint16_t data[((MODBUS_SERIAL_RX_BUFFER_SIZE) / 2) - 3];
    } modbus_read_sub_request_rsp;

    typedef struct _modbus_write_sub_request_rsp {
        uint8_t reference_type;
        uint16_t file_number;
        uint16_t record_number;
        uint16_t record_length;
        uint16_t data[((MODBUS_SERIAL_RX_BUFFER_SIZE) / 2) - 8];
    } modbus_write_sub_request_rsp;

    typedef struct _modbus_read_sub_request {
        uint8_t reference_type;
        uint16_t file_number;
        uint16_t record_number;
        uint16_t record_length;
    } modbus_read_sub_request;

    typedef struct _modbus_write_sub_request {
        uint8_t reference_type;
        uint16_t file_number;
        uint16_t record_number;
        uint16_t record_length;
        uint16_t data[MODBUS_SERIAL_RX_BUFFER_SIZE - 8];
    } modbus_write_sub_request;

    // Purpose:    Initialize RS485 communication. Call this before
    //             using any other RS485 functions.
    // Inputs:     None
    // Outputs:    None
    void modbus_app_init(MODBUS_UART_INTERFACE interface);

    // Purpose:    Get a message from the RS485 bus and store it in a buffer
    // Inputs:     None
    // Outputs:    TRUE if a message was received
    //             FALSE if no message is available
    // Note:       Data will be filled in at the modbus_rx struct:
    bool modbus_app_kbhit();

    MODBUS_TRANSFER_DATA * modbus_app_get_transfer_pointer();

    void modbus_app_push_request(MODBUS_STATES state);

    bool modbus_app_transfer_done();

    void modbus_app_task();

    /*
    read_coils_rsp
    Input:     int8       address            Slave Address
               int8       byte_count         Number of bytes being sent
               int8*      coil_data          Pointer to an array of data to send
    Output:    void
     */
    void modbus_app_read_coils_rsp(uint8_t address, uint8_t byte_count, uint8_t* coil_data);

    /*
    read_discrete_input_rsp
    Input:     int8       address            Slave Address
               int8       byte_count         Number of bytes being sent
               int8*      input_data         Pointer to an array of data to send
    Output:    void
     */
    void modbus_app_read_discrete_input_rsp(uint8_t address, uint8_t byte_count,
            uint8_t *input_data);

    /*
    read_holding_registers_rsp
    Input:     int8       address            Slave Address
               int8       byte_count         Number of bytes being sent
               int8*      reg_data           Pointer to an array of data to send
    Output:    void
     */
    void modbus_app_read_holding_registers_rsp(uint8_t address, uint8_t byte_count,
            uint16_t *reg_data);

    /*
    read_input_registers_rsp
    Input:     int8       address            Slave Address
               int8       byte_count         Number of bytes being sent
               int8*      input_data         Pointer to an array of data to send
    Output:    void
     */
    void modbus_app_read_input_registers_rsp(uint8_t address, uint8_t byte_count,
            uint16_t *input_data);

    /*
    write_single_coil_rsp
    Input:     int8       address            Slave Address
               int16      output_address     Echo of output address received
               int16      output_value       Echo of output value received
    Output:    void
     */
    void modbus_app_write_single_coil_rsp(uint8_t address, uint16_t output_address,
            uint16_t output_value);

    /*
    write_single_register_rsp
    Input:     int8       address            Slave Address
               int16      reg_address        Echo of register address received
               int16      reg_value          Echo of register value received
    Output:    void
     */
    void modbus_app_write_single_register_rsp(uint8_t address, uint16_t reg_address,
            uint16_t reg_value);

    /*
    read_exception_status_rsp
    Input:     int8       address            Slave Address
    Output:    void
     */
    void modbus_app_read_exception_status_rsp(uint8_t address, uint8_t data);

    /*
    diagnostics_rsp
    Input:     int8       address            Slave Address
               int16      sub_func           Echo of sub function received
               int16      data               Echo of data received
    Output:    void
     */
    void modbus_app_diagnostics_rsp(uint8_t address, uint16_t sub_func, uint16_t data);
    /*
    get_comm_event_counter_rsp
    Input:     int8       address            Slave Address
               int16      status             Status, refer to MODBUS documentation
               int16      event_count        Count of events
    Output:    void
     */
    void modbus_app_get_comm_event_counter_rsp(uint8_t address, uint16_t status,
            uint16_t event_count);

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
            uint8_t *events, uint8_t events_len);

    /*
    write_multiple_coils_rsp
    Input:     int8       address            Slave Address
               int16      start_address      Echo of address to start at
               int16      quantity           Echo of amount of coils written to
    Output:    void
     */
    void modbus_app_write_multiple_coils_rsp(uint8_t address, uint16_t start_address,
            uint16_t quantity);

    /*
    write_multiple_registers_rsp
    Input:     int8       address            Slave Address
               int16      start_address      Echo of address to start at
               int16      quantity           Echo of amount of registers written to
    Output:    void
     */
    void modbus_app_write_multiple_registers_rsp(uint8_t address, uint16_t start_address,
            uint16_t quantity);

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
            uint8_t *data, uint8_t data_len);

    /*
    read_file_record_rsp
    Input:     int8                     address            Slave Address
               int8                     byte_count         Number of bytes to send
               read_sub_request_rsp*    request            Structure holding record/data information
    Output:    void
     */
    void modbus_app_read_file_record_rsp(uint8_t address, uint8_t byte_count,
            modbus_read_sub_request_rsp *request);

    /*
    write_file_record_rsp
    Input:     int8                     address            Slave Address
               int8                     byte_count         Echo of number of bytes sent
               write_sub_request_rsp*   request            Echo of Structure holding record information
    Output:    void
     */
    void modbus_app_write_file_record_rsp(uint8_t address, uint8_t byte_count,
            modbus_write_sub_request_rsp *request);

    /*
    mask_write_register_rsp
    Input:     int8        address            Slave Address
               int16       reference_address  Echo of reference address
               int16       AND_mask           Echo of AND mask
               int16       OR_mask            Echo or OR mask
    Output:    void
     */
    void modbus_app_mask_write_register_rsp(uint8_t address, uint16_t reference_address,
            uint16_t AND_mask, uint16_t OR_mask);

    /*
    read_write_multiple_registers_rsp
    Input:     int8        address            Slave Address
               int16*      data               Pointer to an array of data
               int8        data_len           Length of data in bytes
    Output:    void
     */
    void modbus_app_read_write_multiple_registers_rsp(uint8_t address, uint8_t data_len,
            uint16_t *data);

    /*
    read_FIFO_queue_rsp
    Input:     int8        address            Slave Address
               int16       FIFO_len           Length of FIFO in bytes
               int16*      data               Pointer to an array of data
    Output:    void
     */
    void modbus_app_read_FIFO_queue_rsp(uint8_t address, uint16_t FIFO_len, uint16_t *data);

    void modbus_app_exception_rsp(uint8_t address, uint16_t func, MODBUS_EXCEPTION error);


#ifdef	__cplusplus
}
#endif

#endif	/* MODBUS_APP_LAYER_H */

