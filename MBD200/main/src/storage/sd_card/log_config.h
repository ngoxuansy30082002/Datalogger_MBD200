/* 
 * File:   log_config.h
 * Author: LENOVO
 *
 * Created on April 9, 2026, 8:27 PM
 */

#ifndef LOG_CONFIG_H
#define	LOG_CONFIG_H

#include <stdio.h>
#include <string.h>
#include <string.h>
#include <stdarg.h>
#include "definitions.h"
#include <time.h>

#define LOG_CONFIG_DIRECTORY         "/History"
#define LOG_CONFIG_BUFFER_SIZE       8192  


#ifdef	__cplusplus
extern "C" {
#endif

    typedef enum {
        LOG_CONFIG_POWER_UP = 0,
        LOG_CONFIG_WEB,
        LOG_CONFIG_SDCARD_IMPORT,
        LOG_CONFIG_MODBUSTCP,
    } LOG_CONFIG_AUTHOR;

    void Logger_SaveFullConfig(LOG_CONFIG_AUTHOR author);

#ifdef	__cplusplus
}
#endif

#endif	/* LOG_CONFIG_H */

