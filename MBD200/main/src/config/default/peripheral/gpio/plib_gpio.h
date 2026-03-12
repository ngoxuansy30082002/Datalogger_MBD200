/*******************************************************************************
  GPIO PLIB

  Company:
    Microchip Technology Inc.

  File Name:
    plib_gpio.h UUUUUUUUU

  Summary:
    GPIO PLIB Header File

  Description:
    This library provides an interface to control and interact with Parallel
    Input/Output controller (GPIO) module.

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

#ifndef PLIB_GPIO_H
#define PLIB_GPIO_H

#include <device.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Data types and constants
// *****************************************************************************
// *****************************************************************************


/*** Macros for GPIO_RC2 pin ***/
#define GPIO_RC2_Set()               (LATCSET = (1U<<2))
#define GPIO_RC2_Clear()             (LATCCLR = (1U<<2))
#define GPIO_RC2_Toggle()            (LATCINV= (1U<<2))
#define GPIO_RC2_OutputEnable()      (TRISCCLR = (1U<<2))
#define GPIO_RC2_InputEnable()       (TRISCSET = (1U<<2))
#define GPIO_RC2_Get()               ((PORTC >> 2) & 0x1U)
#define GPIO_RC2_GetLatch()          ((LATC >> 2) & 0x1U)
#define GPIO_RC2_PIN                  GPIO_PIN_RC2

/*** Macros for GPIO_RC3 pin ***/
#define GPIO_RC3_Set()               (LATCSET = (1U<<3))
#define GPIO_RC3_Clear()             (LATCCLR = (1U<<3))
#define GPIO_RC3_Toggle()            (LATCINV= (1U<<3))
#define GPIO_RC3_OutputEnable()      (TRISCCLR = (1U<<3))
#define GPIO_RC3_InputEnable()       (TRISCSET = (1U<<3))
#define GPIO_RC3_Get()               ((PORTC >> 3) & 0x1U)
#define GPIO_RC3_GetLatch()          ((LATC >> 3) & 0x1U)
#define GPIO_RC3_PIN                  GPIO_PIN_RC3

/*** Macros for GPIO_RA0 pin ***/
#define GPIO_RA0_Set()               (LATASET = (1U<<0))
#define GPIO_RA0_Clear()             (LATACLR = (1U<<0))
#define GPIO_RA0_Toggle()            (LATAINV= (1U<<0))
#define GPIO_RA0_OutputEnable()      (TRISACLR = (1U<<0))
#define GPIO_RA0_InputEnable()       (TRISASET = (1U<<0))
#define GPIO_RA0_Get()               ((PORTA >> 0) & 0x1U)
#define GPIO_RA0_GetLatch()          ((LATA >> 0) & 0x1U)
#define GPIO_RA0_PIN                  GPIO_PIN_RA0

/*** Macros for GPIO_RB4 pin ***/
#define GPIO_RB4_Set()               (LATBSET = (1U<<4))
#define GPIO_RB4_Clear()             (LATBCLR = (1U<<4))
#define GPIO_RB4_Toggle()            (LATBINV= (1U<<4))
#define GPIO_RB4_OutputEnable()      (TRISBCLR = (1U<<4))
#define GPIO_RB4_InputEnable()       (TRISBSET = (1U<<4))
#define GPIO_RB4_Get()               ((PORTB >> 4) & 0x1U)
#define GPIO_RB4_GetLatch()          ((LATB >> 4) & 0x1U)
#define GPIO_RB4_PIN                  GPIO_PIN_RB4

/*** Macros for GPIO_RA9 pin ***/
#define GPIO_RA9_Set()               (LATASET = (1U<<9))
#define GPIO_RA9_Clear()             (LATACLR = (1U<<9))
#define GPIO_RA9_Toggle()            (LATAINV= (1U<<9))
#define GPIO_RA9_OutputEnable()      (TRISACLR = (1U<<9))
#define GPIO_RA9_InputEnable()       (TRISASET = (1U<<9))
#define GPIO_RA9_Get()               ((PORTA >> 9) & 0x1U)
#define GPIO_RA9_GetLatch()          ((LATA >> 9) & 0x1U)
#define GPIO_RA9_PIN                  GPIO_PIN_RA9

