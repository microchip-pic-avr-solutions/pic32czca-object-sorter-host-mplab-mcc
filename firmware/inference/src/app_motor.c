/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_motor.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It
    implements the logic of the application's state machine and it may call
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
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

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "app_motor.h"
#include "system/time/sys_time.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_MOTOR_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

APP_MOTOR_DATA app_motorData;

static uint32_t forwardStartTime = 0;
static uint32_t homingStartTime = 0;

#define SERVO_MIN_PULSE         500U
#define HOMING_TIMEOUT_MS       3000U
#define FORWARD_TIMEOUT_MS      3400U
#define REVERSE_TIMEOUT_MS      3400U
#define SERVO_MAX_PULSE         1500U
#define DISPENSER_SPEED         30U
#define DROP_SERVO_DELAY_MS     750U

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************

/*
 * Function: APP_MOTOR_Conveyor_SetSpeed
 * -------------------------------------
 * Sets the conveyor motor speed and direction.
 *
 * Parameters:
 *   speed     - Speed percentage (0-100).
 *   direction - Direction (0 or 1).
 *
 * Returns:
 *   None.
 */
void APP_MOTOR_Conveyor_SetSpeed(uint8_t speed, uint8_t direction)
{
    uint16_t duty = (uint16_t)(((uint32_t)PWM0_ChannelPeriodGet(PWM_CHANNEL_0) * speed) / 100U);
    if (direction == 0)
    {
        PWM0_ChannelDutySet(PWM_CHANNEL_0, 0);
        PWM0_ChannelDutySet(PWM_CHANNEL_1, duty);
    }
    else
    {
        PWM0_ChannelDutySet(PWM_CHANNEL_0, duty);
        PWM0_ChannelDutySet(PWM_CHANNEL_1, 0);
    }
}


/*
 * Function: APP_MOTOR_Conveyor_Start
 * ----------------------------------
 * Starts the conveyor motor at specified speed and direction.
 *
 * Parameters:
 *   speed     - Speed percentage (0-100).
 *   direction - Direction (0 or 1).
 *
 * Returns:
 *   None.
 */
void APP_MOTOR_Conveyor_Start(uint8_t speed, uint8_t direction)
{
    APP_MOTOR_Conveyor_SetSpeed(speed, direction);
}


/*
 * Function: APP_MOTOR_Conveyor_Stop
 * ---------------------------------
 * Stops the conveyor motor.
 *
 * Parameters:
 *   None.
 *
 * Returns:
 *   None.
 */
void APP_MOTOR_Conveyor_Stop(void)
{
    APP_MOTOR_Conveyor_SetSpeed(0, 0);
}


/*
 * Function: APP_MOTOR_Dispenser_ApplyPWM
 * --------------------------------------
 * Applies PWM duty cycle to the dispenser motor.
 *
 * Parameters:
 *   speed     - Speed percentage (0-100).
 *   direction - Direction (0 or 1).
 *
 * Returns:
 *   None.
 */
static void APP_MOTOR_Dispenser_ApplyPWM(uint8_t speed, uint8_t direction)
{
    uint16_t duty = (uint16_t)(((uint32_t)PWM0_ChannelPeriodGet(PWM_CHANNEL_2) * speed) / 100U);
    if (direction == 0)
    {
        PWM0_ChannelDutySet(PWM_CHANNEL_2, duty);
        PWM0_ChannelDutySet(PWM_CHANNEL_3, 0);
    }
    else
    {
        PWM0_ChannelDutySet(PWM_CHANNEL_2, 0);
        PWM0_ChannelDutySet(PWM_CHANNEL_3, duty);
    }
}


/*
 * Function: APP_MOTOR_Dispenser_SetSpeed
 * --------------------------------------
 * Sets the dispenser motor speed and direction.
 *
 * Parameters:
 *   speed     - Speed percentage (0-100).
 *   direction - Direction (0 or 1).
 *
 * Returns:
 *   None.
 */
void APP_MOTOR_Dispenser_SetSpeed(uint8_t speed, uint8_t direction)
{
    APP_MOTOR_Dispenser_ApplyPWM(speed, direction);
}


/*
 * Function: APP_MOTOR_Dispenser_Start
 * -----------------------------------
 * Starts the dispenser motor at specified speed and direction.
 *
 * Parameters:
 *   speed     - Speed percentage (0-100).
 *   direction - Direction (0 or 1).
 *
 * Returns:
 *   None.
 */
