/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app_motor.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APP_MOTOR_Initialize" and "APP_MOTOR_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_MOTOR_STATES" definition).  Both
    are defined here for convenience.
*******************************************************************************/

/*******************************************************************************
* Copyright (C) 2026 Microchip Technology Inc. and its subsidiaries.
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

#ifndef _APP_MOTOR_H
#define _APP_MOTOR_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "configuration.h"
#include "definitions.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Type Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application states

  Summary:
    Application states enumeration

  Description:
    This enumeration defines the valid application states.  These states
    determine the behavior of the application at various times.
*/

typedef enum
{
    APP_MOTOR_STATE_INIT = 0,
    APP_MOTOR_STATE_HOMING,
    APP_MOTOR_STATE_SERVICE_TASKS
} APP_MOTOR_STATES;

typedef enum
{
    DISPENSER_STATE_IDLE = 0,
    DISPENSER_STATE_FORWARD,
    DISPENSER_STATE_REVERSE
} DISPENSER_STATES;

typedef enum
{
    DROP_SERVO_IDLE = 0,
    DROP_SERVO_DROPPING,
    DROP_SERVO_RETURNING
} DROP_SERVO_STATE;


// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    Application strings and buffers are be defined outside this structure.
 */

typedef struct
{
    APP_MOTOR_STATES state;
    DISPENSER_STATES dispenserState;
    DROP_SERVO_STATE dropServoState;
    bool isHomed;
    uint32_t reverseStartTime;
    uint32_t dropServoStartTime;
} APP_MOTOR_DATA;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Routines
// *****************************************************************************
// *****************************************************************************
/* These routines are called by drivers when certain events occur.
*/

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_MOTOR_Initialize ( void )

  Summary:
     MPLAB Harmony application initialization routine.

  Description:
    This function initializes the Harmony application.  It places the
    application in its initial state and prepares it to run so that its
    APP_MOTOR_Tasks function can be called.

  Precondition:
    All other system initialization routines should be called before calling
    this routine (in "SYS_Initialize").

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    APP_MOTOR_Initialize();
    </code>

  Remarks:
    This routine must be called from the SYS_Initialize function.
*/

void APP_MOTOR_Initialize(void);
void APP_MOTOR_Tasks(void);

void APP_MOTOR_Conveyor_Start(uint8_t speed, uint8_t direction);
void APP_MOTOR_Conveyor_Stop(void);
void APP_MOTOR_Conveyor_SetSpeed(uint8_t speed, uint8_t direction);

void APP_MOTOR_Dispenser_Start(uint8_t speed, uint8_t direction);
void APP_MOTOR_Dispenser_Stop(void);
void APP_MOTOR_Dispenser_SetSpeed(uint8_t speed, uint8_t direction);

void APP_MOTOR_DropObject(void);
bool APP_MOTOR_IsDropComplete(void);
bool APP_MOTOR_IsAtHome(void);
bool APP_MOTOR_IsReady(void);
void APP_MOTOR_DispenserMoveForward(void);
void APP_MOTOR_DispenserMoveHome(void);
bool APP_MOTOR_DispenserIsIdle(void);

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* _APP_MOTOR_H */

/*******************************************************************************
 End of File
 */
