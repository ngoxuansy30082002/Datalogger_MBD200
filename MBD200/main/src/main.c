#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include "definitions.h"
#include "tcpip/tcpip.h"
#include "tcpip/tcpip_manager.h"

int main(void) {
    SYS_Initialize(NULL);

    bool isNetworkReady = false; 

    while (true) {
        SYS_Tasks();
        HMIDwin_Tasks();

        TCPIP_NET_HANDLE netH = TCPIP_STACK_IndexToNet(0);

        if (netH != NULL && TCPIP_STACK_NetIsReady(netH)) {
            
            IPV4_ADDR ipAddr;
            ipAddr.Val = TCPIP_STACK_NetAddress(netH);

            if (ipAddr.Val != 0) {
                if (!isNetworkReady) {
                    SYS_CONSOLE_PRINT("\r\n================================\r\n");
                    SYS_CONSOLE_PRINT("[ETH] LAN CONNECTED!\r\n");
                    SYS_CONSOLE_PRINT("[ETH] IP Address: %d.%d.%d.%d\r\n", 
                                      ipAddr.v[0], ipAddr.v[1], ipAddr.v[2], ipAddr.v[3]);
                    SYS_CONSOLE_PRINT("================================\r\n\r\n");
                    isNetworkReady = true;
                }
            }
        } else {
            if (isNetworkReady) {
                SYS_CONSOLE_PRINT("\r\n[ETH] MAT KET NOI LAN (Rut cap hoac rot mang)!\r\n");
                isNetworkReady = false;
            }
        }
    }
    return (EXIT_FAILURE);
}