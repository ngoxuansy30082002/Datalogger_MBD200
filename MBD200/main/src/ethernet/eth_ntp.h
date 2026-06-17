/* 
 * File:   eth_ntp.h
 * Author: LENOVO
 *
 * Created on June 6, 2026, 8:51 AM
 */

#ifndef ETH_NTP_H
#define	ETH_NTP_H

#include <stdlib.h>
#include "definitions.h"

#ifdef	__cplusplus
extern "C" {
#endif

    typedef enum {
        ETH_NTP_IDLE = 0,
        ETH_NTP_INIT,
        ETH_NTP_PARSE_TIME
    } ETH_NTP_STATE;

    void EthNtp_Initialize(void);
    void EthNtp_Task(void);
    void EthNtp_TriggerUpdate(void);

#ifdef	__cplusplus
}
#endif

#endif	/* ETH_NTP_H */

