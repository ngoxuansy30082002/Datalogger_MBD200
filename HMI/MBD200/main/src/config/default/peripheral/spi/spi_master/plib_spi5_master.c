/*******************************************************************************
  SPI PLIB

  Company:
    Microchip Technology Inc.

  File Name:
    plib_spi5_master.c

  Summary:
    SPI5 Master Source File

  Description:
    This file has implementation of all the interfaces provided for particular
    SPI peripheral.

*******************************************************************************/

/*******************************************************************************
* Copyright (C) 2018-2019 Microchip Technology Inc. and its subsidiaries.
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

#include "plib_spi5_master.h"
#include "interrupts.h"

// *****************************************************************************
// *****************************************************************************
// Section: SPI5 Implementation
// *****************************************************************************
// *****************************************************************************

/* Global object to save SPI Exchange related data */
static volatile SPI_OBJECT spi5Obj;

#define SPI5_CON_MSTEN                      (1UL << _SPI5CON_MSTEN_POSITION)
#define SPI5_CON_CKP                        (1UL << _SPI5CON_CKP_POSITION)
#define SPI5_CON_CKE                        (0UL << _SPI5CON_CKE_POSITION)
#define SPI5_CON_MODE_32_MODE_16            (0UL << _SPI5CON_MODE16_POSITION)
#define SPI5_CON_ENHBUF                     (1UL << _SPI5CON_ENHBUF_POSITION)
#define SPI5_CON_MCLKSEL                    (0UL << _SPI5CON_MCLKSEL_POSITION)
#define SPI5_CON_MSSEN                      (0UL << _SPI5CON_MSSEN_POSITION)
#define SPI5_CON_SMP                        (0UL << _SPI5CON_SMP_POSITION)

void SPI5_Initialize ( void )
{
    uint32_t rdata = 0U;

    /* Disable SPI5 Interrupts */
    IEC5CLR = 0x10000;
    IEC5CLR = 0x20000;
    IEC5CLR = 0x40000;

    /* STOP and Reset the SPI */
    SPI5CON = 0;

    /* Clear the Receiver buffer */
    rdata = SPI5BUF;
    rdata = rdata;

    /* Clear SPI5 Interrupt flags */
    IFS5CLR = 0x10000;
    IFS5CLR = 0x20000;
    IFS5CLR = 0x40000;

    /* BAUD Rate register Setup */
    SPI5BRG = 1;

    /* CLear the Overflow */
    SPI5STATCLR = _SPI5STAT_SPIROV_MASK;

    /*
    MSTEN = 1
    CKP = 1
    CKE = 0
    MODE<32,16> = 0
    ENHBUF = 1
    MSSEN = 0
    MCLKSEL = 0
    */
    SPI5CONSET = (SPI5_CON_MSSEN | SPI5_CON_MCLKSEL | SPI5_CON_ENHBUF | SPI5_CON_MODE_32_MODE_16 | SPI5_CON_CKE | SPI5_CON_CKP | SPI5_CON_MSTEN | SPI5_CON_SMP);

    /* Enable transmit interrupt when transmit buffer is completely empty (STXISEL = '01') */
    /* Enable receive interrupt when the receive buffer is not empty (SRXISEL = '01') */
    SPI5CONSET = 0x00000005;

    /* Initialize global variables */
    spi5Obj.transferIsBusy = false;
    spi5Obj.callback = NULL;

    /* Enable SPI5 */
    SPI5CONSET = _SPI5CON_ON_MASK;
}

bool SPI5_TransferSetup (SPI_TRANSFER_SETUP* setup, uint32_t spiSourceClock )
{
    uint32_t t_brg;
    uint32_t baudHigh;
    uint32_t baudLow;
    uint32_t errorHigh;
    uint32_t errorLow;

    if ((setup == NULL) || (setup->clockFrequency == 0U))
    {
        return false;
    }

    if(spiSourceClock == 0U)
    {
        // Use Master Clock Frequency set in GUI
        spiSourceClock = 100000000;
    }

    t_brg = (((spiSourceClock / (setup->clockFrequency)) / 2u) - 1u);
    baudHigh = spiSourceClock / (2u * (t_brg + 1u));
    baudLow = spiSourceClock / (2u * (t_brg + 2u));
    errorHigh = baudHigh - setup->clockFrequency;
    errorLow = setup->clockFrequency - baudLow;

    if (errorHigh > errorLow)
    {
        t_brg++;
    }

    if(t_brg > 8191U)
    {
        return false;
    }

    SPI5BRG = t_brg;
    SPI5CON = (SPI5CON & (~(_SPI5CON_MODE16_MASK | _SPI5CON_MODE32_MASK | _SPI5CON_CKP_MASK | _SPI5CON_CKE_MASK))) |
                            ((uint32_t)setup->clockPolarity | (uint32_t)setup->clockPhase | (uint32_t)setup->dataBits);

    return true;
}

