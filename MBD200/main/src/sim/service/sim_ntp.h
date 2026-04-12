/* 
 * File:   sim_ntp.h
 * Author: LENOVO
 *
 * Created on March 31, 2026, 8:21 PM
 */

#ifndef SIM_NTP_H
#define	SIM_NTP_H

#ifdef	__cplusplus
extern "C" {
#endif

    typedef enum {
        SIM_NTP_IDLE = 0,
        SIM_NTP_SETUP_TIMEZONE,
        SIM_NTP_SYNC_TIME,
        SIM_NTP_READY,
        SIM_NTP_ERROR
    } SIM_NTP_STATE;


#ifdef	__cplusplus
}
#endif

#endif	/* SIM_NTP_H */