/*** Macros for GPIO_RA10 pin ***/
#define GPIO_RA10_Set()               (LATASET = (1U<<10))
#define GPIO_RA10_Clear()             (LATACLR = (1U<<10))
#define GPIO_RA10_Toggle()            (LATAINV= (1U<<10))
#define GPIO_RA10_OutputEnable()      (TRISACLR = (1U<<10))
#define GPIO_RA10_InputEnable()       (TRISASET = (1U<<10))
#define GPIO_RA10_Get()               ((PORTA >> 10) & 0x1U)
#define GPIO_RA10_GetLatch()          ((LATA >> 10) & 0x1U)
#define GPIO_RA10_PIN                  GPIO_PIN_RA10

/*** Macros for GPIO_RB8 pin ***/
#define GPIO_RB8_Set()               (LATBSET = (1U<<8))
#define GPIO_RB8_Clear()             (LATBCLR = (1U<<8))
#define GPIO_RB8_Toggle()            (LATBINV= (1U<<8))
#define GPIO_RB8_OutputEnable()      (TRISBCLR = (1U<<8))
#define GPIO_RB8_InputEnable()       (TRISBSET = (1U<<8))
#define GPIO_RB8_Get()               ((PORTB >> 8) & 0x1U)
#define GPIO_RB8_GetLatch()          ((LATB >> 8) & 0x1U)
#define GPIO_RB8_PIN                  GPIO_PIN_RB8

/*** Macros for GPIO_RA1 pin ***/
#define GPIO_RA1_Set()               (LATASET = (1U<<1))
#define GPIO_RA1_Clear()             (LATACLR = (1U<<1))
#define GPIO_RA1_Toggle()            (LATAINV= (1U<<1))
#define GPIO_RA1_OutputEnable()      (TRISACLR = (1U<<1))
#define GPIO_RA1_InputEnable()       (TRISASET = (1U<<1))
#define GPIO_RA1_Get()               ((PORTA >> 1) & 0x1U)
#define GPIO_RA1_GetLatch()          ((LATA >> 1) & 0x1U)
#define GPIO_RA1_PIN                  GPIO_PIN_RA1

/*** Macros for GPIO_RD14 pin ***/
#define GPIO_RD14_Set()               (LATDSET = (1U<<14))
#define GPIO_RD14_Clear()             (LATDCLR = (1U<<14))
#define GPIO_RD14_Toggle()            (LATDINV= (1U<<14))
#define GPIO_RD14_OutputEnable()      (TRISDCLR = (1U<<14))
#define GPIO_RD14_InputEnable()       (TRISDSET = (1U<<14))
#define GPIO_RD14_Get()               ((PORTD >> 14) & 0x1U)
#define GPIO_RD14_GetLatch()          ((LATD >> 14) & 0x1U)
#define GPIO_RD14_PIN                  GPIO_PIN_RD14

/*** Macros for GPIO_RA15 pin ***/
#define GPIO_RA15_Set()               (LATASET = (1U<<15))
#define GPIO_RA15_Clear()             (LATACLR = (1U<<15))
#define GPIO_RA15_Toggle()            (LATAINV= (1U<<15))
#define GPIO_RA15_OutputEnable()      (TRISACLR = (1U<<15))
#define GPIO_RA15_InputEnable()       (TRISASET = (1U<<15))
#define GPIO_RA15_Get()               ((PORTA >> 15) & 0x1U)
#define GPIO_RA15_GetLatch()          ((LATA >> 15) & 0x1U)
#define GPIO_RA15_PIN                  GPIO_PIN_RA15

