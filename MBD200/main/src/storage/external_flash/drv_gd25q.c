#include "drv_gd25q_local.h"
#include "sys/kmem.h"
#include "system/console/sys_console.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global objects
// *****************************************************************************
// *****************************************************************************

#define SQI_CHIP_SELECT         SQI_BDCTRL_SQICS_CS0

#define DRV_SQI_LANE_MODE       SQI_BDCTRL_MODE_QUAD_LANE

#define CMD_DESC_NUMBER         5
#define DUMMY_BYTE              0xFF

static DRV_GD25Q_OBJECT gDrvGD5F1Obj;
static DRV_GD25Q_OBJECT *dObj = &gDrvGD5F1Obj;

/* Table mapping the Flash ID's to their sizes. */
static uint32_t gSstFlashIdSizeTable [6][2] = {
    {0x01, 0x200000}, /* 16 MBit */
    {0x41, 0x200000}, /* 16 MBit */
    {0x02, 0x400000}, /* 32 MBit */
    {0x42, 0x400000}, /* 32 MBit */
    {0x43, 0x800000}, /* 64 MBit */
    {0x18, 0x1000000}, /* 128 MBit */
};

static sqi_dma_desc_t CACHE_ALIGN sqiCmdDesc[CMD_DESC_NUMBER];
static sqi_dma_desc_t CACHE_ALIGN sqiBufDesc[DRV_GD25Q_BUFF_DESC_NUMBER];

static uint8_t CACHE_ALIGN statusRegVal;
static uint8_t CACHE_ALIGN jedecID[4];

static uint8_t CACHE_ALIGN sqi_cmd_1[8];
static uint8_t CACHE_ALIGN sqi_cmd_2[8];

// *****************************************************************************
// *****************************************************************************
// Section: GD5F1 Driver Local Functions
// *****************************************************************************
// *****************************************************************************

static uint32_t maxPowerOfTwo(uint32_t num) {
    uint32_t ret_val = 0;
    uint32_t number = 0;

    for (number = num; number >= 1U; number--) {
        // If number is a power of 2
        if ((number & (number - 1U)) == 0U) {
            ret_val = number;
            break;
        }
    }
    return ret_val;
}

static void DRV_GD25Q_EventHandler(uintptr_t context) {
    DRV_GD25Q_OBJECT *obj = (DRV_GD25Q_OBJECT *) context;

    obj->isTransferDone = true;
}

/* This function returns the flash size in bytes for the specified deviceId. A
 * zero is returned if the device id is not supported. */
static uint32_t DRV_GD25Q_GetFlashSize(uint8_t deviceId) {
    uint8_t i = 0;

    for (i = 0; i < 6; i++) {
        if (deviceId == gSstFlashIdSizeTable[i][0]) {
            return gSstFlashIdSizeTable[i][1];
        }
    }

    return 0;
}

static void DRV_GD25Q_ResetFlash(void) {
    dObj->isTransferDone = false;

    sqi_cmd_1[0] = (uint8_t) GD25Q_CMD_FLASH_RESET_ENABLE;
    sqiCmdDesc[0].bd_ctrl = (SQI_BDCTRL_BUFFLEN_VAL(1) | SQI_BDCTRL_PKTINTEN |
            SQI_BDCTRL_LASTPKT | SQI_BDCTRL_LASTBD |
            SQI_CHIP_SELECT | SQI_BDCTRL_DEASSERT |
            SQI_BDCTRL_DESCEN);

    sqiCmdDesc[0].bd_bufaddr = (uint32_t *) KVA_TO_PA(&sqi_cmd_1);
    sqiCmdDesc[0].bd_stat = 0;
    sqiCmdDesc[0].bd_nxtptr = NULL;

    dObj->gd5f1Plib->DMATransfer((sqi_dma_desc_t *) KVA_TO_PA(&sqiCmdDesc[0]));

    while (dObj->isTransferDone == false) {
        /* Wait for transfer to complete */
    }

    dObj->isTransferDone = false;

    sqi_cmd_2[0] = (uint8_t) GD25Q_CMD_FLASH_RESET;
    sqiCmdDesc[1].bd_ctrl = (SQI_BDCTRL_BUFFLEN_VAL(1) | SQI_BDCTRL_PKTINTEN |
            SQI_BDCTRL_LASTPKT | SQI_BDCTRL_LASTBD |
            SQI_CHIP_SELECT | SQI_BDCTRL_DEASSERT |
            SQI_BDCTRL_DESCEN);

    sqiCmdDesc[1].bd_bufaddr = (uint32_t *) KVA_TO_PA(&sqi_cmd_2);
    sqiCmdDesc[1].bd_stat = 0;
    sqiCmdDesc[1].bd_nxtptr = NULL;

    dObj->gd5f1Plib->DMATransfer((sqi_dma_desc_t *) KVA_TO_PA(&sqiCmdDesc[1]));

    while (dObj->isTransferDone == false) {
        /* Wait for transfer to complete */
    }
}

