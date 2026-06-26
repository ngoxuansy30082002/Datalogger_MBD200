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

#define NTP_NET_TIMEOUT_MS      15000
#define NTP_NTP_TIMEOUT_MS      15000
#define NTP_MAX_RETRY           1

#define FTP_NET_TIMEOUT_MS      15000
#define FTP_UPLOAD_TIMEOUT_MS   60000
#define FTP_MAX_RETRY           1

#ifdef	__cplusplus
extern "C" {
#endif

    typedef enum {
        NET_OWNER_NONE,
        NET_OWNER_NTP,
        NET_OWNER_FTP,
    } SIM_NET_OWNER;

    typedef enum {
        NTP_ST_IDLE,
        NTP_ST_NET_START,
        NTP_ST_NET_WAIT,
        NTP_ST_NTP_START,
        NTP_ST_NTP_WAIT,
        NTP_ST_DONE,
        NTP_ST_ERROR,
    } SIM_MAIN_NTP_STATE;

    typedef struct {
        SIM_MAIN_NTP_STATE state;
        uint32_t timer;
        uint8_t retryCount;
        bool triggered;
    } SIM_MAIN_NTP_CONTEXT;

    typedef enum {
        FTP_ST_IDLE,
        FTP_ST_NET_START,
        FTP_ST_NET_WAIT,
        FTP_ST_FTP_START,
        FTP_ST_FTP_WAIT,
        FTP_ST_DONE,
        FTP_ST_ERROR,
    } SIM_MAIN_FTP_STATE;

    typedef struct {
        SIM_MAIN_FTP_STATE state;
        uint32_t timer;
        uint8_t retryCount;
        bool triggered;
        bool useFtp1;
        bool useFtp2;
    } SIM_MAIN_FTP_CONTEXT;


    void SIMMain_Initialize(void);
    void SIMMain_Task(void);

    void SIMMain_NTPTrigger(void);
    bool SIMMain_NTPIsBusy(void);
    bool SIMMain_NTPIsSuccess(void);
    bool SIMMain_NTPHasError(void);

    void SIMMain_FTPTrigger(bool ftp1, bool ftp2);
    bool SIMMain_FTPIsBusy(void);
    bool SIMMain_FTPIsSuccess(void);
    bool SIMMain_FTPHasError(void);
    bool SIMMain_FTPGetResult(bool * ftp1Success, bool * ftp2Success);


#ifdef	__cplusplus
}
#endif

#endif	/* SIM_MAIN_H */