/*** Macros for GPIO_RD9 pin ***/
#define GPIO_RD9_Set()               (LATDSET = (1U<<9))
#define GPIO_RD9_Clear()             (LATDCLR = (1U<<9))
#define GPIO_RD9_Toggle()            (LATDINV= (1U<<9))
#define GPIO_RD9_OutputEnable()      (TRISDCLR = (1U<<9))
#define GPIO_RD9_InputEnable()       (TRISDSET = (1U<<9))
#define GPIO_RD9_Get()               ((PORTD >> 9) & 0x1U)
#define GPIO_RD9_GetLatch()          ((LATD >> 9) & 0x1U)
#define GPIO_RD9_PIN                  GPIO_PIN_RD9

/*** Macros for GPIO_RC14 pin ***/
#define GPIO_RC14_Set()               (LATCSET = (1U<<14))
#define GPIO_RC14_Clear()             (LATCCLR = (1U<<14))
#define GPIO_RC14_Toggle()            (LATCINV= (1U<<14))
#define GPIO_RC14_OutputEnable()      (TRISCCLR = (1U<<14))
#define GPIO_RC14_InputEnable()       (TRISCSET = (1U<<14))
#define GPIO_RC14_Get()               ((PORTC >> 14) & 0x1U)
#define GPIO_RC14_GetLatch()          ((LATC >> 14) & 0x1U)
#define GPIO_RC14_PIN                  GPIO_PIN_RC14

/*** Macros for GPIO_RD12 pin ***/
#define GPIO_RD12_Set()               (LATDSET = (1U<<12))
#define GPIO_RD12_Clear()             (LATDCLR = (1U<<12))
#define GPIO_RD12_Toggle()            (LATDINV= (1U<<12))
#define GPIO_RD12_OutputEnable()      (TRISDCLR = (1U<<12))
#define GPIO_RD12_InputEnable()       (TRISDSET = (1U<<12))
#define GPIO_RD12_Get()               ((PORTD >> 12) & 0x1U)
#define GPIO_RD12_GetLatch()          ((LATD >> 12) & 0x1U)
#define GPIO_RD12_PIN                  GPIO_PIN_RD12

/*** Macros for GPIO_RD13 pin ***/
#define GPIO_RD13_Set()               (LATDSET = (1U<<13))
#define GPIO_RD13_Clear()             (LATDCLR = (1U<<13))
#define GPIO_RD13_Toggle()            (LATDINV= (1U<<13))
#define GPIO_RD13_OutputEnable()      (TRISDCLR = (1U<<13))
#define GPIO_RD13_InputEnable()       (TRISDSET = (1U<<13))
#define GPIO_RD13_Get()               ((PORTD >> 13) & 0x1U)
#define GPIO_RD13_GetLatch()          ((LATD >> 13) & 0x1U)
#define GPIO_RD13_PIN                  GPIO_PIN_RD13

/*** Macros for GPIO_RG14 pin ***/
#define GPIO_RG14_Set()               (LATGSET = (1U<<14))
#define GPIO_RG14_Clear()             (LATGCLR = (1U<<14))
#define GPIO_RG14_Toggle()            (LATGINV= (1U<<14))
#define GPIO_RG14_OutputEnable()      (TRISGCLR = (1U<<14))
#define GPIO_RG14_InputEnable()       (TRISGSET = (1U<<14))
#define GPIO_RG14_Get()               ((PORTG >> 14) & 0x1U)
#define GPIO_RG14_GetLatch()          ((LATG >> 14) & 0x1U)
#define GPIO_RG14_PIN                  GPIO_PIN_RG14

/*** Macros for GPIO_RG12 pin ***/
#define GPIO_RG12_Set()               (LATGSET = (1U<<12))
#define GPIO_RG12_Clear()             (LATGCLR = (1U<<12))
#define GPIO_RG12_Toggle()            (LATGINV= (1U<<12))
#define GPIO_RG12_OutputEnable()      (TRISGCLR = (1U<<12))
#define GPIO_RG12_InputEnable()       (TRISGSET = (1U<<12))
#define GPIO_RG12_Get()               ((PORTG >> 12) & 0x1U)
#define GPIO_RG12_GetLatch()          ((LATG >> 12) & 0x1U)
#define GPIO_RG12_PIN                  GPIO_PIN_RG12