static void DRV_GD25Q_WriteEnable(void) {
    sqi_cmd_1[0] = (uint8_t) GD25Q_CMD_WRITE_ENABLE;

    sqiCmdDesc[0].bd_ctrl = (SQI_BDCTRL_BUFFLEN_VAL(1) |
            SQI_CHIP_SELECT | SQI_BDCTRL_DEASSERT |
            SQI_BDCTRL_DESCEN);
    sqiCmdDesc[0].bd_bufaddr = (uint32_t *) KVA_TO_PA(&sqi_cmd_1);
    sqiCmdDesc[0].bd_stat = 0;
    sqiCmdDesc[0].bd_nxtptr = (sqi_dma_desc_t *) KVA_TO_PA(&sqiCmdDesc[1]);
}

static void DRV_GD25Q_EnableQuadIO(void) {
    dObj->isTransferDone = false;

    DRV_GD25Q_WriteEnable();

    sqi_cmd_2[0] = (uint8_t) GD25Q_CMD_WRITE_STS_2;
    sqiCmdDesc[1].bd_ctrl = (SQI_BDCTRL_BUFFLEN_VAL(1) |
            SQI_CHIP_SELECT | SQI_BDCTRL_DESCEN);
    sqiCmdDesc[1].bd_bufaddr = (uint32_t *) KVA_TO_PA(sqi_cmd_2);
    sqiCmdDesc[1].bd_stat = 0;
    sqiCmdDesc[1].bd_nxtptr = (sqi_dma_desc_t *) KVA_TO_PA(&sqiBufDesc[0]);

    statusRegVal = (uint8_t) 0x02;
    sqiBufDesc[0].bd_ctrl = (SQI_BDCTRL_BUFFLEN_VAL(1) | SQI_BDCTRL_PKTINTEN |
            SQI_BDCTRL_LASTPKT | SQI_BDCTRL_LASTBD |
            SQI_CHIP_SELECT | SQI_BDCTRL_DEASSERT |
            SQI_BDCTRL_DESCEN);
    sqiBufDesc[0].bd_bufaddr = (uint32_t *) KVA_TO_PA(&statusRegVal);
    sqiBufDesc[0].bd_stat = 0;
    sqiBufDesc[0].bd_nxtptr = NULL;

    dObj->gd5f1Plib->DMATransfer((sqi_dma_desc_t *) KVA_TO_PA(&sqiCmdDesc[0]));

    while (dObj->isTransferDone == false) {
        /* Wait for transfer to finish */
    }
}

static bool DRV_GD25Q_ValidateHandleAndCheckBusy(const DRV_HANDLE handle) {
    /* Validate the handle.
     * If isTransferDone is False then there is an operation in progress.
     */
    if (handle == DRV_HANDLE_INVALID || dObj->isTransferDone == false) {
        return true;
    }

    return false;
}
// *****************************************************************************
// *****************************************************************************
// Section: GD5F1 Driver Global Functions
// *****************************************************************************
// *****************************************************************************

