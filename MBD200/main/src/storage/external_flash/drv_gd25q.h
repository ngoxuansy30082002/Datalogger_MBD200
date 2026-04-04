#ifndef DRV_GD25Q_H
#define DRV_GD25Q_H

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include <stdio.h>
#include "definitions.h"
#include "peripheral/sqi/plib_sqi_common.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility
extern "C" {
#endif

    // DOM-IGNORE-END

    // *****************************************************************************
    // *****************************************************************************
    // Section: Data Types
    // *****************************************************************************
    // *****************************************************************************

    typedef void (*DRV_GD25Q_PLIB_DMA_TRANSFER)(sqi_dma_desc_t *sqiDmaDesc);

    /* Pointer to Register event handler with SQI PLIB. */
    typedef void (*DRV_GD25Q_PLIB_REGISTER_CALLBACK)(SQI_EVENT_HANDLER event_handler, uintptr_t context);

    typedef struct {
        DRV_GD25Q_PLIB_DMA_TRANSFER DMATransfer;

        DRV_GD25Q_PLIB_REGISTER_CALLBACK RegisterCallback;
    } DRV_GD25Q_PLIB_INTERFACE;

    typedef struct {
        const DRV_GD25Q_PLIB_INTERFACE *gd25qPlib;
    } DRV_GD25Q_INIT;

    /*
     Summary:
        GD25Q Driver Transfer Status

     Description:
        This data type will be used to indicate the current transfer status for GD25Q
        driver.

     Remarks:
        None.
     */

    typedef enum {
        /* Transfer is being processed */
        DRV_GD25Q_TRANSFER_BUSY,
        /* Transfer is successfully completed*/
        DRV_GD25Q_TRANSFER_COMPLETED,
        /* Transfer had error*/
        DRV_GD25Q_TRANSFER_ERROR_UNKNOWN,
    } DRV_GD25Q_TRANSFER_STATUS;

    /*
     Summary:
        GD25Q Device Geometry data.

     Description:
        This data type will be used to get the geometry details of the
        GD25Q flash device.

     Remarks:
        None.
     */

    typedef struct {
        uint32_t read_blockSize;
        uint32_t read_numBlocks;
        uint32_t numReadRegions;

        uint32_t write_blockSize;
        uint32_t write_numBlocks;
        uint32_t numWriteRegions;

        uint32_t erase_blockSize;
        uint32_t erase_numBlocks;
        uint32_t numEraseRegions;

        uint32_t blockStartAddress;
    } DRV_GD25Q_GEOMETRY;

    // *****************************************************************************
    // *****************************************************************************
    // Section: GD25Q Driver Module Interface Routines
    // *****************************************************************************
    // *****************************************************************************

    // *****************************************************************************
    /* GD25Q Driver Transfer Event Handler Function Pointer

       Summary
        Pointer to a GD25Q Driver Event handler function

       Description
        This data type defines the required function signature for the GD25Q driver
        event handling callback function. A client must register a pointer
        using the event handling function whose function signature (parameter
        and return value types) match the types specified by this function pointer
        in order to receive transfer related event calls back from the driver.

        This data type is only supported when gd5f1 driver is using
        - QSPI PLIB in SPI mode
        - SPI PLIB

      Parameters:
        event - Identifies the type of event

        context - Value identifying the context of the application that
        registered the event handling function.

      Returns:
        None.

      Example:
        <code>
        void APP_MyTransferEventHandler( DRV_GD25Q_TRANSFER_STATUS event, uintptr_t context )
        {
            MY_APP_DATA_STRUCT* pAppData = (MY_APP_DATA_STRUCT*) context;

            switch(event)
            {
                case DRV_GD25Q_TRANSFER_COMPLETED:
                {
               
                    break;
                }

                case DRV_GD25Q_TRANSFER_ERROR_UNKNOWN:
                default:
                {
                
                    break;
                }
            }
        }
        </code>

      Remarks:
        If the event is DRV_GD25Q_TRANSFER_COMPLETED, it means that the data was
        transferred successfully.

        If the event is DRV_GD25Q_TRANSFER_ERROR_UNKNOWN, it means that the data was not
        transferred successfully.

        The context parameter contains the handle to the client context,
        provided at the time the event handling function was registered using the
        DRV_GD25Q_EventHandlerSet function.  This context handle value is
        passed back to the client as the "context" parameter.  It can be any value
        necessary to identify the client context or instance (such as a pointer to
        the client's data) instance of the client that made the buffer add request.

        The event handler function executes in the driver's interrupt
        context. It is recommended of the application to not perform process
        intensive or blocking operations with in this function.
     */

    typedef void (*DRV_GD25Q_EVENT_HANDLER) (DRV_GD25Q_TRANSFER_STATUS event, uintptr_t context);

    // *****************************************************************************
    /* Function:
        SYS_MODULE_OBJ DRV_GD25Q_Initialize
        (
            const SYS_MODULE_INDEX drvIndex,
            const SYS_MODULE_INIT *const init
        );

      Summary:
        Initializes the GD25Q Driver

      Description:
        This routine initializes the GD25Q driver making it ready for client to use.

      Precondition:
        None.

      Parameters:
        drvIndex -  Identifier for the instance to be initialized

        init     -  Pointer to a data structure containing any data necessary to
                    initialize the driver.

      Returns:
        If successful, returns a valid driver instance object.
        Otherwise it returns SYS_MODULE_OBJ_INVALID.

      Example:
        <code>    

        SYS_MODULE_OBJ  objectHandle;

        const DRV_GD25Q_PLIB_INTERFACE drvGD25QPlibAPI = {
            .CommandWrite  = QSPI_CommandWrite,
            .RegisterRead  = QSPI_RegisterRead,
            .RegisterWrite = QSPI_RegisterWrite,
            .MemoryRead    = QSPI_MemoryRead,
            .MemoryWrite   = QSPI_MemoryWrite
        };

        const DRV_GD25Q_INIT drvGD25QInitData =
        {
            .gd5f1Plib         = &drvGD25QPlibAPI,
        };

        objectHandle = DRV_GD25Q_Initialize((SYS_MODULE_INDEX)DRV_GD25Q_INDEX, (SYS_MODULE_INIT *)&drvGD25QInitData);

        if (SYS_MODULE_OBJ_INVALID == objectHandle)
        {
       
        }
        </code>

      Remarks:
        This routine must be called before any other GD25Q driver routine is called.

        This routine should only be called once during system initialization.
     */

    SYS_MODULE_OBJ DRV_GD25Q_Initialize
    (
            const SYS_MODULE_INDEX drvIndex,
            const SYS_MODULE_INIT * const init
            );

    // ****************************************************************************
    /* Function:
        DRV_HANDLE DRV_GD25Q_Open( const SYS_MODULE_INDEX drvIndex, const DRV_IO_INTENT ioIntent );

      Summary:
        Opens the specified GD25Q driver instance and returns a handle to it

      Description:
        This routine opens the specified GD25Q driver instance and provides a handle.

        It performs the following blocking operations:
        - Resets the Flash Device
        - Puts it on QUAD IO Mode
        - Unlocks the flash if DRV_GD25Q_Open was called with write intent.

        This handle must be provided to all other client-level operations to identify
        the caller and the instance of the driver.

      Preconditions:
        Function DRV_GD25Q_Initialize must have been called before calling this
        function.

        Driver should be in ready state to accept the request. Can be checked by
        calling DRV_GD25Q_Status().

      Parameters:
        drvIndex   -  Identifier for the instance to be opened

        ioIntent   -  Zero or more of the values from the enumeration
                      DRV_IO_INTENT "ORed" together to indicate the intended use
                      of the driver

      Returns:
        If successful, the routine returns a valid open-instance handle (a
        number identifying both the caller and the module instance).

        If an error occurs, DRV_HANDLE_INVALID is returned. Errors can occur
        under the following circumstances:
            - if the driver hardware instance being opened is not initialized.

      Example:
        <code>
        DRV_HANDLE handle;

        handle = DRV_GD25Q_Open(DRV_GD25Q_INDEX);
        if (DRV_HANDLE_INVALID == handle)
        {
      
        }
        </code>

      Remarks:
        The handle returned is valid until the DRV_GD25Q_Close routine is called.

        If the driver has already been opened, it should not be opened again.
     */

    DRV_HANDLE DRV_GD25Q_Open(const SYS_MODULE_INDEX drvIndex, const DRV_IO_INTENT ioIntent);

    // *****************************************************************************
    /* Function:
        void DRV_GD25Q_Close( const DRV_HANDLE handle );

      Summary:
        Closes an opened-instance of the GD25Q driver

      Description:
        This routine closes an opened-instance of the GD25Q driver, invalidating
        the handle.

      Precondition:
        DRV_GD25Q_Open must have been called to obtain a valid opened device handle.

      Parameters:
        handle       - A valid open-instance handle, returned from the driver's
                       open routine

      Returns:
        None

      Example:
        <code>
        DRV_HANDLE handle; 

        DRV_GD25Q_Close(handle);
        </code>

      Remarks:
        After calling this routine, the handle passed in "handle" must not be used
        with any of the remaining driver routines. A new handle must be obtained by
        calling DRV_GD25Q_Open before the caller may use the driver again.

        Usually there is no need for the driver client to verify that the Close
        operation has completed.
     */

    void DRV_GD25Q_Close(const DRV_HANDLE handle);

    // *************************************************************************
    /* Function:
        SYS_STATUS DRV_GD25Q_Status( const SYS_MODULE_INDEX drvIndex );

      Summary:
        Gets the current status of the GD25Q driver module.

      Description:
        This routine provides the current status of the GD25Q driver module.

      Preconditions:
        Function DRV_GD25Q_Initialize should have been called before calling
        this function.

      Parameters:
        drvIndex   -  Identifier for the instance used to initialize driver

      Returns:
        SYS_STATUS_READY - Indicates that the driver is ready and accept
        requests for new operations.

        SYS_STATUS_UNINITIALIZED - Indicates the driver is not initialized.

        SYS_STATUS_BUSY - Indicates the driver is in busy state.

      Example:
        <code>
        SYS_STATUS          Status;

        Status = DRV_GD25Q_Status(DRV_GD25Q_INDEX);

        if (status == SYS_STATUS_READY)
        {
        
        }
        </code>

      Remarks:
        None.
     */

    SYS_STATUS DRV_GD25Q_Status();

    // *****************************************************************************
    /* Function:
        bool DRV_GD25Q_UnlockFlash( const DRV_HANDLE handle );

      Summary:
        Unlocks the flash device for Erase and Program operations.

      Description:
        This function schedules a blocking operation for unlocking the flash blocks
        globally. This allows to perform erase and program operations on the flash.

      Precondition:
        The DRV_GD25Q_Open() routine must have been called for the
        specified GD25Q driver instance.

      Parameters:
        handle       - A valid open-instance handle, returned from the driver's
                       open routine

      Returns:
        true
            - if the unlock is successfully completed

        false
            - if Write enable fails before sending unlock command to flash and 
            - if Unlock flash command itself fails

      Example:
        <code>
        DRV_HANDLE handle;  

        if(DRV_GD25Q_UnlockFlash(handle) == false)
        {
        
        }

        </code>

      Remarks:
        None.
     */

    bool DRV_GD25Q_UnlockFlash(const DRV_HANDLE handle);

    // *****************************************************************************
    /* Function:
        bool DRV_GD25Q_ReadJedecId( const DRV_HANDLE handle, void *jedec_id );

      Summary:
        Reads JEDEC-ID of the flash device.

      Description:
        This function schedules a blocking operation for reading the JEDEC-ID.
        This information can be used to get the flash device geometry.

      Precondition:
        The DRV_GD25Q_Open() routine must have been called for the
        specified GD25Q driver instance.

      Parameters:
        handle       - A valid open-instance handle, returned from the driver's
                       open routine

      Returns:
        true  - if the read is successfully completed

        false - if read jedec-id command fails

      Example:
        <code>

        DRV_HANDLE handle;  
        uint32_t jedec_id = 0;

        if(DRV_GD25Q_ReadJedecId(handle, &jedec_id) == false)
        {
        
        }

        </code>

      Remarks:
        None.
     */

    bool DRV_GD25Q_ReadJedecId(const DRV_HANDLE handle, void *jedec_id);

    // **************************************************************************
    /* Function:
        bool DRV_GD25Q_SectorErase( const DRV_HANDLE handle, uint32_t address );

      Summary:
        Erase the sector from the specified block start address.

      Description:
        This function schedules a non-blocking sector erase operation of flash memory.
        Each Sector is of 4 KByte.

        The requesting client should call DRV_GD25Q_TransferStatusGet() API to know
        the current status of the request.

      Preconditions:
        The DRV_GD25Q_Open() routine must have been called for the
        specified GD25Q driver instance.

      Parameters:
        handle        - A valid open-instance handle, returned from the driver's
                       open routine

        address       - block start address from where a sector needs to be erased.

      Returns:
        true
            - if the erase request is successfully sent to the flash

        false
            - if Write enable fails before sending sector erase command to flash
            - if sector erase command itself fails

      Example:
        <code>

        DRV_HANDLE handle;  
        uint32_t sectorStart = 0;

        if(DRV_GD25Q_SectorErase(handle, sectorStart) == false)
        {
        
        }
   
        while(DRV_GD25Q_TRANSFER_BUSY == DRV_GD25Q_TransferStatusGet(handle));

        </code>

      Remarks:
        None.
     */

    bool DRV_GD25Q_SectorErase(const DRV_HANDLE handle, uint32_t address);

    // **************************************************************************
    /* Function:
        bool DRV_GD25Q_BulkErase( const DRV_HANDLE handle, uint32_t address );

      Summary:
        Erase a block from the specified block start address.

      Description:
        This function schedules a non-blocking block erase operation of flash memory.
        The block size can be 8 KByte, 32KByte or 64 KByte.

        The requesting client should call DRV_GD25Q_TransferStatusGet() API to know
        the current status of the request.

      Preconditions:
        The DRV_GD25Q_Open() routine must have been called for the
        specified GD25Q driver instance.

      Parameters:
        handle        - A valid open-instance handle, returned from the driver's
                       open routine

        address       - block start address to be erased.

      Returns:
        true
            - if the erase request is successfully sent to the flash

        false
            - if Write enable fails before sending sector erase command to flash
            - if block erase command itself fails

      Example:
        <code>

        DRV_HANDLE handle;  
        uint32_t blockStart = 0;

        if(DRV_GD25Q_SectorErase(handle, blockStart) == false)
        {
        
        }
   
        while(DRV_GD25Q_TransferStatusGet(handle) == DRV_GD25Q_TRANSFER_BUSY);

        </code>

      Remarks:
        None.
     */

    bool DRV_GD25Q_BulkErase(const DRV_HANDLE handle, uint32_t address);

    // **************************************************************************
    /* Function:
        bool DRV_GD25Q_ChipErase( const DRV_HANDLE handle );

      Summary:
        Erase entire flash memory.

      Description:
        This function schedules a non-blocking chip erase operation of flash memory.

        The requesting client should call DRV_GD25Q_TransferStatusGet() API to know
        the current status of the request.

      Preconditions:
        The DRV_GD25Q_Open() routine must have been called for the
        specified GD25Q driver instance.

      Parameters:
        handle        - A valid open-instance handle, returned from the driver's
                        open routine

      Returns:
        true
            - if the erase request is successfully sent to the flash

        false
            - if Write enable fails before sending sector erase command to flash
            - if chip erase command itself fails

      Example:
        <code>

        DRV_HANDLE handle; 

        if(DRV_GD25Q_ChipErase(handle) == flase)
        {
        
        }
   
        while(DRV_GD25Q_TransferStatusGet(handle) == DRV_GD25Q_TRANSFER_BUSY);

        </code>

      Remarks:
        None.
     */

    bool DRV_GD25Q_ChipErase(const DRV_HANDLE handle);

    // *****************************************************************************
    /* Function:
        bool DRV_GD25Q_Read( const DRV_HANDLE handle, void *rx_data, uint32_t rx_data_length, uint32_t address );

      Summary:
        Reads n bytes of data from the specified start address of flash memory.

      Description:
        This function schedules a blocking operation for reading requested
        number of data bytes from the flash memory.

      Precondition:
        The DRV_GD25Q_Open() routine must have been called for the
        specified GD25Q driver instance.

      Parameters:
        handle          - A valid open-instance handle, returned from the driver's
                          open routine

     *rx_data        - Buffer pointer into which the data read from the GD25Q
                          Flash memory will be placed.

        rx_data_length  - Total number of bytes to be read.

        address         - Read memory start address from where the data should be
                          read.

      Returns:
        true - if number of bytes requested are read from flash memory

        false - if read command itself fails

      Example:
        <code>

        #define BUFFER_SIZE  1024
        #define MEM_ADDRESS  0x0

        DRV_HANDLE handle;  
        uint8_t CACHE_ALIGN readBuffer[BUFFER_SIZE];

        if (DRV_GD25Q_Read(handle, (void *)&readBuffer, BUFFER_SIZE, MEM_ADDRESS) == false)
        {
        
        }
   
        while(DRV_GD25Q_TransferStatusGet(handle) == DRV_GD25Q_TRANSFER_BUSY);

        </code>

      Remarks:
        None.
     */

    bool DRV_GD25Q_Read(const DRV_HANDLE handle, void *rx_data, uint32_t rx_data_length, uint32_t address);

    // *****************************************************************************
    /* Function:
        bool DRV_GD25Q_PageWrite( const DRV_HANDLE handle, void *tx_data, uint32_t tx_data_length, uint32_t address );

      Summary:
        Writes one page of data starting at the specified address.

      Description:
        This function schedules a non-blocking write operation for writing maximum one page
        of data into flash memory.

        The requesting client should call DRV_GD25Q_TransferStatusGet() API to know
        the current status of the request.

      Preconditions:
        The DRV_GD25Q_Open() routine must have been called for the
        specified GD25Q driver instance.

        The flash address location which has to be written, must have been erased
        before using the GD25Q_xxxErase() routine.

        The flash address has to be a Page aligned address.

      Parameters:
        handle          - A valid open-instance handle, returned from the driver's
                          open routine

     *tx_data        - The source buffer containing data to be programmed into GD25Q
                          Flash

        tx_data_length  - Total number of bytes to be written. should not be greater
                          than page size

        address         - Write memory start address from where the data should be
                          written

      Returns:
        true
            - if the write request is successfully sent to the flash

        false
            - if Write enable fails before sending sector erase command to flash
            - if write command itself fails

      Example:
        <code>

        #define PAGE_SIZE    256
        #define BUFFER_SIZE  1024
        #define MEM_ADDRESS  0x0

        DRV_HANDLE handle;  
        uint8_t CACHE_ALIGN writeBuffer[BUFFER_SIZE];
        bool status = false;

        if(false == DRV_GD25Q_SectorErase(handle))
        {
        
        }
   
        while(DRV_GD25Q_TransferStatusGet(handle) == DRV_GD25Q_TRANSFER_BUSY);

        for (uint32_t j = 0; j < BUFFER_SIZE; j += PAGE_SIZE)
        {
            if (DRV_GD25Q_PageWrite(handle, (void *)&writeBuffer[j], (MEM_ADDRESS + j)) == false)
            {
                status = false;
                break;
            }

        
            while(DRV_GD25Q_TransferStatusGet(handle) == DRV_GD25Q_TRANSFER_BUSY);
            status = true;
        }

        if(status == false)
        {
        
        }

        </code>

      Remarks:
        None.
     */

    bool DRV_GD25Q_PageWrite(const DRV_HANDLE handle, void *tx_data, uint32_t address);

    bool DRV_GD25Q_ByteWrite(const DRV_HANDLE handle, void *tx_data, uint32_t address, uint32_t lenData);
    // *****************************************************************************
    /* Function:
        DRV_GD25Q_TRANSFER_STATUS DRV_GD25Q_TransferStatusGet( const DRV_HANDLE handle );

      Summary:
        Gets the current status of the transfer request.

      Description:
        This routine gets the current status of the transfer request. The application
        must use this routine where the status of a scheduled request needs to be
        polled on.

      Preconditions:
        The DRV_GD25Q_Open() routine must have been called for the
        specified GD25Q driver instance.

      Parameters:
        handle          - A valid open-instance handle, returned from the driver's
                          open routine

      Returns:
        DRV_GD25Q_TRANSFER_ERROR_UNKNOWN - If the flash status register read request fails

        DRV_GD25Q_TRANSFER_BUSY - If the current transfer request is still being processed

        DRV_GD25Q_TRANSFER_COMPLETED - If the transfer request is completed

      Example:
        <code>

        DRV_HANDLE handle; 

        if (DRV_GD25Q_TransferStatusGet(handle) == DRV_GD25Q_TRANSFER_COMPLETED)
        {
       
        }
        </code>

      Remarks:
        None.
     */

    DRV_GD25Q_TRANSFER_STATUS DRV_GD25Q_TransferStatusGet(const DRV_HANDLE handle);

    // *****************************************************************************
    /* Function:
        bool DRV_GD25Q_GeometryGet( const DRV_HANDLE handle, GD25Q_GEOMETRY *geometry );

      Summary:
        Returns the geometry of the device.

      Description:
        This API gives the following geometrical details of the GD25Q Flash:
        - Number of Read/Write/Erase Blocks and their size in each region of the device
        - Flash block start address.

      Precondition:
        The DRV_GD25Q_Open() routine must have been called for the
        specified GD25Q driver instance.

      Parameters:
        handle            - A valid open-instance handle, returned from the driver's
                            open routine

     *geometry_table   - pointer to flash device geometry table instance

      Returns:
        true  - if able to get the geometry details of the flash

        false - if read device id fails

      Example:
        <code>

        DRV_HANDLE handle;  

        DRV_GD25Q_GEOMETRY gd5f1FlashGeometry;

        uint32_t readBlockSize, writeBlockSize, eraseBlockSize;
        uint32_t nReadBlocks, nReadRegions, totalFlashSize;

        DRV_GD25Q_GeometryGet(handle, &gd5f1FlashGeometry);

        readBlockSize  = gd5f1FlashGeometry.read_blockSize;
        nReadBlocks = gd5f1FlashGeometry.read_numBlocks;
        nReadRegions = gd5f1FlashGeometry.numReadRegions;

        writeBlockSize  = gd5f1FlashGeometry.write_blockSize;
        eraseBlockSize  = gd5f1FlashGeometry.erase_blockSize;

        totalFlashSize = readBlockSize * nReadBlocks * nReadRegions;

        </code>

      Remarks:
        This API is more useful when used to interface with Memory driver.
     */

    bool DRV_GD25Q_GeometryGet(const DRV_HANDLE handle, DRV_GD25Q_GEOMETRY *geometry);

    // *****************************************************************************
    /* Function:
        void DRV_GD25Q_EventHandlerSet(
            const DRV_HANDLE handle,
            const DRV_GD25Q_EVENT_HANDLER eventHandler,
            const uintptr_t context
        )

      Summary:
        Allows a client to identify a transfer event handling function for the driver
        to call back when the requested transfer has finished.

      Description:
        This function allows a client to register a transfer event handling function
        with the driver to call back when the requested transfer has finished.

        The event handler should be set before the client submits any transfer
        requests that could generate events. The event handler once set, persists
        until the client closes the driver or sets another event handler (which
        could be a "NULL" pointer to indicate no callback).

        This function is only supported when gd5f1 driver is using
        - QSPI PLIB in SPI mode
        - SPI PLIB

      Precondition:
        DRV_GD25Q_Open must have been called to obtain a valid opened device handle.

      Parameters:
        handle - A valid open-instance handle, returned from the driver's open routine.

        eventHandler - Pointer to the event handler function.

        context - The value of parameter will be passed back to the client
        unchanged, when the eventHandler function is called.  It can be used to
        identify any client specific data object that identifies the instance of the
        client module (for example, it may be a pointer to the client module's state
        structure).

      Returns:
        None.

      Example:
        <code>

        #define BUFFER_SIZE  256
        #define MEM_ADDRESS  0x00

    
        MY_APP_OBJ myAppObj;

        uint8_t CACHE_ALIGN myBuffer[BUFFER_SIZE];

    
        void APP_GD25QTransferEventHandler(DRV_GD25Q_TRANSFER_STATUS event, uintptr_t context)
        {
       
            MY_APP_OBJ* pMyAppObj = (MY_APP_OBJ *) context;

            switch(event)
            {
                case DRV_GD25Q_TRANSFER_COMPLETED:
                {
             
                    break;
                }

                case DRV_GD25Q_TRANSFER_ERROR:
                {
              
                    break;
                }

                default:
                {
                    break;
                }
            }
        }   

        DRV_GD25Q_EventHandlerSet( myHandle, APP_GD25QTransferEventHandler, (uintptr_t)&myAppObj );

        if (DRV_GD25Q_Read(myHandle, myBuffer, BUFFER_SIZE, MEM_ADDRESS) == false)
        {
       
        }

        </code>

      Remarks:
        If the client does not want to be notified when the queued buffer transfer
        has completed, it does not need to register a callback.
     */
    /* MISRA C-2012 Rule 8.6 deviated:2 Deviation record ID -  H3_MISRAC_2012_R_8_6_DR_1 */

    void DRV_GD25Q_EventHandlerSet(
            const DRV_HANDLE handle,
            const DRV_GD25Q_EVENT_HANDLER eventHandler,
            const uintptr_t context
            );

    bool DRV_GD25Q_ReadStatus(const DRV_HANDLE handle, void *rx_data, uint32_t rx_data_length);

    /* MISRAC 2012 deviation block end */

#ifdef __cplusplus
}
#endif

#endif // #ifndef DRV_GD25Q_H
/*******************************************************************************
 End of File
 */