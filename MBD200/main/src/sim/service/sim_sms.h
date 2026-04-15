#ifndef SIM_SMS_H
#define	SIM_SMS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "definitions.h"

#ifdef	__cplusplus
extern "C" {
#endif

    typedef enum {
        SIM_SMS_IDLE = 0,
        SIM_SMS_SENDING,
        SIM_SMS_ERROR
    } SIM_SMS_STATE;

    typedef struct {
        char phoneNumber[24];
        char message[160]; 
    } SIM_SMS_MSG;

    void SIM_SMS_Initialize(void);
    void SIM_SMS_Process(void);
    bool SIM_SMS_IsReady(void);
    bool SIM_SMS_Send(const char* phoneNumber, const char* message);

#ifdef	__cplusplus
}
#endif

#endif	/* SIM_SMS_H */