bool DRV_GD25Q_UnlockFlash(const DRV_HANDLE handle) {
    if (DRV_GD25Q_ValidateHandleAndCheckBusy(handle) == true) {
        return false;
    }

    dObj->isTransferDone = false;

    DRV_GD25Q_WriteEnable();

    sqi_cmd_2[0] = (uint8_t) GD25Q_CMD_UNPROTECT_GLOBAL;
    sqiCmdDesc[1].bd_ctrl = (SQI_BDCTRL_BUFFLEN_VAL(1) |
            SQI_CHIP_SELECT | SQI_BDCTRL_DESCEN);
    sqiCmdDesc[1].bd_bufaddr = (uint32_t *) KVA_TO_PA(&sqi_cmd_2);
    sqiCmdDesc[1].bd_stat = 0;
    sqiCmdDesc[1].bd_nxtptr = (sqi_dma_desc_t *) KVA_TO_PA(&sqiBufDesc[0]);


    statusRegVal = (uint8_t) 0x00;
    sqiBufDesc[0].bd_ctrl = (SQI_BDCTRL_BUFFLEN_VAL(1) | SQI_BDCTRL_PKTINTEN |
            SQI_BDCTRL_LASTPKT | SQI_BDCTRL_LASTBD |
            SQI_CHIP_SELECT | SQI_BDCTRL_DEASSERT |
            SQI_BDCTRL_DESCEN);
    sqiBufDesc[0].bd_bufaddr = (uint32_t *) KVA_TO_PA(&statusRegVal);
    sqiBufDesc[0].bd_stat = 0;
    sqiBufDesc[0].bd_nxtptr = NULL;

    dObj->gd5f1Plib->DMATransfer((sqi_dma_desc_t *) KVA_TO_PA(&sqiCmdDesc[0]));

    while (dObj->isTransferDone == false) {
        /* Wait for  transfer to complete */
    }
    return true;
}

bool DRV_GD25Q_ReadJedecId(const DRV_HANDLE handle, void *jedec_id) {
    if (DRV_GD25Q_ValidateHandleAndCheckBusy(handle) == true) {
        return false;
    }

    dObj->isTransferDone = false;

    sqi_cmd_1[0] = (uint8_t) GD25Q_CMD_JEDEC_ID_READ;
    sqiCmdDesc[0].bd_ctrl = (SQI_BDCTRL_BUFFLEN_VAL(1) |
            SQI_CHIP_SELECT | SQI_BDCTRL_DESCEN);
    sqiCmdDesc[0].bd_bufaddr = (uint32_t *) KVA_TO_PA(&sqi_cmd_1);
    sqiCmdDesc[0].bd_stat = 0;
    sqiCmdDesc[0].bd_nxtptr = (sqi_dma_desc_t *) KVA_TO_PA(&sqiBufDesc[0]);

    sqiBufDesc[0].bd_ctrl = (SQI_BDCTRL_BUFFLEN_VAL(4) | SQI_BDCTRL_PKTINTEN |
            SQI_BDCTRL_LASTPKT | SQI_BDCTRL_LASTBD | SQI_BDCTRL_DIR_READ |
            SQI_CHIP_SELECT | SQI_BDCTRL_DEASSERT |
            SQI_BDCTRL_DESCEN);
    sqiBufDesc[0].bd_bufaddr = (uint32_t *) KVA_TO_PA((uint8_t*) jedec_id);
    sqiBufDesc[0].bd_stat = 0;
    sqiBufDesc[0].bd_nxtptr = NULL;

    dObj->gd5f1Plib->DMATransfer((sqi_dma_desc_t *) KVA_TO_PA(&sqiCmdDesc[0]));

    while (dObj->isTransferDone == false) {
        /* Wait for transfer to complete */
    }

    return true;
}

bool DRV_GD25Q_ReadStatus(const DRV_HANDLE handle, void *rx_data, uint32_t rx_data_length) {
    uint8_t *status = (uint8_t *) rx_data;

    if (DRV_GD25Q_ValidateHandleAndCheckBusy(handle) == true) {
        return false;
    }

    dObj->isTransferDone = false;

    sqi_cmd_1[0] = (uint8_t) GD25Q_CMD_READ_STATUS_REG;
    sqi_cmd_1[1] = DUMMY_BYTE;
    sqiCmdDesc[0].bd_ctrl = (SQI_BDCTRL_BUFFLEN_VAL(2) |
            SQI_CHIP_SELECT | SQI_BDCTRL_DESCEN);
    sqiCmdDesc[0].bd_bufaddr = (uint32_t *) KVA_TO_PA(&sqi_cmd_1);
    sqiCmdDesc[0].bd_stat = 0;
    sqiCmdDesc[0].bd_nxtptr = (sqi_dma_desc_t *) KVA_TO_PA(&sqiBufDesc[0]);

    sqiBufDesc[0].bd_ctrl = (SQI_BDCTRL_BUFFLEN_VAL(rx_data_length) | SQI_BDCTRL_PKTINTEN |
            SQI_BDCTRL_LASTPKT | SQI_BDCTRL_LASTBD |
            SQI_BDCTRL_DIR_READ |
            SQI_CHIP_SELECT | SQI_BDCTRL_DEASSERT |
            SQI_BDCTRL_DESCEN);
    sqiBufDesc[0].bd_bufaddr = (uint32_t *) KVA_TO_PA(&statusRegVal);
    sqiBufDesc[0].bd_stat = 0;
    sqiBufDesc[0].bd_nxtptr = NULL;

    dObj->gd5f1Plib->DMATransfer((sqi_dma_desc_t *) KVA_TO_PA(&sqiCmdDesc[0]));

    while (dObj->isTransferDone == false) {
        /* Wait for transfer to complete */
    }

    *status = statusRegVal;
    return true;
}

