#ifndef ETH_NET_H
#define	ETH_NET_H

#include <stdint.h>
#include <stdbool.h>

#ifdef	__cplusplus
extern "C" {
#endif

    void EthNet_Initialize(void);
    
    void EthNet_Process(void);
    
    bool EthNet_IsReady(void);

#ifdef	__cplusplus
}
#endif

#endif	/* ETH_NET_H */