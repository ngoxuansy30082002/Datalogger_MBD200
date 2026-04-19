/* 
 * File:   sim_net.h
 * Author: LENOVO
 *
 * Created on March 31, 2026, 8:19 PM
 */

#ifndef SIM_NET_H
#define	SIM_NET_H

#ifdef	__cplusplus
extern "C" {
#endif

    typedef enum {
        SIM_NET_IDLE = 0,
        SIM_NET_DEFINE_PDP,
        SIM_NET_DEACTIVE_PDP,
        SIM_NET_ACTIVE_PDP,
        SIM_NET_CHECK_ACTIVE,
        SIM_NET_DEACTIVE_PDP_STOP,
        SIM_NET_READY,
        SIM_NET_ERROR,
        SIM_NET_COUNT
    } SIM_NET_STATE;

    bool SIMNet_Start(bool restart);
    void SIMNet_Stop(void);
    void SIMNet_Process(void);
    bool SIMNet_IsReady(void);
    bool SIMNet_HasError(void);


#ifdef	__cplusplus
}
#endif

#endif	/* SIM_NET_H */