bool SPI5_Write(void* pTransmitData, size_t txSize)
{
    return(SPI5_WriteRead(pTransmitData, txSize, NULL, 0));
}

bool SPI5_Read(void* pReceiveData, size_t rxSize)
{
    return(SPI5_WriteRead(NULL, 0, pReceiveData, rxSize));
}

bool SPI5_IsTransmitterBusy (void)
{
    return ((SPI5STAT & _SPI5STAT_SRMT_MASK) == 0U)? true : false;
}

bool SPI5_WriteRead (void* pTransmitData, size_t txSize, void* pReceiveData, size_t rxSize)
{
    bool isRequestAccepted = false;

    /* Verify the request */
    if((spi5Obj.transferIsBusy == false) && (((txSize > 0U) && (pTransmitData != NULL)) || ((rxSize > 0U) && (pReceiveData != NULL))))
    {
        isRequestAccepted = true;
        spi5Obj.txBuffer = pTransmitData;
        spi5Obj.rxBuffer = pReceiveData;
        spi5Obj.rxCount = 0;
        spi5Obj.txCount = 0;
        spi5Obj.dummySize = 0;

        if (pTransmitData != NULL)
        {
            spi5Obj.txSize = txSize;
        }
        else
        {
            spi5Obj.txSize = 0;
        }

        if (pReceiveData != NULL)
        {
            spi5Obj.rxSize = rxSize;
        }
        else
        {
            spi5Obj.rxSize = 0;
        }

        spi5Obj.transferIsBusy = true;

        size_t txSz = spi5Obj.txSize;
        if (spi5Obj.rxSize > txSz)
        {
            spi5Obj.dummySize = spi5Obj.rxSize - txSz;
        }

        /* Clear the receive overflow error if any */
        SPI5STATCLR = _SPI5STAT_SPIROV_MASK;

        /* Make sure there is no data pending in the RX FIFO */
        /* Depending on 8/16/32 bit mode, there may be 16/8/4 bytes in the FIFO */
        while ((SPI5STAT & _SPI5STAT_SPIRBE_MASK) == 0U)
        {
            (void)SPI5BUF;
        }

        /* Configure SPI to generate receive interrupt when receive buffer is empty (SRXISEL = '01') */
        SPI5CONCLR = _SPI5CON_SRXISEL_MASK;
        SPI5CONSET = 0x00000001;

        /* Configure SPI to generate transmit interrupt when the transmit shift register is empty (STXISEL = '00')*/
        SPI5CONCLR = _SPI5CON_STXISEL_MASK;

        /* Disable the receive interrupt */
        IEC5CLR = 0x20000;

        /* Disable the transmit interrupt */
        IEC5CLR = 0x40000;

        /* Clear the receive interrupt flag */
        IFS5CLR = 0x20000;

        /* Clear the transmit interrupt flag */
        IFS5CLR = 0x40000;

        /* Start the first write here itself, rest will happen in ISR context */
        if((_SPI5CON_MODE32_MASK) == (SPI5CON & (_SPI5CON_MODE32_MASK)))
        {
            spi5Obj.txSize >>= 2;
            spi5Obj.dummySize >>= 2;
            spi5Obj.rxSize >>= 2;

            txSz = spi5Obj.txSize;
            if(spi5Obj.txCount < txSz)
            {
                SPI5BUF = *((uint32_t*)spi5Obj.txBuffer);
                spi5Obj.txCount++;
            }
            else if (spi5Obj.dummySize > 0U)
            {
                SPI5BUF = (uint32_t)(0xffU);
                spi5Obj.dummySize--;
            }
            else
            {
                /* Nothing to process */
            }
        }
        else if((_SPI5CON_MODE16_MASK) == (SPI5CON & (_SPI5CON_MODE16_MASK)))
        {
            spi5Obj.txSize >>= 1;
            spi5Obj.dummySize >>= 1;
            spi5Obj.rxSize >>= 1;

            txSz = spi5Obj.txSize;
            if (spi5Obj.txCount < txSz)
            {
                SPI5BUF = *((uint16_t*)spi5Obj.txBuffer);
                spi5Obj.txCount++;
            }
            else if (spi5Obj.dummySize > 0U)
            {
                SPI5BUF = (uint16_t)(0xffU);
                spi5Obj.dummySize--;
            }
            else
            {
                /* Nothing to process */
            }
        }
        else
        {
            if (spi5Obj.txCount < txSz)
            {
                SPI5BUF = *((uint8_t*)spi5Obj.txBuffer);
                spi5Obj.txCount++;
            }
            else if (spi5Obj.dummySize > 0U)
            {
                SPI5BUF = (uint8_t)(0xffU);
                spi5Obj.dummySize--;
            }
            else
            {
                /* Nothing to process */
            }
        }

        if (rxSize > 0U)
        {
            /* Enable receive interrupt to complete the transfer in ISR context.
             * Keep the transmit interrupt disabled. Transmit interrupt will be
             * enabled later if txCount < txSize, when rxCount = rxSize.
             */
            IEC5SET = 0x20000;
        }
        else
        {
            if (spi5Obj.txCount != txSz)
            {
                /* Configure SPI to generate transmit buffer empty interrupt only if more than
                 * data is pending (STXISEL = '01')  */
                SPI5CONSET = 0x00000004;
            }
            /* Enable transmit interrupt to complete the transfer in ISR context */
            IEC5SET = 0x40000;
        }
    }

    return isRequestAccepted;
}