/*** Macros for GPIO_RG13 pin ***/
#define GPIO_RG13_Set()               (LATGSET = (1U<<13))
#define GPIO_RG13_Clear()             (LATGCLR = (1U<<13))
#define GPIO_RG13_Toggle()            (LATGINV= (1U<<13))
#define GPIO_RG13_OutputEnable()      (TRISGCLR = (1U<<13))
#define GPIO_RG13_InputEnable()       (TRISGSET = (1U<<13))
#define GPIO_RG13_Get()               ((PORTG >> 13) & 0x1U)
#define GPIO_RG13_GetLatch()          ((LATG >> 13) & 0x1U)
#define GPIO_RG13_PIN                  GPIO_PIN_RG13

/*** Macros for GPIO_RE2 pin ***/
#define GPIO_RE2_Set()               (LATESET = (1U<<2))
#define GPIO_RE2_Clear()             (LATECLR = (1U<<2))
#define GPIO_RE2_Toggle()            (LATEINV= (1U<<2))
#define GPIO_RE2_OutputEnable()      (TRISECLR = (1U<<2))
#define GPIO_RE2_InputEnable()       (TRISESET = (1U<<2))
#define GPIO_RE2_Get()               ((PORTE >> 2) & 0x1U)
#define GPIO_RE2_GetLatch()          ((LATE >> 2) & 0x1U)
#define GPIO_RE2_PIN                  GPIO_PIN_RE2

/*** Macros for GPIO_RE3 pin ***/
#define GPIO_RE3_Set()               (LATESET = (1U<<3))
#define GPIO_RE3_Clear()             (LATECLR = (1U<<3))
#define GPIO_RE3_Toggle()            (LATEINV= (1U<<3))
#define GPIO_RE3_OutputEnable()      (TRISECLR = (1U<<3))
#define GPIO_RE3_InputEnable()       (TRISESET = (1U<<3))
#define GPIO_RE3_Get()               ((PORTE >> 3) & 0x1U)
#define GPIO_RE3_GetLatch()          ((LATE >> 3) & 0x1U)
#define GPIO_RE3_PIN                  GPIO_PIN_RE3

/*** Macros for GPIO_RE4 pin ***/
#define GPIO_RE4_Set()               (LATESET = (1U<<4))
#define GPIO_RE4_Clear()             (LATECLR = (1U<<4))
#define GPIO_RE4_Toggle()            (LATEINV= (1U<<4))
#define GPIO_RE4_OutputEnable()      (TRISECLR = (1U<<4))
#define GPIO_RE4_InputEnable()       (TRISESET = (1U<<4))
#define GPIO_RE4_Get()               ((PORTE >> 4) & 0x1U)
#define GPIO_RE4_GetLatch()          ((LATE >> 4) & 0x1U)
#define GPIO_RE4_PIN                  GPIO_PIN_RE4


// *****************************************************************************
/* GPIO Port

  Summary:
    Identifies the available GPIO Ports.

  Description:
    This enumeration identifies the available GPIO Ports.

  Remarks:
    The caller should not rely on the specific numbers assigned to any of
    these values as they may change from one processor to the next.

    Not all ports are available on all devices.  Refer to the specific
    device data sheet to determine which ports are supported.
*/


#define    GPIO_PORT_A  (0)
#define    GPIO_PORT_B  (1)
#define    GPIO_PORT_C  (2)
#define    GPIO_PORT_D  (3)
#define    GPIO_PORT_E  (4)
#define    GPIO_PORT_F  (5)
#define    GPIO_PORT_G  (6)
typedef uint32_t GPIO_PORT;

