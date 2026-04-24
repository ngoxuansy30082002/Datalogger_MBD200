#include "ethernet/eth_net.h"
#include "definitions.h"
#include "tcpip/tcpip.h"         
#include "tcpip/tcpip_manager.h" 

static bool _isNetworkReady = false; 
static uint32_t _printTimer = 0; 

void EthNet_Initialize(void) {
    _isNetworkReady = false;
    _printTimer = 0;
    SYS_CONSOLE_PRINT("\r\nEthernet init\r\n");
}

bool EthNet_IsReady(void) {
    return _isNetworkReady;
}

void EthNet_Process(void) {
    TCPIP_NET_HANDLE netH = TCPIP_STACK_IndexToNet(0);

    if (netH != NULL && TCPIP_STACK_NetIsReady(netH)) {
        IPV4_ADDR ipAddr;
        ipAddr.Val = TCPIP_STACK_NetAddress(netH);

        if (ipAddr.Val != 0) {
            if (!_isNetworkReady) { 
                SYS_CONSOLE_PRINT("\r\neth: link up, dhcp bound\r\n");
                SYS_CONSOLE_PRINT("eth: ip address %d.%d.%d.%d\r\n", 
                                  ipAddr.v[0], ipAddr.v[1], ipAddr.v[2], ipAddr.v[3]);
                _isNetworkReady = true; 
            }
        } else {
            uint32_t curTick = SYS_TMR_TickCountGet();
            if (curTick - _printTimer >= (SYS_TMR_TickCounterFrequencyGet() * 2)) {
                SYS_CONSOLE_PRINT("eth: link up, requesting dhcp...\r\n");
                _printTimer = curTick;
            }
        }
    } else {
        if (_isNetworkReady) { 
            SYS_CONSOLE_PRINT("\r\neth: link down\r\n");
            _isNetworkReady = false; 
        } else {
            uint32_t curTick = SYS_TMR_TickCountGet();
            if (curTick - _printTimer >= (SYS_TMR_TickCounterFrequencyGet() * 2)) {
                SYS_CONSOLE_PRINT("eth: waiting for link...\r\n");
                _printTimer = curTick;
            }
        }
    }
}