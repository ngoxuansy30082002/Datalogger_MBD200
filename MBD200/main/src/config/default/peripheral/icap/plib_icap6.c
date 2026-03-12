/*******************************************************************************
  Input Capture (ICAP6) Peripheral Library (PLIB)

  Company:
    Microchip Technology Inc.

  File Name:
    plib_icap6.c

  Summary:
    ICAP6 Source File

  Description:
    None

*******************************************************************************/

/*******************************************************************************
* Copyright (C) 2019 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/
#include "plib_icap6.h"
#include "interrupts.h"

static volatile ICAP_OBJECT icap6Obj;
// *****************************************************************************

// *****************************************************************************
// Section: ICAP6 Implementation
// *****************************************************************************
// *****************************************************************************


void ICAP6_Initialize (void)
{
    /*Setup IC6CON    */
    /*ICM     = 3        */
    /*ICI     = 0        */
    /*ICTMR = 0*/
    /*C32     = 0        */
    /*FEDGE = 0        */
    /*SIDL     = false    */

    IC6CON = 0x3;


        IEC0SET = _IEC0_IC6IE_MASK;
}


void ICAP6_Enable (void)
{
    IC6CONSET = _IC6CON_ON_MASK;
}


void ICAP6_Disable (void)
{
    IC6CONCLR = _IC6CON_ON_MASK;
}

uint16_t ICAP6_CaptureBufferRead (void)
{
    return (uint16_t)IC6BUF;
}



void ICAP6_CallbackRegister(ICAP_CALLBACK callback, uintptr_t context)
{
    icap6Obj.callback = callback;
    icap6Obj.context = context;
}

void __attribute__((used)) INPUT_CAPTURE_6_InterruptHandler(void)
{
    uintptr_t context = icap6Obj.context;
    if( (icap6Obj.callback != NULL))
    {
        icap6Obj.callback(context);
    }
    IFS0CLR = _IFS0_IC6IF_MASK;    //Clear IRQ flag

}


bool ICAP6_ErrorStatusGet (void)
{
    bool status = false;
    status = (((IC6CON >> ICAP_STATUS_OVERFLOW) & 0x1U) != 0U);
    return status;
}
