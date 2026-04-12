#include "fram.h"
#include "drv_fm25cl.h"

static const char * __TAG__ = "FRAM";

static SYS_STATUS _initStatus = 0;
static FRAM_STATES _writeState = 0;
static FRAM_STATES _readState = 0;

static iqueue_t _writeQueue;
static iqueue_t _readQueue;
static uint8_t _writeQueueBuffer[FRAM_QUEUE_SIZE * sizeof (FRAM_QUEUE_ITEM)];
static uint8_t _readQueueBuffer[FRAM_QUEUE_SIZE * sizeof (FRAM_QUEUE_ITEM)];
static uint8_t _framData[2048] = {0};

static const FRAM_PARTITION _partition[FRAM_DATA_COUNT] = {
    {},
    {.type = FRAM_DATA_COUNTER, .address = FRAM_ADDR_COUNTER, .maxSize = 16},
};

static uint16_t _packFramData(const FRAM_QUEUE_ITEM * item) {
    uint8_t headerSize = sizeof (FRAM_METADATA);
    const FRAM_PARTITION * part = &_partition[item->type];
    if ((item->size + headerSize) > part->maxSize) return 0;

    FRAM_METADATA header = {
        .type = item->type,
        .len = item->size,
    };
    header.crc = Helpers_CRC32Calculate((uint8_t *) item->buffer, item->size);
    memcpy(_framData, &header, headerSize);
    memcpy(&_framData[headerSize], item->buffer, item->size);

    return (item->size + headerSize);
}

static bool _unpackFramData(const FRAM_QUEUE_ITEM * item) {
    FRAM_METADATA header;
    uint8_t headerSize = sizeof (FRAM_METADATA);

    memcpy(&header, _framData, headerSize);
    if (header.type != item->type || header.len != item->size) return false;

    uint32_t crc = Helpers_CRC32Calculate((uint8_t *) & _framData[headerSize], header.len);
    if (header.crc != crc) return false;
    
    memcpy(item->buffer, &_framData[headerSize], item->size);

    return true;
}

void Fram_Initialize(void) {
    DrvFM25CL_Initialize();
    _initStatus = SYS_STATUS_UNINITIALIZED;
    iqueue_init(&_writeQueue, FRAM_QUEUE_SIZE, sizeof (FRAM_QUEUE_ITEM), &_writeQueueBuffer);
    iqueue_init(&_readQueue, FRAM_QUEUE_SIZE, sizeof (FRAM_QUEUE_ITEM), &_readQueueBuffer);
}