bool SPI5_IsBusy (void)
{
    uint32_t StatRead = SPI5STAT;
    return (((spi5Obj.transferIsBusy) != false) || (( StatRead & _SPI5STAT_SRMT_MASK) == 0U));
}

void SPI5_CallbackRegister (SPI_CALLBACK callback, uintptr_t context)
{
    spi5Obj.callback = callback;

    spi5Obj.context = context;
}

void __attribute__((used)) SPI5_RX_InterruptHandler (void)
{
    uint32_t receivedData = 0;

    /* Check if the receive buffer is empty or not */
    if ((SPI5STAT & _SPI5STAT_SPIRBE_MASK) == 0U)
    {
        /* Receive buffer is not empty. Read the received data. */
        receivedData = SPI5BUF;

        size_t rxCount = spi5Obj.rxCount;
        size_t txCount = spi5Obj.txCount;
        if (rxCount < spi5Obj.rxSize)
        {
            /* Copy the received data to the user buffer */
            if((_SPI5CON_MODE32_MASK) == (SPI5CON & (_SPI5CON_MODE32_MASK)))
            {
                ((uint32_t*)spi5Obj.rxBuffer)[rxCount] = receivedData;
                rxCount++;
            }
            else if((_SPI5CON_MODE16_MASK) == (SPI5CON & (_SPI5CON_MODE16_MASK)))
            {
                ((uint16_t*)spi5Obj.rxBuffer)[rxCount] = (uint16_t)receivedData;
                rxCount++;
            }
            else
            {
                ((uint8_t*)spi5Obj.rxBuffer)[rxCount] = (uint8_t)receivedData;
                rxCount++;
            }

            spi5Obj.rxCount = rxCount;

            if (rxCount == spi5Obj.rxSize)
            {
                if (txCount < spi5Obj.txSize)
                {
                    /* Reception of all bytes is complete. However, there are few more
                     * bytes to be transmitted as txCount != txSize. Finish the
                     * transmission of the remaining bytes from the transmit interrupt. */

                    /* Disable the receive interrupt */
                    IEC5CLR = 0x20000;

                    /* Generate TX interrupt when buffer is completely empty (STXISEL = '00') */
                    SPI5CONCLR = _SPI5CON_STXISEL_MASK;
                    SPI5CONSET = 0x00000004;

                    /* Enable the transmit interrupt. Callback will be given from the
                     * transmit interrupt, when all bytes are shifted out. */
                    IEC5SET = 0x40000;
                }
            }
        }
        if (rxCount < spi5Obj.rxSize)
        {
            /* More bytes pending to be received .. */
            if((_SPI5CON_MODE32_MASK) == (SPI5CON & (_SPI5CON_MODE32_MASK)))
            {
                if (txCount < spi5Obj.txSize)
                {
                    SPI5BUF = ((uint32_t*)spi5Obj.txBuffer)[txCount];
                    txCount++;
                }
                else if (spi5Obj.dummySize > 0U)
                {
                    SPI5BUF = (uint32_t)(0xffU);
                    spi5Obj.dummySize--;
                }
                else
                {
                    /* Do Nothing */
                }
            }
            else if((_SPI5CON_MODE16_MASK) == (SPI5CON & (_SPI5CON_MODE16_MASK)))
            {
                if (txCount < spi5Obj.txSize)
                {
                    SPI5BUF = ((uint16_t*)spi5Obj.txBuffer)[txCount];
                    txCount++;
                }
                else if (spi5Obj.dummySize > 0U)
                {
                    SPI5BUF = (uint16_t)(0xffU);
                    spi5Obj.dummySize--;
                }
                else
                {
                    /* Do Nothing */
                }
            }
            else
            {
                if (txCount < spi5Obj.txSize)
                {
                    SPI5BUF = ((uint8_t*)spi5Obj.txBuffer)[txCount];
                    txCount++;
                }
                else if (spi5Obj.dummySize > 0U)
                {
                    SPI5BUF = (uint8_t)(0xffU);
                    spi5Obj.dummySize--;
                }
                else
                {
                    /* Do Nothing */
                }
            }
            spi5Obj.txCount = txCount;
        }
        else
        {
            if(rxCount == spi5Obj.rxSize)
            {
                if (txCount == spi5Obj.txSize)
                {
                    /* Clear receiver overflow error if any */
                    SPI5STATCLR = _SPI5STAT_SPIROV_MASK;

                    /* Disable receive interrupt */
                    IEC5CLR = 0x20000;

                    /* Transfer complete. Give a callback */
                    spi5Obj.transferIsBusy = false;

                    if(spi5Obj.callback != NULL)
                    {
                        uintptr_t context = spi5Obj.context;
                        spi5Obj.callback(context);
                    }
                }
            }
        }
    }

    /* Clear SPI5 RX Interrupt flag */
    /* This flag should cleared only after reading buffer */
    IFS5CLR = 0x20000;
}