void APP_MOTOR_Dispenser_Start(uint8_t speed, uint8_t direction)
{
    APP_MOTOR_Dispenser_SetSpeed(speed, direction);
}


/*
 * Function: APP_MOTOR_Dispenser_Stop
 * ----------------------------------
 * Stops the dispenser motor.
 *
 * Parameters:
 *   None.
 *
 * Returns:
 *   None.
 */
void APP_MOTOR_Dispenser_Stop(void)
{
    APP_MOTOR_Dispenser_ApplyPWM(0, 0);
}


/*
 * Function: APP_MOTOR_DropObject
 * ------------------------------
 * Triggers the drop servo to release an object.
 *
 * Parameters:
 *   None.
 *
 * Returns:
 *   None.
 */
void APP_MOTOR_DropObject(void)
{
    if (app_motorData.dropServoState == DROP_SERVO_IDLE)
    {
        TC0_CH0_CompareASet(SERVO_MAX_PULSE);
        app_motorData.dropServoState = DROP_SERVO_DROPPING;
        app_motorData.dropServoStartTime = SYS_TIME_CountToMS(SYS_TIME_CounterGet());
    }
}


/*
 * Function: APP_MOTOR_IsDropComplete
 * ----------------------------------
 * Checks if the drop servo operation is complete.
 *
 * Parameters:
 *   None.
 *
 * Returns:
 *   true if drop is complete, false otherwise.
 */
bool APP_MOTOR_IsDropComplete(void)
{
    return (app_motorData.dropServoState == DROP_SERVO_IDLE);
}


/*
 * Function: APP_MOTOR_DispenserMoveForward
 * ----------------------------------------
 * Moves the dispenser forward from home position.
 *
 * Parameters:
 *   None.
 *
 * Returns:
 *   None.
 */
void APP_MOTOR_DispenserMoveForward(void)
{
    if (app_motorData.isHomed && app_motorData.dispenserState == DISPENSER_STATE_IDLE)
    {
        APP_MOTOR_Dispenser_ApplyPWM(DISPENSER_SPEED, 0);
        app_motorData.dispenserState = DISPENSER_STATE_FORWARD;
        forwardStartTime = SYS_TIME_CountToMS(SYS_TIME_CounterGet());
    }
}


/*
 * Function: APP_MOTOR_DispenserMoveHome
 * -------------------------------------
 * Moves the dispenser back to home position.
 *
 * Parameters:
 *   None.
 *
 * Returns:
 *   None.
 */
void APP_MOTOR_DispenserMoveHome(void)
{
    if (app_motorData.dispenserState == DISPENSER_STATE_IDLE && STOP_LS_Get())
    {
        APP_MOTOR_Dispenser_ApplyPWM(DISPENSER_SPEED, 1);
        app_motorData.reverseStartTime = SYS_TIME_CountToMS(SYS_TIME_CounterGet());
        app_motorData.dispenserState = DISPENSER_STATE_REVERSE;
    }
}


/*
 * Function: APP_MOTOR_DispenserIsIdle
 * -----------------------------------
 * Checks if the dispenser is idle.
 *
 * Parameters:
 *   None.
 *
 * Returns:
 *   true if idle, false otherwise.
 */
bool APP_MOTOR_DispenserIsIdle(void)
{
    return (app_motorData.dispenserState == DISPENSER_STATE_IDLE);
}


/*
 * Function: APP_MOTOR_IsAtHome
 * ----------------------------
 * Checks if the dispenser is at the home position (limit switch).
 *
 * Parameters:
 *   None.
 *
 * Returns:
 *   true if at home, false otherwise.
 */
bool APP_MOTOR_IsAtHome(void)
{
    return !STOP_LS_Get();
}


/*
 * Function: APP_MOTOR_IsReady
 * ---------------------------
 * Checks if the motor subsystem is ready for operation.
 *
 * Parameters:
 *   None.
 *
 * Returns:
 *   true if ready, false otherwise.
 */