void Fram_Task(void) {
    static FRAM_QUEUE_ITEM item;
    static uint32_t timeoutTick = 0;
    static uint32_t openTick = 0;
    static FRAM_RESULT rslt = 0;

    if (_initStatus != SYS_STATUS_READY && _initStatus != SYS_STATUS_ERROR) {
        if (!TIME_IS_EXPIRED(openTick, 100))
            return;

        openTick = TICK_NOW();
        _initStatus = DrvFM25CL_Open();
        if (_initStatus == SYS_STATUS_READY)
            SYS_CONSOLE_PRINT("%s - %s\t Init SUCCESS\r\n", __TAG__, __func__);
        else if (_initStatus == SYS_STATUS_ERROR)
            SYS_CONSOLE_PRINT("%s - %s\t Init FAIL\r\n", __TAG__, __func__);

        return;
    }

    switch (_writeState) {
        case FRAM_IDLE:
        {
            size_t wQueueSize;
            iqueue_size(&_writeQueue, &wQueueSize);
            if (wQueueSize == 0 || _readState != FRAM_IDLE) break;

            if (iqueue_dequeue(&_writeQueue, &item) == I_OK) {
                timeoutTick = TICK_NOW();
                rslt = FRAM_RES_SUCCESS;
                _writeState = FRAM_TRANSFER;
            }
            break;
        }
        case FRAM_TRANSFER:
        {
            if (TIME_IS_EXPIRED(timeoutTick, 500)) {
                _writeState = FRAM_DONE;
                rslt = FRAM_RES_TIMEOUT;
                break;
            }

            if (DrvFM25CL_GetTransferStatus() == DRV_FM25CL_TRANSFER_BUSY) break;

            const FRAM_PARTITION * part = &_partition[item.type];
            uint16_t size = _packFramData(&item);
            bool res = false;
            if (size > 0) res = DrvFM25CL_writeMultiRegister(part->address, _framData, size);
            if (!res) {
                _writeState = FRAM_DONE;
                rslt = FRAM_RES_FAIL;
                break;
            }

            timeoutTick = TICK_NOW();
            _writeState = FRAM_WAIT_TRANSFER;
            break;
        }
        case FRAM_WAIT_TRANSFER:
        {
            if (TIME_IS_EXPIRED(timeoutTick, 500)) {
                _writeState = FRAM_DONE;
                rslt = FRAM_RES_TIMEOUT;
                break;
            }

            if (DrvFM25CL_GetTransferStatus() == DRV_FM25CL_TRANSFER_BUSY) break;

            _writeState = FRAM_DONE;
            break;
        }
        case FRAM_DONE:
        {
            //            SYS_CONSOLE_PRINT("write done: %u\r\n", rslt);
            if (item.callback) item.callback(item.type, rslt);
            _writeState = FRAM_IDLE;
            break;
        }
    }


    switch (_readState) {
        case FRAM_IDLE:
        {
            size_t queueSize;
            iqueue_size(&_readQueue, &queueSize);
            if (queueSize == 0 || _writeState != FRAM_IDLE) break;

            if (iqueue_dequeue(&_readQueue, &item) == I_OK) {
                timeoutTick = TICK_NOW();
                rslt = FRAM_RES_SUCCESS;
                _readState = FRAM_TRANSFER;
            }
            break;
        }
        case FRAM_TRANSFER:
        {
            const FRAM_PARTITION * part = &_partition[item.type];
            if (!DrvFM25CL_readMultiRegister(part->address, _framData, item.size + sizeof (FRAM_METADATA))) {
                _readState = FRAM_DONE;
                rslt = FRAM_RES_FAIL;
                break;
            }

            timeoutTick = TICK_NOW();
            _readState = FRAM_WAIT_TRANSFER;
            break;
        }
        case FRAM_WAIT_TRANSFER:
        {
            if (TIME_IS_EXPIRED(timeoutTick, 500)) {
                _readState = FRAM_DONE;
                rslt = FRAM_RES_TIMEOUT;
                break;
            }

            if (DrvFM25CL_GetTransferStatus() == DRV_FM25CL_TRANSFER_BUSY) break;

            bool res = _unpackFramData(&item);
            if (!res) rslt = FRAM_RES_FAIL;

            _readState = FRAM_DONE;
            break;
        }
        case FRAM_DONE:
        {
            //            SYS_CONSOLE_PRINT("read done: %u\r\n", rslt);
            if (item.callback) item.callback(item.type, rslt);
            _readState = FRAM_IDLE;
            break;
        }
    }
}

bool Fram_SaveBlockData(FRAM_DATA_TYPE type, void * buffer, uint16_t size, void (*clb)(int type, int rlst)) {
    if (type < 0 || type >= FRAM_DATA_COUNT || !buffer || size == 0) return false;
    const FRAM_PARTITION * part = &_partition[type];
    if (part->maxSize < size) return false;

    FRAM_QUEUE_ITEM item = {
        .type = type,
        .buffer = buffer,
        .size = size,
        .callback = clb
    };
    return (iqueue_enqueue(&_writeQueue, &item) == I_OK);
}

bool Fram_LoadBlockData(FRAM_DATA_TYPE type, void * buffer, uint16_t size, void (*clb)(int type, int rlst)) {
    if (type < 0 || type >= FRAM_DATA_COUNT || !buffer || size == 0) return false;
    const FRAM_PARTITION * part = &_partition[type];
    if (part->maxSize < size) return false;

    FRAM_QUEUE_ITEM item = {
        .type = type,
        .buffer = buffer,
        .size = size,
        .callback = clb
    };
    return (iqueue_enqueue(&_readQueue, &item) == I_OK);
}
