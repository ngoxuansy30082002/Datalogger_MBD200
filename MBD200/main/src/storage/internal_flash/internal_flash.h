/* 
 * File:   internal_flash.h
 * Author: LENOVO
 *
 * Created on July 24, 2025, 10:25 AM
 */

#ifndef INTERNAL_FLASH_H
#define	INTERNAL_FLASH_H

#include <stdio.h>
#include <string.h>
#include "definitions.h"

#define VA_OF_DEVICE_INFO               0 // virtual address (in dee 1 address is 1 word)

#ifdef	__cplusplus
extern "C" {
#endif

    bool InFlash_Initialize(void);
    bool InFlash_SaveDeviceInfo(uint8_t *devInfo, uint16_t size);
    bool InFlash_LoadDeviceInfo(uint8_t *devInfo, uint16_t size);

#ifdef	__cplusplus
}
#endif

#endif	/* INTERNAL_FLASH_H */

