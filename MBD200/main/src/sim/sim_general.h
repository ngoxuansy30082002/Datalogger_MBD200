/* 
 * File:   sim_general.h
 * Author: LENOVO
 *
 * Created on March 31, 2026, 8:19 PM
 */

#ifndef SIM_GENERAL_H
#define	SIM_GENERAL_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define SIM_TRANSFER_BUFF_SIZE          4096
#define SIM_TRANSFER_GAP_TIME           100 //ms
#define SIM_CONTEXT_ID                  1
#define SIM_MQTT_CLIENT_ID              0
#define SIM_FTP_TIMEOUT                 40
#define SIM_FTP_FILE_LEN                4096
#define SIM_FTP_PATH_LEN                256

#ifdef	__cplusplus
extern "C" {
#endif

    typedef int (*SIM_CMD_BUILDER)(int state, char* buffer, size_t maxLen, const char* format);
    typedef bool (*SIM_CMD_PARSER)(int state, char* buffer, size_t maxLen);

    typedef struct {
        const char* cmd;
        SIM_CMD_BUILDER builderFunc;
        const char* respOk;
        const char* respFail;
        uint32_t timeoutMs;
        uint8_t attempts;
        SIM_CMD_PARSER parserFunc;
        int nextStateOk;
        int nextStateFail;
    } SIM_CMD_SEQ;

#ifdef	__cplusplus
}
#endif

#endif	/* SIM_GENERAL_H */