typedef enum
{
    GPIO_INTERRUPT_ON_MISMATCH,
    GPIO_INTERRUPT_ON_RISING_EDGE,
    GPIO_INTERRUPT_ON_FALLING_EDGE,
    GPIO_INTERRUPT_ON_BOTH_EDGES,
}GPIO_INTERRUPT_STYLE;

// *****************************************************************************
/* GPIO Port Pins

  Summary:
    Identifies the available GPIO port pins.

  Description:
    This enumeration identifies the available GPIO port pins.

  Remarks:
    The caller should not rely on the specific numbers assigned to any of
    these values as they may change from one processor to the next.

    Not all pins are available on all devices.  Refer to the specific
    device data sheet to determine which pins are supported.
*/


#define     GPIO_PIN_RA0  (0U)
#define     GPIO_PIN_RA1  (1U)
#define     GPIO_PIN_RA2  (2U)
#define     GPIO_PIN_RA3  (3U)
#define     GPIO_PIN_RA4  (4U)
#define     GPIO_PIN_RA5  (5U)
#define     GPIO_PIN_RA6  (6U)
#define     GPIO_PIN_RA7  (7U)
#define     GPIO_PIN_RA9  (9U)
#define     GPIO_PIN_RA10  (10U)
#define     GPIO_PIN_RA14  (14U)
#define     GPIO_PIN_RA15  (15U)
#define     GPIO_PIN_RB0  (16U)
#define     GPIO_PIN_RB1  (17U)
#define     GPIO_PIN_RB2  (18U)
#define     GPIO_PIN_RB3  (19U)
#define     GPIO_PIN_RB4  (20U)
#define     GPIO_PIN_RB5  (21U)
#define     GPIO_PIN_RB6  (22U)
#define     GPIO_PIN_RB7  (23U)
#define     GPIO_PIN_RB8  (24U)
#define     GPIO_PIN_RB9  (25U)
#define     GPIO_PIN_RB10  (26U)
#define     GPIO_PIN_RB11  (27U)
#define     GPIO_PIN_RB12  (28U)
#define     GPIO_PIN_RB13  (29U)
#define     GPIO_PIN_RB14  (30U)
#define     GPIO_PIN_RB15  (31U)
#define     GPIO_PIN_RC1  (33U)
#define     GPIO_PIN_RC2  (34U)
#define     GPIO_PIN_RC3  (35U)
#define     GPIO_PIN_RC4  (36U)
#define     GPIO_PIN_RC12  (44U)
#define     GPIO_PIN_RC13  (45U)
#define     GPIO_PIN_RC14  (46U)
#define     GPIO_PIN_RC15  (47U)
#define     GPIO_PIN_RD0  (48U)
#define     GPIO_PIN_RD1  (49U)
#define     GPIO_PIN_RD2  (50U)
#define     GPIO_PIN_RD3  (51U)
#define     GPIO_PIN_RD4  (52U)
#define     GPIO_PIN_RD5  (53U)
#define     GPIO_PIN_RD9  (57U)
#define     GPIO_PIN_RD10  (58U)
#define     GPIO_PIN_RD11  (59U)
#define     GPIO_PIN_RD12  (60U)
#define     GPIO_PIN_RD13  (61U)
#define     GPIO_PIN_RD14  (62U)
#define     GPIO_PIN_RD15  (63U)
#define     GPIO_PIN_RE0  (64U)
#define     GPIO_PIN_RE1  (65U)
#define     GPIO_PIN_RE2  (66U)
#define     GPIO_PIN_RE3  (67U)
#define     GPIO_PIN_RE4  (68U)
#define     GPIO_PIN_RE5  (69U)
#define     GPIO_PIN_RE6  (70U)
#define     GPIO_PIN_RE7  (71U)
#define     GPIO_PIN_RE8  (72U)
#define     GPIO_PIN_RE9  (73U)
#define     GPIO_PIN_RF0  (80U)
#define     GPIO_PIN_RF1  (81U)
#define     GPIO_PIN_RF2  (82U)
#define     GPIO_PIN_RF3  (83U)
#define     GPIO_PIN_RF4  (84U)
#define     GPIO_PIN_RF5  (85U)
#define     GPIO_PIN_RF8  (88U)
#define     GPIO_PIN_RF12  (92U)
#define     GPIO_PIN_RF13  (93U)
#define     GPIO_PIN_RG0  (96U)
#define     GPIO_PIN_RG1  (97U)
#define     GPIO_PIN_RG6  (102U)
#define     GPIO_PIN_RG7  (103U)
#define     GPIO_PIN_RG8  (104U)
#define     GPIO_PIN_RG9  (105U)
#define     GPIO_PIN_RG12  (108U)
#define     GPIO_PIN_RG13  (109U)
#define     GPIO_PIN_RG14  (110U)
#define     GPIO_PIN_RG15  (111U)

    /* This element should not be used in any of the GPIO APIs.
       It will be used by other modules or application to denote that none of the GPIO Pin is used */
