#include "drv_spi_adc.h"

static uint8_t _idxAdcDevice = 0;
static volatile bool _transferDone = false;

static const DRV_ADC_PLIB _adcPlib = {
    .writeRead = (DRV_ADC_WRITE_READ) SPI6_WriteRead,
    .read_t = (DRV_ADC_READ) SPI6_Read,
    .write_t = (DRV_ADC_WRITE) SPI6_Write,
    .isBusy = (DRV_ADC_IS_BUSY) SPI6_IsBusy,
    .callbackRegister = (DRV_ADC_CALLBACK_REGISTER) SPI6_CallbackRegister,

    .csPin = GPIO_PIN_RD14,
};

void _spiCallbackHandler(uintptr_t context) {
    if (_idxAdcDevice == 0)
        GPIO_PinSet(_adcPlib.csPin);
    else if (_idxAdcDevice == 1)
        GPIO_PinClear(_adcPlib.csPin);

    _transferDone = true;
}

void DrvSpiAdc_Initialize(void) {
    _adcPlib.callbackRegister(_spiCallbackHandler, (uintptr_t) NULL);
}

bool DrvSpiAdc_WriteRead(void * txData, size_t txSz, void * rxData, size_t rxSz, uint8_t idxAdcDevice) {
    if (idxAdcDevice == 0)
        GPIO_PinClear(_adcPlib.csPin);
    else if (idxAdcDevice == 1)
        GPIO_PinSet(_adcPlib.csPin);

    _idxAdcDevice = idxAdcDevice;
    bool res = _adcPlib.writeRead(txData, txSz, rxData, rxSz);

    if (res) {
        _transferDone = false;
        int count = 65535;
        while (_adcPlib.isBusy() && !_transferDone)
            if (--count < 0) break;
    }

    return res;
}