void __attribute__((used)) SPI5_TX_InterruptHandler (void)
{
    /* If there are more words to be transmitted, then transmit them here and keep track of the count */
    if((SPI5STAT & _SPI5STAT_SPITBE_MASK) == _SPI5STAT_SPITBE_MASK)
    {
        size_t txCount = spi5Obj.txCount;
        if (txCount < spi5Obj.txSize)
        {
            if((_SPI5CON_MODE32_MASK) == (SPI5CON & (_SPI5CON_MODE32_MASK)))
            {
                SPI5BUF = ((uint32_t*)spi5Obj.txBuffer)[txCount];
                txCount++;
            }
            else if((_SPI5CON_MODE16_MASK) == (SPI5CON & (_SPI5CON_MODE16_MASK)))
            {
                SPI5BUF = ((uint16_t*)spi5Obj.txBuffer)[txCount];
                txCount++;
            }
            else
            {
                SPI5BUF = ((uint8_t*)spi5Obj.txBuffer)[txCount];
                txCount++;
            }

            spi5Obj.txCount = txCount;

            if (txCount == spi5Obj.txSize)
            {
                /* All bytes are submitted to the SPI module. Now, enable transmit
                 * interrupt when the shift register is empty (STXISEL = '00')*/
                SPI5CONCLR = _SPI5CON_STXISEL_MASK;
            }
        }
        else if (txCount == spi5Obj.txSize)
        {
            if ((SPI5STAT & _SPI5STAT_SRMT_MASK) != 0U)
            {
                /* This part of code is executed when the shift register is empty. */

                /* Clear receiver overflow error if any */
                SPI5STATCLR = _SPI5STAT_SPIROV_MASK;

                /* Disable transmit interrupt */
                IEC5CLR = 0x40000;

                /* Transfer complete. Give a callback */
                spi5Obj.transferIsBusy = false;

                if(spi5Obj.callback != NULL)
                {
                    uintptr_t context = spi5Obj.context;
                    spi5Obj.callback(context);
                }
            }
        }
        else
        {
            /* Do Nothing */
        }
    }
    /* Clear the transmit interrupt flag */
    IFS5CLR = 0x40000;
}