#define    GPIO_PIN_NONE   (-1)

typedef uint32_t GPIO_PIN;


void GPIO_Initialize(void);

// *****************************************************************************
// *****************************************************************************
// Section: GPIO Functions which operates on multiple pins of a port
// *****************************************************************************
// *****************************************************************************

uint32_t GPIO_PortRead(GPIO_PORT port);

void GPIO_PortWrite(GPIO_PORT port, uint32_t mask, uint32_t value);

uint32_t GPIO_PortLatchRead ( GPIO_PORT port );

void GPIO_PortSet(GPIO_PORT port, uint32_t mask);

void GPIO_PortClear(GPIO_PORT port, uint32_t mask);

void GPIO_PortToggle(GPIO_PORT port, uint32_t mask);

void GPIO_PortInputEnable(GPIO_PORT port, uint32_t mask);

void GPIO_PortOutputEnable(GPIO_PORT port, uint32_t mask);

// *****************************************************************************
// *****************************************************************************
// Section: GPIO Functions which operates on one pin at a time
// *****************************************************************************
// *****************************************************************************

static inline void GPIO_PinWrite(GPIO_PIN pin, bool value)
{
     uint32_t xvalue = (uint32_t)value;
    GPIO_PortWrite((pin>>4U), (uint32_t)(0x1U) << (pin & 0xFU), (xvalue) << (pin & 0xFU));
}

static inline bool GPIO_PinRead(GPIO_PIN pin)
{
    return ((((GPIO_PortRead((GPIO_PORT)(pin>>4U))) >> (pin & 0xFU)) & 0x1U) != 0U);
}

static inline bool GPIO_PinLatchRead(GPIO_PIN pin)
{
    return (((GPIO_PortLatchRead((GPIO_PORT)(pin>>4U)) >> (pin & 0xFU)) & 0x1U) != 0U);
}

static inline void GPIO_PinToggle(GPIO_PIN pin)
{
    GPIO_PortToggle((pin>>4U), (uint32_t)0x1U << (pin & 0xFU));
}

static inline void GPIO_PinSet(GPIO_PIN pin)
{
    GPIO_PortSet((pin>>4U), (uint32_t)0x1U << (pin & 0xFU));
}

static inline void GPIO_PinClear(GPIO_PIN pin)
{
    GPIO_PortClear((pin>>4U), (uint32_t)0x1U << (pin & 0xFU));
}

static inline void GPIO_PinInputEnable(GPIO_PIN pin)
{
    GPIO_PortInputEnable((pin>>4U), (uint32_t)0x1U << (pin & 0xFU));
}

static inline void GPIO_PinOutputEnable(GPIO_PIN pin)
{
    GPIO_PortOutputEnable((pin>>4U), (uint32_t)0x1U << (pin & 0xFU));
}


// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    }

#endif
// DOM-IGNORE-END
#endif // PLIB_GPIO_H
