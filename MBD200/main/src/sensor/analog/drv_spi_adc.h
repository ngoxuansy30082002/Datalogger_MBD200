/* 
 * File:   drv_spi_adc.h
 * Author: LENOVO
 *
 * Created on May 23, 2026, 9:30 PM
 */

#ifndef DRV_SPI_ADC_H
#define	DRV_SPI_ADC_H

#include <stdio.h>
#include <string.h>
#include "peripheral/gpio/plib_gpio.h"
#include "peripheral/spi/spi_master/plib_spi6_master.h"

#ifdef	__cplusplus
extern "C" {
#endif

    typedef void (* DRV_ADC_CALLBACK)(uintptr_t context);
    typedef bool (* DRV_ADC_WRITE_READ)(void* pTransmitData, size_t txSize, void *pReceiveData, size_t rxSize);
    typedef bool (* DRV_ADC_WRITE)(void* pTransmitData, size_t txSize);
    typedef bool (* DRV_ADC_READ)(void* pReceiveData, size_t rxSize);
    typedef bool (* DRV_ADC_IS_BUSY)(void);
    typedef void (* DRV_ADC_CALLBACK_REGISTER)(DRV_ADC_CALLBACK callback, uintptr_t context);

    typedef struct {
        DRV_ADC_WRITE_READ writeRead;
        DRV_ADC_WRITE write_t;
        DRV_ADC_READ read_t;
        DRV_ADC_IS_BUSY isBusy;
        DRV_ADC_CALLBACK_REGISTER callbackRegister;

        GPIO_PIN csPin;
    } DRV_ADC_PLIB;

    void DrvSpiAdc_Initialize(void);
    bool DrvSpiAdc_WriteRead(void * txData, size_t txSz, void * rxData, size_t rxSz, uint8_t idxAdcDevice);

#ifdef	__cplusplus
}
#endif

#endif	/* DRV_SPI_ADC_H */

