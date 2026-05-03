/* 
 * File:   consoleLogs.h
 * Author: Syxn
 *
 * Created on November 24, 2024, 11:07 AM
 */

#ifndef CONSOLELOGS_H
#define	CONSOLELOGS_H

#include <stdio.h>
#include <string.h>
#include "definitions.h"

#define CONSOLE_MAX_CONTENT_LEN         256
#define CONSOLE_MAX_ENTRY               20               

#ifdef	__cplusplus
extern "C" {
#endif

    typedef enum {
        CONSOLE_INFO,
        CONSOLE_SUCCESS,
        CONSOLE_WARNING,
        CONSOLE_ERROR
    } CONSOLE_TYPE;

    typedef struct {
        CONSOLE_TYPE type;
        char content[CONSOLE_MAX_CONTENT_LEN];
        uint8_t len;
        char timestamp[26];
    } CONSOLE_ENTRY;

    typedef struct {
        CONSOLE_ENTRY entry[CONSOLE_MAX_ENTRY];
        int front;
        int rear;
        int size;
    } CONSOLE_QUEUE_T;

    typedef struct {
        CONSOLE_QUEUE_T queue;
    } CONSOLE_DATA;

    void CONSOLE_LOGS_Initialize(void);
    bool ConsoleLos_Push(char* content, int len, CONSOLE_TYPE type);
    bool ConsoleLos_Get(char* content, size_t content_size, char* type, size_t type_size,char* timestamp);

    extern CONSOLE_DATA csLogsDt;

#ifdef	__cplusplus
}
#endif

#endif	/* CONSOLELOGS_H */