bool APP_MOTOR_IsReady(void)
{
    return (app_motorData.state == APP_MOTOR_STATE_SERVICE_TASKS);
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_MOTOR_Initialize ( void )

  Remarks:
    See prototype in app_motor.h.
 */

void APP_MOTOR_Initialize(void)
{
    app_motorData.state = APP_MOTOR_STATE_INIT;
    app_motorData.dispenserState = DISPENSER_STATE_IDLE;
    app_motorData.dropServoState = DROP_SERVO_IDLE;
    app_motorData.isHomed = false;
    app_motorData.reverseStartTime = 0;
    app_motorData.dropServoStartTime = 0;
}


/******************************************************************************
  Function:
    void APP_MOTOR_Tasks ( void )

  Remarks:
    See prototype in app_motor.h.
 */

void APP_MOTOR_Tasks(void)
{

    switch (app_motorData.state)
    {
        case APP_MOTOR_STATE_INIT:
        {
            PWM0_ChannelsStart(PWM_CHANNEL_0_MASK | PWM_CHANNEL_1_MASK |
                    PWM_CHANNEL_2_MASK | PWM_CHANNEL_3_MASK);

            TC0_CH0_CompareStart();
            TC0_CH0_CompareASet(SERVO_MIN_PULSE);

            if (!STOP_LS_Get())
            {
                app_motorData.isHomed = true;
                app_motorData.dispenserState = DISPENSER_STATE_IDLE;
                app_motorData.state = APP_MOTOR_STATE_SERVICE_TASKS;
            }
            else
            {
                APP_MOTOR_Dispenser_ApplyPWM(DISPENSER_SPEED, 1);
                homingStartTime = SYS_TIME_CountToMS(SYS_TIME_CounterGet());
                app_motorData.state = APP_MOTOR_STATE_HOMING;
            }
            break;
        }

        case APP_MOTOR_STATE_HOMING:
        {
            uint32_t elapsed = SYS_TIME_CountToMS(SYS_TIME_CounterGet()) - homingStartTime;
            bool limitSwitchActive = !STOP_LS_Get();
            if (limitSwitchActive || elapsed >= HOMING_TIMEOUT_MS)
            {
                APP_MOTOR_Dispenser_ApplyPWM(0, 0);
                app_motorData.isHomed = limitSwitchActive;
                app_motorData.dispenserState = DISPENSER_STATE_IDLE;
                app_motorData.state = APP_MOTOR_STATE_SERVICE_TASKS;
            }
            break;
        }

        case APP_MOTOR_STATE_SERVICE_TASKS:
        {
            switch (app_motorData.dispenserState)
            {
                case DISPENSER_STATE_FORWARD:
                {
                    uint32_t elapsed = SYS_TIME_CountToMS(SYS_TIME_CounterGet()) - forwardStartTime;
                    if (elapsed >= FORWARD_TIMEOUT_MS)
                    {
                        APP_MOTOR_Dispenser_ApplyPWM(0, 0);
                        app_motorData.dispenserState = DISPENSER_STATE_IDLE;
                    }
                    break;
                }

                case DISPENSER_STATE_REVERSE:
                {
                    uint32_t elapsed = SYS_TIME_CountToMS(SYS_TIME_CounterGet()) - app_motorData.reverseStartTime;
                    bool limitSwitchActive = !STOP_LS_Get();
                    if (limitSwitchActive || elapsed >= REVERSE_TIMEOUT_MS)
                    {
                        APP_MOTOR_Dispenser_ApplyPWM(0, 0);
                        app_motorData.dispenserState = DISPENSER_STATE_IDLE;
                    }
                    break;
                }

                case DISPENSER_STATE_IDLE:
                default:
                    break;
            }
            break;
        }

        default:
        {
            break;
        }
    }

    if (app_motorData.dropServoState != DROP_SERVO_IDLE)
    {
        uint32_t elapsed = SYS_TIME_CountToMS(SYS_TIME_CounterGet()) - app_motorData.dropServoStartTime;
        if (elapsed >= DROP_SERVO_DELAY_MS)
        {
            if (app_motorData.dropServoState == DROP_SERVO_DROPPING)
            {
                TC0_CH0_CompareASet(SERVO_MIN_PULSE);
                app_motorData.dropServoState = DROP_SERVO_RETURNING;
                app_motorData.dropServoStartTime = SYS_TIME_CountToMS(SYS_TIME_CounterGet());
            }
            else if (app_motorData.dropServoState == DROP_SERVO_RETURNING)
            {
                app_motorData.dropServoState = DROP_SERVO_IDLE;
            }
        }
    }
}


/*******************************************************************************
 End of File
 */