DRV_GD25Q_TRANSFER_STATUS DRV_GD25Q_TransferStatusGet(const DRV_HANDLE handle) {
    DRV_GD25Q_TRANSFER_STATUS status = DRV_GD25Q_TRANSFER_ERROR_UNKNOWN;

    if (handle == DRV_HANDLE_INVALID) {
        return status;
    }

    if (dObj->isTransferDone == true) {
        status = DRV_GD25Q_TRANSFER_COMPLETED;
    } else {
        status = DRV_GD25Q_TRANSFER_BUSY;
    }

    return status;
}

bool DRV_GD25Q_Read(const DRV_HANDLE handle, void *rx_data, uint32_t rx_data_length, uint32_t address) {
    uint32_t pendingBytes = rx_data_length;
    uint8_t *readBuffer = (uint8_t *) rx_data;
    uint32_t numBytes = 0;
    uint32_t i = 0;

    if (DRV_GD25Q_ValidateHandleAndCheckBusy(handle) == true) {
        return false;
    }
    if ((rx_data_length == 0U) || (rx_data_length > (DRV_GD25Q_PAGE_SIZE * DRV_GD25Q_BUFF_DESC_NUMBER))) {
        return false;
    }

    dObj->isTransferDone = false;

    // Construct parameters to issue read command
    sqi_cmd_1[0] = (uint8_t) GD25Q_CMD_QUAD_OUT_FAST_READ;
    sqi_cmd_1[1] = (uint8_t) (0xFFU & (address >> 16));
    sqi_cmd_1[2] = (uint8_t) (0xFFU & (address >> 8));
    sqi_cmd_1[3] = (uint8_t) (0xFFU & (address >> 0));
    sqiCmdDesc[0].bd_ctrl = (SQI_BDCTRL_BUFFLEN_VAL(4) |
            SQI_CHIP_SELECT | SQI_BDCTRL_DESCEN);
    sqiCmdDesc[0].bd_bufaddr = (uint32_t *) KVA_TO_PA(&sqi_cmd_1);
    sqiCmdDesc[0].bd_stat = 0;
    sqiCmdDesc[0].bd_nxtptr = (sqi_dma_desc_t *) KVA_TO_PA(&sqiCmdDesc[1]);

    sqi_cmd_2[0] = DUMMY_BYTE;
    sqiCmdDesc[1].bd_ctrl = (SQI_BDCTRL_BUFFLEN_VAL(1) |
            SQI_CHIP_SELECT | SQI_BDCTRL_DESCEN);
    sqiCmdDesc[1].bd_bufaddr = (uint32_t *) KVA_TO_PA(&sqi_cmd_2);
    sqiCmdDesc[1].bd_stat = 0;
    sqiCmdDesc[1].bd_nxtptr = (sqi_dma_desc_t *) KVA_TO_PA(&sqiBufDesc[0]);

    while (i < DRV_GD25Q_BUFF_DESC_NUMBER) {
        if (pendingBytes >= DRV_GD25Q_PAGE_SIZE) {
            numBytes = DRV_GD25Q_PAGE_SIZE;
        } else {
            numBytes = maxPowerOfTwo(pendingBytes);
        }
        sqiBufDesc[i].bd_ctrl = (SQI_BDCTRL_BUFFLEN_VAL(numBytes) | SQI_BDCTRL_PKTINTEN |
                DRV_SQI_LANE_MODE | SQI_BDCTRL_DIR_READ |
                SQI_CHIP_SELECT | SQI_BDCTRL_DESCEN);

        sqiBufDesc[i].bd_bufaddr = (uint32_t *) KVA_TO_PA(readBuffer);
        sqiBufDesc[i].bd_stat = 0;
        sqiBufDesc[i].bd_nxtptr = (sqi_dma_desc_t *) KVA_TO_PA(&sqiBufDesc[i + 1]);

        pendingBytes -= numBytes;
        readBuffer += numBytes;
        i++;
        if (pendingBytes == 0U) {
            break;
        }
    }

    /* The last descriptor must indicate the end of the descriptor list */
    sqiBufDesc[i - 1].bd_ctrl |= (SQI_BDCTRL_LASTPKT | SQI_BDCTRL_LASTBD |
            SQI_BDCTRL_DEASSERT);
    sqiBufDesc[i - 1].bd_nxtptr = NULL;

    dObj->gd5f1Plib->DMATransfer((sqi_dma_desc_t *) KVA_TO_PA(&sqiCmdDesc[0]));

    return true;
}

