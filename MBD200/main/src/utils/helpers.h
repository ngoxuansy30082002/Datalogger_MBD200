/* 
 * File:   helpers.h
 * Author: Syxn
 *
 * Created on January 4, 2025, 4:11 PM
 */

#ifndef HELPERS_H
#define	HELPERS_H

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "definitions.h"

#ifdef	__cplusplus
extern "C" {
#endif


    uint8_t Helpers_Make8(uint16_t var, uint8_t offset);
    uint16_t Helpers_Make16(uint8_t high, uint8_t low);
    uint8_t Helpers_HexFromChars(char high, char low);

#ifdef	__cplusplus
}
#endif

#endif	/* HELPERS_H */

