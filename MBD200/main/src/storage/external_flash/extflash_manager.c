#include "extflash_manager.h"

static const char * __TAG__ = "EXTFL";

static EXTFL_WRITE_STATE _writeState = EXTFL_WRITE_IDLE;
static EXTFL_READ_STATE _readState = EXTFL_READ_IDLE;

static iqueue_t _writeQueue;
static iqueue_t _readQueue;
static uint8_t _writeQueueStorage[EXTFL_QUEUE_SIZE * sizeof (EXTFL_QUEUE_ENTRY)];
static uint8_t _readQueueStorage[EXTFL_QUEUE_SIZE * sizeof (EXTFL_QUEUE_ENTRY)];

static DRV_HANDLE _driverHandle;
static bool _openStatus = 0;
static uint32_t _currentAddrFlash = 0;
static CACHE_ALIGN uint8_t _transferBuffer[EXTFL_PAGE_SIZE * 8];


static const EXTFL_PARTITION _partition[EXTFL_NUM_PARTITION] = {

};

static uint32_t _maxPowerOfTwo(uint32_t num) {
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

static bool _eraseSectorFlash(uint32_t address) {
    return DRV_SST26_SectorErase(_driverHandle, address);
}

static bool _transferIsBusy() {
    return (DRV_SST26_TransferStatusGet(_driverHandle) != DRV_SST26_TRANSFER_COMPLETED);
}

static bool _readFlash(uint32_t address, uint32_t len) {
    return DRV_SST26_Read(_driverHandle, _transferBuffer, len, address);
}

static uint16_t _calculateDataSize(EXTFL_DATA_TYPE dataType) {
    switch (dataType) {

        default: return 0;
    }
}

static bool _writeStreamByteToFlash(uint8_t *pData, uint32_t wLen, bool *finish) {

    enum {
        WRITE_STATE_IDLE = 0,
        WRITE_STATE_FIRST_PART,
        WRITE_STATE_FIRST_PART_WAIT,
        WRITE_STATE_BLOCK,
        WRITE_STATE_BLOCK_WAIT,
        WRITE_STATE_LAST_PART,
        WRITE_STATE_LAST_PART_WAIT,
    };

    static int writeStreamState = WRITE_STATE_IDLE;
    static uint16_t remainingLen = 0;
    static uint8_t *pWriteData = NULL;
    static uint16_t numByteFirst = 0;
    static uint16_t numBlocks = 0;
    static uint32_t chunkSize = 0;
    static bool syncLock = false;
    bool rtn = true;

    if (wLen == 0u || !pData) return false;

    if (wLen > 0 && !syncLock) {
        SYS_CONSOLE_PRINT("wlen: %u\r\n", wLen);

        SYS_CONSOLE_PRINT("pData     : %x %x %x %x\r\n", pData[0], pData[1], pData[2], pData[3]);
        pWriteData = pData;
        remainingLen = wLen;

        numByteFirst = EXTFL_PAGE_SIZE - (_currentAddrFlash % EXTFL_PAGE_SIZE);
        if (numByteFirst > remainingLen) numByteFirst = remainingLen;
        remainingLen -= numByteFirst;

        numBlocks = remainingLen / EXTFL_PAGE_SIZE;

        writeStreamState = (numByteFirst > 0) ? WRITE_STATE_FIRST_PART :
                (numBlocks > 0) ? WRITE_STATE_BLOCK :
                WRITE_STATE_LAST_PART;

        syncLock = true;
    }

    switch (writeStreamState) {
        case WRITE_STATE_IDLE:
            SYS_CONSOLE_PRINT("w %u\r\n", 10);
            syncLock = false;
            *finish = true;
            return rtn;
            break;

        case WRITE_STATE_FIRST_PART:
            chunkSize = _maxPowerOfTwo(numByteFirst);
            rtn = DRV_SST26_ByteWrite(_driverHandle, pWriteData,
                    (uint32_t) _currentAddrFlash, chunkSize);
            SYS_CONSOLE_PRINT(" %u: %u", 0, chunkSize);
            writeStreamState = WRITE_STATE_FIRST_PART_WAIT;
            break;

        case WRITE_STATE_FIRST_PART_WAIT:
            SYS_CONSOLE_PRINT(" %u", 1);
            if (_transferIsBusy()) break;

            _currentAddrFlash += chunkSize;
            pWriteData += chunkSize;
            numByteFirst -= chunkSize;
            if (numByteFirst > 0) {
                writeStreamState = WRITE_STATE_FIRST_PART;
            } else if (numBlocks > 0) {
                writeStreamState = WRITE_STATE_BLOCK;
            } else if (remainingLen > 0) {
                writeStreamState = WRITE_STATE_LAST_PART;
            } else {
                writeStreamState = WRITE_STATE_IDLE;
            }
            break;

        case WRITE_STATE_BLOCK:
            SYS_CONSOLE_PRINT(" %u", 2);
            rtn = DRV_SST26_PageWrite(_driverHandle, pWriteData,
                    (uint32_t) _currentAddrFlash);
            writeStreamState = WRITE_STATE_BLOCK_WAIT;
            break;

        case WRITE_STATE_BLOCK_WAIT:
            SYS_CONSOLE_PRINT(" %u", 3);
            if (_transferIsBusy()) break;

            _currentAddrFlash += EXTFL_PAGE_SIZE;
            pWriteData += EXTFL_PAGE_SIZE;
            remainingLen -= EXTFL_PAGE_SIZE;
            numBlocks--;
            if (numBlocks > 0) {
                writeStreamState = WRITE_STATE_BLOCK;
            } else if (remainingLen > 0) {
                writeStreamState = WRITE_STATE_LAST_PART;
            } else {
                writeStreamState = WRITE_STATE_IDLE;
            }
            break;

        case WRITE_STATE_LAST_PART:
            chunkSize = _maxPowerOfTwo(remainingLen);
            rtn = DRV_SST26_ByteWrite(_driverHandle, pWriteData,
                    (uint32_t) _currentAddrFlash, chunkSize);
            SYS_CONSOLE_PRINT(" %u: %u", 4, chunkSize);
            writeStreamState = WRITE_STATE_LAST_PART_WAIT;
            break;

        case WRITE_STATE_LAST_PART_WAIT:
            SYS_CONSOLE_PRINT(" %u", 5);
            if (_transferIsBusy()) break;

            _currentAddrFlash += chunkSize;
            pWriteData += chunkSize;
            remainingLen -= chunkSize;
            if (remainingLen > 0) {
                writeStreamState = WRITE_STATE_LAST_PART;
            } else {
                writeStreamState = WRITE_STATE_IDLE;
            }
            break;
    }

    *finish = false;
    return rtn;
}

static bool _unpackBufferLoaded(void * buffer, uint16_t bufferSize, EXTFL_DATA_TYPE dataType) {
    switch (dataType) {
        default: false;
    }

    return true;
}

static bool _packBufferSave(void * buffer, uint16_t maxBufferSize, EXTFL_DATA_TYPE dataType) {
    switch (dataType) {
        default:
            return false;
    }
    return true;
}

void ExtFlash_Initialize() {
    _writeState = EXTFL_WRITE_IDLE;
    _readState = EXTFL_READ_IDLE;
    _currentAddrFlash = 0;
    iqueue_init(&_writeQueue, EXTFL_QUEUE_SIZE, sizeof (EXTFL_QUEUE_ENTRY), &_writeQueueStorage);
    iqueue_init(&_readQueue, EXTFL_QUEUE_SIZE, sizeof (EXTFL_QUEUE_ENTRY), &_readQueueStorage);
}

void ExtFlash_Task() {
    static EXTFL_QUEUE_ENTRY qEntry;
    static uint32_t timeoutTick = 0;
    static uint32_t openTick = 0;
    static EXTFL_RESULT rslt = 0;

    uint32_t currentTick = SYS_TMR_TickCountGet();
    uint32_t tickPerSecond = SYS_TMR_TickCounterFrequencyGet();

    if (!_openStatus) {
        if (currentTick - openTick < tickPerSecond / 100)
            return;

        _openStatus = DRV_SST26_IsOpened();
        if (_openStatus)
            SYS_CONSOLE_PRINT("%s - %s\t Init SUCCESS\r\n", __TAG__, __func__);
        else
            SYS_CONSOLE_PRINT("%s - %s\t Init FAIL\r\n", __TAG__, __func__);

        return;
    }

    switch (_writeState) {
        case EXTFL_WRITE_IDLE:
        {
            size_t queueSize;
            iqueue_size(&_writeQueue, &queueSize);
            if (queueSize == 0) break;

            if (iqueue_dequeue(&_writeQueue, &qEntry) == I_OK) {
                timeoutTick = currentTick;
                memset(_transferBuffer, 0, sizeof (_transferBuffer));
                if (_packBufferSave(_transferBuffer, sizeof (_transferBuffer), qEntry.type)) {
                    DCACHE_CLEAN_BY_ADDR((uint32_t) _transferBuffer, qEntry.size);
                    _writeState = EXTFL_WRITE_ERASE_SECTOR;
                }
            }
            break;
        }

        case EXTFL_WRITE_ERASE_SECTOR:
        {
            if (currentTick - timeoutTick > (tickPerSecond / 1000 * EXTFL_TIMEOUT_TRANSFER)) {
                _writeState = EXTFL_WRITE_DONE;
                rslt = EXTFL_TRANSFER_TIMEOUT;
                break;
            }
            if (_transferIsBusy()) break;

            _currentAddrFlash = _partition[qEntry.type].startAddress;
            if (!_eraseSectorFlash(_currentAddrFlash)) {
                _writeState = EXTFL_WRITE_DONE;
                rslt = EXTFL_ERASE_FAIL;
                break;
            }

            timeoutTick = currentTick;
            _writeState = EXTFL_WRITE_TRANSFER;
            break;
        }
        case EXTFL_WRITE_TRANSFER:
        {
            bool finish;
            if (currentTick - timeoutTick > (tickPerSecond / 1000 * EXTFL_TIMEOUT_TRANSFER)) {
                _writeState = EXTFL_WRITE_DONE;
                rslt = EXTFL_TRANSFER_TIMEOUT;
                break;
            }
            if (_transferIsBusy()) break;

            if (!_writeStreamByteToFlash(_transferBuffer, qEntry.size, &finish)) {
                _writeState = EXTFL_WRITE_DONE;
                rslt = EXTFL_WRITE_FAIL;
                break;
            }
            if (!finish) break;

            rslt = EXTFL_SUCCESS;
            _writeState = EXTFL_WRITE_DONE;
            break;
        }
        case EXTFL_WRITE_DONE:
        {
            SYS_CONSOLE_PRINT("write done: %u\r\n", rslt);
            if (qEntry.callback) qEntry.callback(qEntry.type, rslt);
            _writeState = EXTFL_WRITE_IDLE;
            break;
        }
    }


    size_t wQueueSize;
    iqueue_size(&_writeQueue, &wQueueSize);
    if (_writeState != EXTFL_WRITE_IDLE || wQueueSize != 0) return;

    switch (_readState) {
        case EXTFL_READ_IDLE:
        {
            size_t queueSize;
            iqueue_size(&_readQueue, &queueSize);
            if (queueSize == 0) break;

            if (iqueue_dequeue(&_readQueue, &qEntry) == I_OK) {
                timeoutTick = currentTick;
                _readState = EXTFL_READ_TRANSFER;
            }
            break;
        }
        case EXTFL_READ_TRANSFER:
        {
            uint32_t address = _partition[qEntry.type].startAddress;
            if (!_readFlash(address, qEntry.size)) {
                _readState = EXTFL_READ_DONE;
                rslt = EXTFL_READ_FAIL;
                break;
            }

            timeoutTick = currentTick;
            _readState = EXTFL_READ_WAIT_TRANSFER;
            break;
        }
        case EXTFL_READ_WAIT_TRANSFER:
        {
            if (currentTick - timeoutTick > (tickPerSecond / 1000 * EXTFL_TIMEOUT_TRANSFER)) {
                _readState = EXTFL_READ_DONE;
                rslt = EXTFL_TRANSFER_TIMEOUT;
                break;
            }
            if (_transferIsBusy()) break;

            _readState = EXTFL_READ_VERIFY_DATA;
            break;
        }
        case EXTFL_READ_VERIFY_DATA:
        {
            bool res = false;
            res = _unpackBufferLoaded(_transferBuffer, qEntry.size, qEntry.type);

            if (!res) rslt = EXTFL_VERIFY_FAIL;
            else rslt = EXTFL_SUCCESS;
            _readState = EXTFL_READ_DONE;
            break;
        }
        case EXTFL_READ_DONE:
        {
            SYS_CONSOLE_PRINT("read done: %u\r\n", rslt);
            if (qEntry.callback) qEntry.callback(qEntry.type, rslt);
            _readState = EXTFL_WRITE_IDLE;
            break;
        }
    }
}

bool ExtFlash_SaveConfig(EXTFL_DATA_TYPE dataType, void (*clb)(int type, int rlst)) {
    EXTFL_QUEUE_ENTRY entry = {
        .type = dataType,
        .callback = clb,
        .size = _calculateDataSize(dataType)
    };

    return (iqueue_enqueue(&_writeQueue, &entry) == I_OK);
}

bool ExtFlash_LoadConfig(EXTFL_DATA_TYPE dataType, void (*clb)(int type, int rlst)) {
    EXTFL_QUEUE_ENTRY entry = {
        .type = dataType,
        .callback = clb,
        .size = _calculateDataSize(dataType)
    };

    return (iqueue_enqueue(&_readQueue, &entry) == I_OK);
}