bool DRV_GD25Q_PageWrite(const DRV_HANDLE handle, void *tx_data, uint32_t address) {
    if (DRV_GD25Q_ValidateHandleAndCheckBusy(handle) == true) {
        return false;
    }

    dObj->isTransferDone = false;

    DRV_GD25Q_WriteEnable();

    // Construct parameters to issue page program command
    sqi_cmd_2[0] = (uint8_t) GD25Q_CMD_QUAD_PAGE_PROGRAM;
    sqi_cmd_2[1] = (uint8_t) (0xFFU & (address >> 16));
    sqi_cmd_2[2] = (uint8_t) (0xFFU & (address >> 8));
    sqi_cmd_2[3] = (uint8_t) (0xFFU & (address >> 0));

    sqiCmdDesc[1].bd_ctrl = (SQI_BDCTRL_BUFFLEN_VAL(4) |
            SQI_CHIP_SELECT | SQI_BDCTRL_DESCEN);
    sqiCmdDesc[1].bd_bufaddr = (uint32_t *) KVA_TO_PA(&sqi_cmd_2);
    sqiCmdDesc[1].bd_stat = 0;
    sqiCmdDesc[1].bd_nxtptr = (sqi_dma_desc_t *) KVA_TO_PA(&sqiBufDesc[0]);

    sqiBufDesc[0].bd_ctrl = (SQI_BDCTRL_BUFFLEN_VAL(DRV_GD25Q_PAGE_SIZE) | SQI_BDCTRL_PKTINTEN |
            SQI_BDCTRL_LASTPKT | SQI_BDCTRL_LASTBD |
            DRV_SQI_LANE_MODE | SQI_BDCTRL_SCHECK |
            SQI_CHIP_SELECT | SQI_BDCTRL_DEASSERT |
            SQI_BDCTRL_DESCEN);
    sqiBufDesc[0].bd_bufaddr = (uint32_t *) KVA_TO_PA((uint8_t*) tx_data);
    sqiBufDesc[0].bd_stat = 0;
    sqiBufDesc[0].bd_nxtptr = NULL;

    dObj->gd5f1Plib->DMATransfer((sqi_dma_desc_t *) KVA_TO_PA(&sqiCmdDesc[0]));

    return true;
}

