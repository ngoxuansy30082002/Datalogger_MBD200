#ifndef DRV_GD25Q_LOCAL_H
#define DRV_GD25Q_LOCAL_H

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************
#include <string.h>
#include "configuration.h"
#include "drv_gd25q.h"
// *****************************************************************************
// *****************************************************************************
// Section: Local Data Type Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************

/* GD5F1 Command set

  Summary:
    Enumeration listing the GD5F1VF commands.

  Description:
    This enumeration defines the commands used to interact with the GD5F1VF
    series of devices.

  Remarks:
    None
 */

typedef enum {
    /* Reset enable command. */
    GD25Q_CMD_FLASH_RESET_ENABLE = 0x66,

    /* Command to reset the flash. */
    GD25Q_CMD_FLASH_RESET = 0x99,

    /* Command to Enable QUAD IO */
    GD25Q_CMD_ENABLE_QUAD_IO = 0x38,

    /* Command to Reset QUAD IO */
    GD25Q_CMD_RESET_QUAD_IO = 0xFF,

    /* Command to read JEDEC-ID of the flash device. */
    GD25Q_CMD_JEDEC_ID_READ = 0x9F,

    /* QUAD Command to read JEDEC-ID of the flash device. */
    GD25Q_CMD_QUAD_JEDEC_ID_READ = 0xAF,

    /* Command to perform High Speed Read */
    GD25Q_CMD_HIGH_SPEED_READ = 0x0B,

    /* Write enable command. */
    GD25Q_CMD_WRITE_ENABLE = 0x06,

    /* Page Program command. */
    GD25Q_CMD_PAGE_PROGRAM = 0x02,

    /* Command to read the Flash status register. */
    GD25Q_CMD_READ_STATUS_REG = 0x05,

    /* Command to perform sector erase */
    GD25Q_CMD_SECTOR_ERASE = 0x20,

    /* Command to perform Bulk erase */
    GD25Q_CMD_BULK_ERASE_64K = 0xD8,

    /* Command to perform Chip erase */
    GD25Q_CMD_CHIP_ERASE = 0x60,

    /* Command to unlock the flash device. */
    GD25Q_CMD_UNPROTECT_GLOBAL = 0x01,

    GD25Q_CMD_WRITE_STS_2 = 0x31,

    GD25Q_CMD_QUAD_PAGE_PROGRAM = 0x32,

    GD25Q_CMD_QUAD_OUT_FAST_READ = 0x6B,

} GD25Q_CMD;

// *****************************************************************************

/* GD5F1 Driver operations.

  Summary:
    Enumeration listing the GD5F1 driver operations.

  Description:
    This enumeration defines the possible GD5F1 driver operations.

  Remarks:
    None
 */

/**************************************
 * GD5F1 Driver Hardware Instance Object
 **************************************/
typedef struct {
    /* Flag to indicate in use  */
    bool inUse;

    /* Flag to indication read operation status*/
    volatile bool internal_write_complete_flag;

    /* Flag to indicate status of transfer */
    volatile bool isTransferDone;

    /* The status of the driver */
    SYS_STATUS status;

    /* PLIB API list that will be used by the driver to access the hardware */
    const DRV_GD25Q_PLIB_INTERFACE *gd5f1Plib;

} DRV_GD25Q_OBJECT;





#endif //#ifndef DRV_GD25Q_LOCAL_H

/*******************************************************************************
 End of File
 */

