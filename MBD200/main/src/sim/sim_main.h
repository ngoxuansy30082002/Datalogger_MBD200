/* 
 * File:   sim_main.h
 * Author: LENOVO
 *
 * Created on April 4, 2026, 11:38 AM
 */

#ifndef SIM_MAIN_H
#define	SIM_MAIN_H

#include <stdio.h>
#include <string.h>
#include "definitions.h"

#ifdef	__cplusplus
extern "C" {
#endif


    void SIMMain_Initialize(void);
    void SIMMain_Task(void);

#ifdef	__cplusplus
}
#endif

#endif	/* SIM_MAIN_H */