bool DRV_GD25Q_ByteWrite(const DRV_HANDLE handle, void *tx_data, uint32_t address, uint32_t lenData) {
    if (DRV_GD25Q_ValidateHandleAndCheckBusy(handle) == true) {
        return false;
    }

    dObj->isTransferDone = false;

    DRV_GD25Q_WriteEnable();

    // Construct parameters to issue page program command
    sqi_cmd_2[0] = (uint8_t) GD25Q_CMD_QUAD_PAGE_PROGRAM;
    sqi_cmd_2[1] = (uint8_t) (0xFFU & (address >> 16));
    sqi_cmd_2[2] = (uint8_t) (0xFFU & (address >> 8));
    sqi_cmd_2[3] = (uint8_t) (0xFFU & (address >> 0));
    sqiCmdDesc[1].bd_ctrl = (SQI_BDCTRL_BUFFLEN_VAL(4) |
            SQI_CHIP_SELECT | SQI_BDCTRL_DESCEN);
    sqiCmdDesc[1].bd_bufaddr = (uint32_t *) KVA_TO_PA(&sqi_cmd_2);
    sqiCmdDesc[1].bd_stat = 0;
    sqiCmdDesc[1].bd_nxtptr = (sqi_dma_desc_t *) KVA_TO_PA(&sqiBufDesc[0]);

    sqiBufDesc[0].bd_ctrl = (SQI_BDCTRL_BUFFLEN_VAL(lenData) | SQI_BDCTRL_PKTINTEN |
            SQI_BDCTRL_LASTPKT | SQI_BDCTRL_LASTBD |
            DRV_SQI_LANE_MODE | SQI_BDCTRL_SCHECK |
            SQI_CHIP_SELECT | SQI_BDCTRL_DEASSERT |
            SQI_BDCTRL_DESCEN);
    sqiBufDesc[0].bd_bufaddr = (uint32_t *) KVA_TO_PA((uint8_t*) tx_data);
    sqiBufDesc[0].bd_stat = 0;
    sqiBufDesc[0].bd_nxtptr = NULL;

    dObj->gd5f1Plib->DMATransfer((sqi_dma_desc_t *) KVA_TO_PA(&sqiCmdDesc[0]));

    return true;
}

static bool DRV_GD25Q_Erase(uint8_t *instruction, uint32_t length) {
    dObj->isTransferDone = false;

    DRV_GD25Q_WriteEnable();

    sqiCmdDesc[1].bd_ctrl = (SQI_BDCTRL_BUFFLEN_VAL(length) | SQI_BDCTRL_PKTINTEN |
            SQI_BDCTRL_LASTPKT | SQI_BDCTRL_LASTBD |
            SQI_BDCTRL_SCHECK |
            SQI_CHIP_SELECT | SQI_BDCTRL_DEASSERT |
            SQI_BDCTRL_DESCEN);
    sqiCmdDesc[1].bd_bufaddr = (uint32_t *) KVA_TO_PA(instruction);
    sqiCmdDesc[1].bd_stat = 0;
    sqiCmdDesc[1].bd_nxtptr = NULL;

    dObj->gd5f1Plib->DMATransfer((sqi_dma_desc_t *) KVA_TO_PA(&sqiCmdDesc[0]));

    return true;
}

bool DRV_GD25Q_SectorErase(const DRV_HANDLE handle, uint32_t address) {
    if (DRV_GD25Q_ValidateHandleAndCheckBusy(handle) == true) {
        return false;
    }

    sqi_cmd_2[0] = (uint8_t) GD25Q_CMD_SECTOR_ERASE;
    sqi_cmd_2[1] = (uint8_t) (0xFFU & (address >> 16));
    sqi_cmd_2[2] = (uint8_t) (0xFFU & (address >> 8));
    sqi_cmd_2[3] = (uint8_t) (0xFFU & (address >> 0));

    return (DRV_GD25Q_Erase(&sqi_cmd_2[0], 4));
}

bool DRV_GD25Q_BulkErase(const DRV_HANDLE handle, uint32_t address) {
    if (DRV_GD25Q_ValidateHandleAndCheckBusy(handle) == true) {
        return false;
    }

    sqi_cmd_2[0] = (uint8_t) GD25Q_CMD_BULK_ERASE_64K;
    sqi_cmd_2[1] = (uint8_t) (0xFFU & (address >> 16));
    sqi_cmd_2[2] = (uint8_t) (0xFFU & (address >> 8));
    sqi_cmd_2[3] = (uint8_t) (0xFFU & (address >> 0));

    return (DRV_GD25Q_Erase(&sqi_cmd_2[0], 4));
}

bool DRV_GD25Q_ChipErase(const DRV_HANDLE handle) {
    if (DRV_GD25Q_ValidateHandleAndCheckBusy(handle) == true) {
        return false;
    }

    sqi_cmd_2[0] = (uint8_t) GD25Q_CMD_CHIP_ERASE;

    return (DRV_GD25Q_Erase(&sqi_cmd_2[0], 1));
}

