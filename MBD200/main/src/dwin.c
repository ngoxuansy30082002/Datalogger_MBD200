#include "dwin.h"
#include "definitions.h"

void DWIN_WriteInt(uint16_t vp_address, int16_t value)
{
    uint8_t dwin_frame[8];

    dwin_frame[0] = 0x5A;
    dwin_frame[1] = 0xA5;
    dwin_frame[2] = 0x05; 
    dwin_frame[3] = 0x82; 

    dwin_frame[4] = (uint8_t)((vp_address >> 8) & 0xFF);
    dwin_frame[5] = (uint8_t)(vp_address & 0xFF);

    dwin_frame[6] = (uint8_t)((value >> 8) & 0xFF);
    dwin_frame[7] = (uint8_t)(value & 0xFF);

    while(UART6_WriteIsBusy() == true)
    {
    }

    UART6_Write((void*)dwin_frame, 8);
}