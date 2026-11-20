/*******************************************************************************
  Board Support Package Header File.

  Company:
    Microchip Technology Inc.

  File Name:
    bsp.h

  Summary:
    Board Support Package Header File 

  Description:
    This file contains constants, macros, type definitions and function
    declarations 
*******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2023 Microchip Technology Inc. and its subsidiaries.
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
// DOM-IGNORE-END

#ifndef BSP_H
#define BSP_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "device.h"
#include "peripheral/pio/plib_pio.h"

// *****************************************************************************
// *****************************************************************************
// Section: BSP Macros
// *****************************************************************************
// *****************************************************************************
#define PIC32CZ_CA70_CURIOSITY_ULTRA
#define BOARD_NAME    "PIC32CZ-CA70-CURIOSITY-ULTRA"

/*** OUTPUT PIO Macros for LED0 ***/
#define BSP_LED0_PIN        PIO_PIN_PD23
#define BSP_LED0_Get()      ((PIOD_REGS->PIO_PDSR >> 23) & 0x1)
#define BSP_LED0_On()       (PIOD_REGS->PIO_CODR = (1UL<<23))
#define BSP_LED0_Off()      (PIOD_REGS->PIO_SODR = (1UL<<23))
#define BSP_LED0_Toggle()   (PIOD_REGS->PIO_ODSR ^= (1UL<<23))

/*** OUTPUT PIO Macros for MIKROBUS_SPI_CS ***/
#define BSP_MIKROBUS_SPI_CS_PIN        PIO_PIN_PA10
#define BSP_MIKROBUS_SPI_CS_Get()      ((PIOA_REGS->PIO_PDSR >> 10) & 0x1)
#define BSP_MIKROBUS_SPI_CS_On()       (PIOA_REGS->PIO_CODR = (1UL<<10))
#define BSP_MIKROBUS_SPI_CS_Off()      (PIOA_REGS->PIO_SODR = (1UL<<10))
#define BSP_MIKROBUS_SPI_CS_Toggle()   (PIOA_REGS->PIO_ODSR ^= (1UL<<10))

/*** OUTPUT PIO Macros for LED1 ***/
#define BSP_LED1_PIN        PIO_PIN_PD29
#define BSP_LED1_Get()      ((PIOD_REGS->PIO_PDSR >> 29) & 0x1)
#define BSP_LED1_On()       (PIOD_REGS->PIO_CODR = (1UL<<29))
#define BSP_LED1_Off()      (PIOD_REGS->PIO_SODR = (1UL<<29))
#define BSP_LED1_Toggle()   (PIOD_REGS->PIO_ODSR ^= (1UL<<29))


/*** INPUT PIO Macros for STOP_LS ***/
#define BSP_STOP_LS_PIN                    PIO_PIN_PC31
#define BSP_STOP_LS_Get()                  ((PIOC_REGS->PIO_PDSR >> 31) & 0x1)
#define BSP_STOP_LS_STATE_PRESSED          0
#define BSP_STOP_LS_STATE_RELEASED         1
#define BSP_STOP_LS_InterruptEnable()      (PIOC_REGS->PIO_IER = (1UL<<31))
#define BSP_STOP_LS_InterruptDisable()     (PIOC_REGS->PIO_IDR = (1UL<<31))

/*** INPUT PIO Macros for USER_SWITCH1 ***/
#define BSP_USER_SWITCH1_PIN                    PIO_PIN_PA09
#define BSP_USER_SWITCH1_Get()                  ((PIOA_REGS->PIO_PDSR >> 09) & 0x1)
#define BSP_USER_SWITCH1_STATE_PRESSED          0
#define BSP_USER_SWITCH1_STATE_RELEASED         1
#define BSP_USER_SWITCH1_InterruptEnable()      (PIOA_REGS->PIO_IER = (1UL<<09))
#define BSP_USER_SWITCH1_InterruptDisable()     (PIOA_REGS->PIO_IDR = (1UL<<09))

/*** INPUT PIO Macros for USER_SWITCH0 ***/
#define BSP_USER_SWITCH0_PIN                    PIO_PIN_PA01
#define BSP_USER_SWITCH0_Get()                  ((PIOA_REGS->PIO_PDSR >> 01) & 0x1)
#define BSP_USER_SWITCH0_STATE_PRESSED          0
#define BSP_USER_SWITCH0_STATE_RELEASED         1
#define BSP_USER_SWITCH0_InterruptEnable()      (PIOA_REGS->PIO_IER = (1UL<<01))
#define BSP_USER_SWITCH0_InterruptDisable()     (PIOA_REGS->PIO_IDR = (1UL<<01))

/*** INPUT PIO Macros for USB_VBUS_SENSE ***/
#define BSP_USB_VBUS_SENSE_PIN                    PIO_PIN_PC24
#define BSP_USB_VBUS_SENSE_Get()                  ((PIOC_REGS->PIO_PDSR >> 24) & 0x1)
#define BSP_USB_VBUS_SENSE_STATE_PRESSED          1
#define BSP_USB_VBUS_SENSE_STATE_RELEASED         0
#define BSP_USB_VBUS_SENSE_InterruptEnable()      (PIOC_REGS->PIO_IER = (1UL<<24))
#define BSP_USB_VBUS_SENSE_InterruptDisable()     (PIOC_REGS->PIO_IDR = (1UL<<24))



// *****************************************************************************
// *****************************************************************************
// Section: Interface Routines
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Function:
    void BSP_Initialize(void)

  Summary:
    Performs the necessary actions to initialize a board

  Description:
    This function initializes the LED and Switch ports on the board.  This
    function must be called by the user before using any APIs present on this
    BSP.

  Precondition:
    None.

  Parameters:
    None

  Returns:
    None.

  Example:
    <code>
    BSP_Initialize();
    </code>

  Remarks:
    None
*/

void BSP_Initialize(void);

#endif // BSP_H

/*******************************************************************************
 End of File
*/