bool DRV_GD25Q_GeometryGet(const DRV_HANDLE handle, DRV_GD25Q_GEOMETRY *geometry) {
    uint32_t flash_size = 0;
    bool status = true;

    if (DRV_GD25Q_ReadJedecId(handle, (void *) &jedecID) == false) {
        status = false;
    } else {
        SYS_CONSOLE_PRINT("SPIFLash ID: %x %x, %x, %x \n\r", jedecID[0], jedecID[1], jedecID[2], jedecID[3]);
        flash_size = DRV_GD25Q_GetFlashSize(jedecID[2]);

        if (flash_size == 0U) {
            status = false;
        }

        if (DRV_GD25Q_START_ADDRESS >= flash_size) {
            status = false;
        } else {
            flash_size = flash_size - DRV_GD25Q_START_ADDRESS;

            /* Flash size should be at-least of a Erase Block size */
            if (flash_size < DRV_GD25Q_ERASE_BUFFER_SIZE) {
                status = false;
            } else {
                /* Read block size and number of blocks */
                geometry->read_blockSize = 1;
                geometry->read_numBlocks = flash_size;

                /* Write block size and number of blocks */
                geometry->write_blockSize = DRV_GD25Q_PAGE_SIZE;
                geometry->write_numBlocks = (flash_size / DRV_GD25Q_PAGE_SIZE);

                /* Erase block size and number of blocks */
                geometry->erase_blockSize = DRV_GD25Q_ERASE_BUFFER_SIZE;
                geometry->erase_numBlocks = (flash_size / DRV_GD25Q_ERASE_BUFFER_SIZE);

                geometry->numReadRegions = 1;
                geometry->numWriteRegions = 1;
                geometry->numEraseRegions = 1;

                geometry->blockStartAddress = DRV_GD25Q_START_ADDRESS;
            }
        }
    }

    return status;
}

DRV_HANDLE DRV_GD25Q_Open(const SYS_MODULE_INDEX drvIndex, const DRV_IO_INTENT ioIntent) {

    /* Reset GD5F1 Flash device */
    DRV_GD25Q_ResetFlash();

    /* Put GD5F1 Flash device on QUAD IO Mode */
    DRV_GD25Q_EnableQuadIO();

    if ((ioIntent & DRV_IO_INTENT_WRITE) == (DRV_IO_INTENT_WRITE)) {
        /* Unlock the Flash */
        if (DRV_GD25Q_UnlockFlash((DRV_HANDLE) drvIndex) == false) {
            return DRV_HANDLE_INVALID;
        }
    }

    DRV_GD25Q_ReadJedecId((DRV_HANDLE) drvIndex, (void *) & jedecID);
    SYS_CONSOLE_PRINT("SPIFLash ID: %x%x%x\n\r", jedecID[0], jedecID[1], jedecID[2]);
    if (jedecID[0] == 0xC8 && jedecID[1] != 0xFF && jedecID[2] != 0xFF)
        dObj->status = SYS_STATUS_READY;

    return ((DRV_HANDLE) drvIndex);
}

void DRV_GD25Q_Close(const DRV_HANDLE handle) {

}

/* MISRA C-2012 Rule 11.3, 11.8 deviated below. Deviation record ID -
   H3_MISRAC_2012_R_11_3_DR_1 & H3_MISRAC_2012_R_11_8_DR_1*/

SYS_MODULE_OBJ DRV_GD25Q_Initialize(const SYS_MODULE_INDEX drvIndex, const SYS_MODULE_INIT * const init) {
    DRV_GD25Q_INIT *gd25qInit = NULL;

    if (dObj->inUse == true)
        return SYS_MODULE_OBJ_INVALID;

    dObj->status = SYS_STATUS_UNINITIALIZED;
    dObj->inUse = true;
    gd25qInit = (DRV_GD25Q_INIT *) init;
    dObj->gd5f1Plib = gd25qInit->gd25qPlib;
    dObj->gd5f1Plib->RegisterCallback(DRV_GD25Q_EventHandler, (uintptr_t) dObj);
    dObj->isTransferDone = true;

    /* Return the driver index */
    return ( (SYS_MODULE_OBJ) drvIndex);
}

/* MISRAC 2012 deviation block end */

SYS_STATUS DRV_GD25Q_Status() {
    /* Return the driver status */
    return (gDrvGD5F1Obj.status);
}
