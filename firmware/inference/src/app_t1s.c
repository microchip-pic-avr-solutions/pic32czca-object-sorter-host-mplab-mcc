/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_t1s.c

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

#include "app_t1s.h"
#include "app_motor.h"

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
    This structure should be initialized by the APP_T1S_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

APP_T1S_DATA app_t1sData;

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************

/*
 * Function: APP_T1S_StopAllMotors
 * -------------------------------
 * Stops all motors and resets motor state variables.
 *
 * Parameters:
 *   None.
 *
 * Returns:
 *   None.
 */
static void APP_T1S_StopAllMotors(void)
{
    APP_MOTOR_Conveyor_Stop();
    APP_MOTOR_Dispenser_Stop();
    app_t1sData.conveyor_state = 0;
    app_t1sData.dispenser_state = 0;
}


/*
 * Function: APP_T1S_Data_Handle
 * -----------------------------
 * Handles data communication with client0 and client1.
 *
 * Parameters:
 *   None.
 *
 * Returns:
 *   None.
 */
static void APP_T1S_Data_Handle(void)
{
    uint8_t txBuffer[T1S_TX_DATA_SIZE];
    uint8_t rxBuffer[T1S_RX_DATA_SIZE];

    switch (app_t1sData.host_state)
    {
        case APP_T1S_HOST_SEND_READ_CLIENT0:
        {
            app_t1sData.client0_ready = TCPIP_TCP_IsConnected(app_t1sData.client0_socket);
            
            if (!app_t1sData.client0_ready)
            {
                app_t1sData.host_state = APP_T1S_HOST_SEND_READ_CLIENT1;
                break;
            }
            
            if (TCPIP_TCP_PutIsReady(app_t1sData.client0_socket) >= sizeof(txBuffer))
            {
                memset(txBuffer, 0, sizeof(txBuffer));
                
                txBuffer[0] = T1S_DATA_CMD;
                txBuffer[1] = app_t1sData.client0_gate;

                TCPIP_TCP_ArrayPut(app_t1sData.client0_socket, txBuffer, sizeof(txBuffer));
                
                app_t1sData.host_state = APP_T1S_HOST_WAIT_REPLY_CLIENT0;
            }
            break;
        }

        case APP_T1S_HOST_WAIT_REPLY_CLIENT0:
        {
            app_t1sData.client0_ready = TCPIP_TCP_IsConnected(app_t1sData.client0_socket);
            
            if (!app_t1sData.client0_ready)
            {
                app_t1sData.host_state = APP_T1S_HOST_SEND_READ_CLIENT1;
                break;
            }
            
            if (TCPIP_TCP_GetIsReady(app_t1sData.client0_socket) >= sizeof(rxBuffer))
            {
                TCPIP_TCP_ArrayGet(app_t1sData.client0_socket, rxBuffer, sizeof(rxBuffer));

                if (rxBuffer[0] == T1S_DATA_ACK)
                {
                    app_t1sData.client0_gateControl = rxBuffer[1];
                    app_t1sData.client0_gateStatus = rxBuffer[2];
                    app_t1sData.client0_objectCount = (uint16_t)rxBuffer[3] | ((uint16_t)rxBuffer[4] << 8);
                    
                    if (rxBuffer[5])
                    {
                        app_t1sData.client0_objectPassed = 1;
                    }
                }
                
                app_t1sData.host_state = APP_T1S_HOST_SEND_READ_CLIENT1;
            }
            break;
        }

        case APP_T1S_HOST_SEND_READ_CLIENT1:
        {
            app_t1sData.client1_ready = TCPIP_TCP_IsConnected(app_t1sData.client1_socket);
            
            if (!app_t1sData.client1_ready)
            {
                app_t1sData.host_state = APP_T1S_HOST_SEND_READ_CLIENT0;
                break;
            }
            
            if (TCPIP_TCP_PutIsReady(app_t1sData.client1_socket) >= sizeof(txBuffer))
            {
                memset(txBuffer, 0, sizeof(txBuffer));
                
                txBuffer[0] = T1S_DATA_CMD;
                txBuffer[1] = app_t1sData.client1_gate;

                TCPIP_TCP_ArrayPut(app_t1sData.client1_socket, txBuffer, sizeof(txBuffer));
                
                app_t1sData.host_state = APP_T1S_HOST_WAIT_REPLY_CLIENT1;
            }
            break;
        }

        case APP_T1S_HOST_WAIT_REPLY_CLIENT1:
        {
            app_t1sData.client1_ready = TCPIP_TCP_IsConnected(app_t1sData.client1_socket);
            
            if (!app_t1sData.client1_ready)
            {
                app_t1sData.host_state = APP_T1S_HOST_SEND_READ_CLIENT0;
                break;
            }
            
            if (TCPIP_TCP_GetIsReady(app_t1sData.client1_socket) >= sizeof(rxBuffer))
            {
                TCPIP_TCP_ArrayGet(app_t1sData.client1_socket, rxBuffer, sizeof(rxBuffer));

                if (rxBuffer[0] == T1S_DATA_ACK)
                {
                    app_t1sData.client1_gateControl = rxBuffer[1];
                    app_t1sData.client1_gateStatus = rxBuffer[2];
                    app_t1sData.client1_objectCount = (uint16_t)rxBuffer[3] | ((uint16_t)rxBuffer[4] << 8);
                    
                    if (rxBuffer[5])
                    {
                        app_t1sData.client1_objectPassed = 1;
                    }
                }
                
                app_t1sData.host_state = APP_T1S_HOST_SEND_READ_CLIENT0;
            }
            break;
        }
    }
}


/*
 * Function: APP_T1S_GetTimeMS
 * ---------------------------
 * Gets the current system time in milliseconds.
 *
 * Parameters:
 *   None.
 *
 * Returns:
 *   Current time in milliseconds.
 */
static uint32_t APP_T1S_GetTimeMS(void)
{
    return SYS_TIME_CountToMS(SYS_TIME_CounterGet());
}


/*
 * Function: APP_T1S_TimeoutCheck
 * ------------------------------
 * Checks if the specified timeout has elapsed.
 *
 * Parameters:
 *   timeout_ms - Timeout duration in milliseconds.
 *
 * Returns:
 *   true if timeout elapsed, false otherwise.
 */
static bool APP_T1S_TimeoutCheck(uint32_t timeout_ms)
{
    uint32_t elapsed = APP_T1S_GetTimeMS() - app_t1sData.auto_timeout_start;
    return (elapsed >= timeout_ms);
}


/*
 * Function: APP_T1S_TimeoutReset
 * ------------------------------
 * Resets the timeout start time to current time.
 *
 * Parameters:
 *   None.
 *
 * Returns:
 *   None.
 */
static void APP_T1S_TimeoutReset(void)
{
    app_t1sData.auto_timeout_start = APP_T1S_GetTimeMS();
}


/*
 * Function: APP_T1S_ClientsReady
 * ------------------------------
 * Checks if both clients are connected and ready.
 *
 * Parameters:
 *   None.
 *
 * Returns:
 *   true if both clients ready, false otherwise.
 */
static bool APP_T1S_ClientsReady(void)
{
    return (app_t1sData.client0_ready && app_t1sData.client1_ready);
}


/*
 * Function: APP_T1S_Auto_Handle
 * -----------------------------
 * Handles the automatic sorting state machine.
 *
 * Parameters:
 *   None.
 *
 * Returns:
 *   None.
 */
static void APP_T1S_Auto_Handle(void)
{
    /* Only run in auto mode with capture active */
    if (app_t1sData.mode != 0 || !app_t1sData.capture_active)
    {
        app_t1sData.auto_state = AUTO_STATE_HOME_CHECK;
        return;
    }

    /* Check client connections before proceeding with gate operations */
    if (!APP_T1S_ClientsReady())
    {
        if (app_t1sData.auto_state >= AUTO_STATE_GATE_OPEN &&
            app_t1sData.auto_state <= AUTO_STATE_CONVEYOR_STOP)
        {
            APP_T1S_StopAllMotors();
            
            app_t1sData.client0_gate = 0;
            app_t1sData.client1_gate = 0;
            app_t1sData.auto_state = AUTO_STATE_HOME_CHECK;
            return;
        }
    }

    switch (app_t1sData.auto_state)
    {
        case AUTO_STATE_HOME_CHECK:
        {
            if (!APP_T1S_ClientsReady())
            {
                break;
            }

            if (APP_MOTOR_IsAtHome())
            {
                app_t1sData.auto_state = AUTO_STATE_FORWARD;
            }
            
            else
            {
                APP_MOTOR_DispenserMoveHome();
                APP_T1S_TimeoutReset();
                app_t1sData.auto_state = AUTO_STATE_HOME_WAIT;
            }
            break;
        }

        case AUTO_STATE_HOME_WAIT:
        {
            if (APP_MOTOR_DispenserIsIdle() && APP_MOTOR_IsAtHome())
            {
                app_t1sData.auto_state = AUTO_STATE_FORWARD;
            }
            
            else if (APP_T1S_TimeoutCheck(AUTO_TIMEOUT_MS))
            {
                APP_MOTOR_Dispenser_Stop();
                app_t1sData.auto_state = AUTO_STATE_HOME_CHECK;
            }
            break;
        }

        case AUTO_STATE_FORWARD:
        {
            APP_MOTOR_DispenserMoveForward();
            APP_T1S_TimeoutReset();
            
            app_t1sData.auto_state = AUTO_STATE_FORWARD_WAIT;
            break;
        }

        case AUTO_STATE_FORWARD_WAIT:
        {
            if (APP_MOTOR_DispenserIsIdle())
            {
                app_t1sData.auto_state = AUTO_STATE_HOME_CHECK_2;
            }
            else if (APP_T1S_TimeoutCheck(AUTO_TIMEOUT_MS))
            {
                APP_MOTOR_Dispenser_Stop();
                
                app_t1sData.auto_state = AUTO_STATE_HOME_CHECK;
            }
            break;
        }

        case AUTO_STATE_HOME_CHECK_2:
        {
            if (APP_MOTOR_IsAtHome())
            {
                APP_T1S_TimeoutReset();
                
                app_t1sData.auto_state = AUTO_STATE_CAPTURE;
            }
            
            else
            {
                APP_MOTOR_DispenserMoveHome();
                APP_T1S_TimeoutReset();
                
                app_t1sData.auto_state = AUTO_STATE_HOME_WAIT_2;
            }
            break;
        }

        case AUTO_STATE_HOME_WAIT_2:
        {
            if (APP_MOTOR_DispenserIsIdle() && APP_MOTOR_IsAtHome())
            {
                APP_T1S_TimeoutReset();
                
                app_t1sData.auto_state = AUTO_STATE_CAPTURE;
            }
            
            else if (APP_T1S_TimeoutCheck(AUTO_TIMEOUT_MS))
            {
                APP_MOTOR_Dispenser_Stop();
                
                app_t1sData.auto_state = AUTO_STATE_HOME_CHECK;
            }
            break;
        }

        case AUTO_STATE_CAPTURE:
        {
            if (APP_ML_IsReady())
            {
                APP_ML_TriggerInference();
                APP_T1S_TimeoutReset();
                
                app_t1sData.auto_state = AUTO_STATE_CLASSIFY_WAIT;
            }
            
            else if (APP_T1S_TimeoutCheck(AUTO_TIMEOUT_MS))
            {
                app_t1sData.ml_classification = CLASS_UNKNOWN;
                app_t1sData.ml_confidence = 0;
                app_t1sData.auto_state = AUTO_STATE_GATE_OPEN;
            }
            break;
        }

        case AUTO_STATE_CLASSIFY_WAIT:
        {
            if (APP_ML_IsResultReady())
            {
                app_t1sData.ml_classification = APP_ML_GetClassificationResult();
                app_t1sData.ml_confidence = APP_ML_GetConfidence();
                
                APP_ML_AcknowledgeResult();
                
                app_t1sData.auto_state = AUTO_STATE_GATE_OPEN;
            }
            
            else if (APP_T1S_TimeoutCheck(AUTO_TIMEOUT_MS))
            {
                APP_ML_AcknowledgeResult();
                
                app_t1sData.auto_state = AUTO_STATE_HOME_CHECK;
            }
            break;
        }

        case AUTO_STATE_GATE_OPEN:
        {
            if (!APP_T1S_ClientsReady())
            {
                app_t1sData.auto_state = AUTO_STATE_HOME_CHECK;
                break;
            }

            if (app_t1sData.ml_classification == CLASS_CIRCLE)
            {
                app_t1sData.client0_gate = 1;
            }
            
            else if (app_t1sData.ml_classification == CLASS_TRIANGLE)
            {
                app_t1sData.client1_gate = 1;
            }
            
            APP_T1S_TimeoutReset();
            
            app_t1sData.auto_state = AUTO_STATE_GATE_OPEN_WAIT;
            break;
        }

        case AUTO_STATE_GATE_OPEN_WAIT:
        {
            bool gateOpen = false;

            if (app_t1sData.ml_classification == CLASS_CIRCLE)
            {
                if (!app_t1sData.client0_ready)
                {
                    app_t1sData.client0_gate = 0;
                    app_t1sData.auto_state = AUTO_STATE_HOME_CHECK;
                    break;
                }
                
                gateOpen = (app_t1sData.client0_gateStatus == 1);
            }
            
            else if (app_t1sData.ml_classification == CLASS_TRIANGLE)
            {
                if (!app_t1sData.client1_ready)
                {
                    app_t1sData.client1_gate = 0;
                    app_t1sData.auto_state = AUTO_STATE_HOME_CHECK;
                    break;
                }
                
                gateOpen = (app_t1sData.client1_gateStatus == 1);
            }
            
            else
            {
                gateOpen = true;
            }

            if (gateOpen)
            {
                app_t1sData.auto_state = AUTO_STATE_DROP;
            }
            
            else if (APP_T1S_TimeoutCheck(AUTO_TIMEOUT_MS))
            {
                app_t1sData.client0_gate = 0;
                app_t1sData.client1_gate = 0;
                app_t1sData.auto_state = AUTO_STATE_HOME_CHECK;
            }
            
            break;
        }

        case AUTO_STATE_DROP:
        {
            APP_MOTOR_DropObject();
            APP_T1S_TimeoutReset();
            
            app_t1sData.auto_state = AUTO_STATE_DROP_WAIT;
            break;
        }

        case AUTO_STATE_DROP_WAIT:
        {
            if (APP_MOTOR_IsDropComplete())
            {
                app_t1sData.auto_state = AUTO_STATE_CONVEYOR_START;
            }
           
            else if (APP_T1S_TimeoutCheck(AUTO_TIMEOUT_MS))
            {
                app_t1sData.auto_state = AUTO_STATE_CONVEYOR_START;
            }
            break;
        }

        case AUTO_STATE_CONVEYOR_START:
        {
            app_t1sData.conveyor_state = 1;
            app_t1sData.conveyor_speed = 100;
            app_t1sData.client0_objectPassed = 0;
            app_t1sData.client1_objectPassed = 0;

            if (app_t1sData.ml_classification == CLASS_UNKNOWN)
            {
                app_t1sData.unknown_count++;
               
                APP_MOTOR_Conveyor_Start(100, 1);
                APP_T1S_TimeoutReset();
                
                app_t1sData.auto_state = AUTO_STATE_CONVEYOR_REVERSE;
            }
            
            else
            {
                app_t1sData.unknown_count = 0;
               
                APP_MOTOR_Conveyor_Start(100, 0);
                APP_T1S_TimeoutReset();
                
                app_t1sData.auto_state = AUTO_STATE_OBJECT_WAIT;
            }
            
            break;
        }

        case AUTO_STATE_OBJECT_WAIT:
        {
            uint32_t timeout_ms = AUTO_TIMEOUT_MS;
            bool timeout_elapsed = false;

            if (app_t1sData.ml_classification == CLASS_CIRCLE)
            {
                timeout_ms = AUTO_OBJECT_WAIT_CIRCLE;
                timeout_elapsed = APP_T1S_TimeoutCheck(timeout_ms);
                
                if (timeout_elapsed)
                {
                    app_t1sData.circle_count++;
                }
            }
            
            else if (app_t1sData.ml_classification == CLASS_TRIANGLE)
            {
                timeout_ms = AUTO_OBJECT_WAIT_TRIANGLE;
                timeout_elapsed = APP_T1S_TimeoutCheck(timeout_ms);
                
                if (timeout_elapsed)
                {
                    app_t1sData.triangle_count++;
                }
            }
            
            else if (app_t1sData.ml_classification == CLASS_SQUARE)
            {
                timeout_ms = AUTO_OBJECT_WAIT_SQUARE;
                timeout_elapsed = APP_T1S_TimeoutCheck(timeout_ms);
               
                if (timeout_elapsed)
                {
                    app_t1sData.square_count++;
                }
            }

            if (timeout_elapsed)
            {
                app_t1sData.auto_state = AUTO_STATE_GATE_CLOSE;
            }
            
            break;
        }

        case AUTO_STATE_SQUARE_BIN_WAIT:
        {
            if (APP_T1S_TimeoutCheck(AUTO_BIN_WAIT_MS))
            {
                app_t1sData.auto_state = AUTO_STATE_GATE_CLOSE;
            }
            
            break;
        }

        case AUTO_STATE_CONVEYOR_REVERSE:
        {
            if (APP_T1S_TimeoutCheck(AUTO_REVERSE_WAIT_MS))
            {
                APP_MOTOR_Conveyor_Stop();
                
                app_t1sData.conveyor_state = 0;

                if (app_t1sData.unknown_count >= AUTO_MAX_UNKNOWN)
                {
                    app_t1sData.unknown_count = 0;
                    app_t1sData.capture_active = false;
                    app_t1sData.auto_state = AUTO_STATE_HOME_CHECK;
                }
                
                else
                {
                    app_t1sData.auto_state = AUTO_STATE_HOME_CHECK;
                }
            }
            break;
        }

        case AUTO_STATE_GATE_CLOSE:
        {
            app_t1sData.client0_gate = 0;
            app_t1sData.client1_gate = 0;
            
            APP_T1S_TimeoutReset();
            
            app_t1sData.auto_state = AUTO_STATE_GATE_CLOSE_WAIT;
            break;
        }

        case AUTO_STATE_GATE_CLOSE_WAIT:
        {
            bool gateClosed = (app_t1sData.client0_gateStatus == 0) &&
                              (app_t1sData.client1_gateStatus == 0);

            if (gateClosed)
            {
                app_t1sData.auto_state = AUTO_STATE_CONVEYOR_STOP;
            }
            
            else if (APP_T1S_TimeoutCheck(AUTO_TIMEOUT_MS))
            {
                app_t1sData.auto_state = AUTO_STATE_CONVEYOR_STOP;
            }
            
            break;
        }

        case AUTO_STATE_CONVEYOR_STOP:
        {
            APP_MOTOR_Conveyor_Stop();
            
            app_t1sData.conveyor_state = 0;
            app_t1sData.auto_state = AUTO_STATE_HOME_CHECK;
            break;
        }

        default:
            app_t1sData.auto_state = AUTO_STATE_HOME_CHECK;
            break;
    }
}


/*
 * Function: APP_T1S_GUI_Handle
 * ----------------------------
 * Handles GUI TCP communication for control and status.
 *
 * Parameters:
 *   None.
 *
 * Returns:
 *   None.
 */
static void APP_T1S_GUI_Handle(void)
{
    uint8_t guiRxBuffer[T1S_GUI_DATA_SIZE];
    uint8_t guiTxBuffer[T1S_GUI_DATA_SIZE];

    if (app_t1sData.gui_socket == INVALID_SOCKET)
    {
        app_t1sData.gui_socket = TCPIP_TCP_ServerOpen(IP_ADDRESS_TYPE_IPV4, T1S_GUI_PORT, 0);
        return;
    }

    if (!TCPIP_TCP_IsConnected(app_t1sData.gui_socket) ||
        (app_t1sData.gui_ready && TCPIP_TCP_WasDisconnected(app_t1sData.gui_socket)))
    {
        if (app_t1sData.gui_ready)
        {
            TCPIP_TCP_Discard(app_t1sData.gui_socket);
            TCPIP_TCP_Close(app_t1sData.gui_socket);
            
            app_t1sData.gui_socket = INVALID_SOCKET;
            app_t1sData.gui_ready = false;
            app_t1sData.mode = 0;
            app_t1sData.capture_active = false;
            app_t1sData.ml_result_pending = false;
            app_t1sData.auto_state = AUTO_STATE_HOME_CHECK;

            APP_T1S_StopAllMotors();
        }
        
        return;
    }

    app_t1sData.gui_ready = true;

    if (TCPIP_TCP_GetIsReady(app_t1sData.gui_socket) >= T1S_GUI_DATA_SIZE)
    {
        TCPIP_TCP_ArrayGet(app_t1sData.gui_socket, guiRxBuffer, T1S_GUI_DATA_SIZE);

        uint8_t cmd = guiRxBuffer[0];

        switch (cmd)
        {
            case GUI_CMD_MODE:
            {
                app_t1sData.mode = guiRxBuffer[1];
                if (app_t1sData.mode == 0)
                {
                    app_t1sData.ml_result_pending = false;
                }
                break;
            }
            
            case GUI_CMD_CONVEYOR:
            {
                if (app_t1sData.mode == 1)
                {
                    uint8_t state = guiRxBuffer[1];
                    uint8_t speed = guiRxBuffer[2];
                    uint8_t direction = guiRxBuffer[3];

                    if (state && !app_t1sData.conveyor_state)
                    {
                        app_t1sData.conveyor_speed = (speed > 0) ? speed : 100;
                        app_t1sData.conveyor_direction = direction;
                        
                        APP_MOTOR_Conveyor_Start(app_t1sData.conveyor_speed, app_t1sData.conveyor_direction);
                    }
                    
                    else if (!state && app_t1sData.conveyor_state)
                    {
                        APP_MOTOR_Conveyor_Stop();
                    }
                    
                    else if (state && app_t1sData.conveyor_state &&
                             (speed != app_t1sData.conveyor_speed || direction != app_t1sData.conveyor_direction))
                    {
                        app_t1sData.conveyor_speed = speed;
                        app_t1sData.conveyor_direction = direction;
                        
                        APP_MOTOR_Conveyor_SetSpeed(speed, direction);
                    }
                    
                    app_t1sData.conveyor_state = state;
                }
                
                break;
            }

            case GUI_CMD_DISPENSER:
            {
                if (app_t1sData.mode == 1)
                {
                    uint8_t state = guiRxBuffer[1];
                    uint8_t speed = guiRxBuffer[2];
                    uint8_t direction = guiRxBuffer[3];

                    if (state && !app_t1sData.dispenser_state)
                    {
                        app_t1sData.dispenser_speed = (speed > 0) ? speed : 50;
                        app_t1sData.dispenser_direction = direction;
                        
                        APP_MOTOR_Dispenser_Start(app_t1sData.dispenser_speed, app_t1sData.dispenser_direction);
                    }
                    
                    else if (!state && app_t1sData.dispenser_state)
                    {
                        APP_MOTOR_Dispenser_Stop();
                    }
                    
                    else if (state && app_t1sData.dispenser_state &&
                             (speed != app_t1sData.dispenser_speed || direction != app_t1sData.dispenser_direction))
                    {
                        app_t1sData.dispenser_speed = speed;
                        app_t1sData.dispenser_direction = direction;
                        
                        APP_MOTOR_Dispenser_SetSpeed(speed, direction);
                    }
                    
                    app_t1sData.dispenser_state = state;
                }
                
                break;
            }

            case GUI_CMD_DROP:
            {
                if (app_t1sData.mode == 1)
                {
                    APP_MOTOR_DropObject();
                }
                
                break;
            }
            
            case GUI_CMD_GATE:
            {
                if (app_t1sData.mode == 1)
                {
                    uint8_t client_id = guiRxBuffer[1];
                    uint8_t gate_state = guiRxBuffer[2];

                    if (client_id == 0)
                    {
                        app_t1sData.client0_gate = gate_state;
                    
                    }
                    
                    else
                    {
                        app_t1sData.client1_gate = gate_state;
                    }
                }
                break;
            }

            case GUI_CMD_CLASSIFY:
            {
                if (app_t1sData.mode == 1)
                {
                    uint8_t class_type = guiRxBuffer[1];

                    switch (class_type)
                    {
                        case CLASS_CIRCLE:
                            app_t1sData.client0_gate = 1;
                            break;
                        case CLASS_TRIANGLE:
                            app_t1sData.client1_gate = 1;
                            break;
                        case CLASS_SQUARE:
                        case CLASS_UNKNOWN:
                        default:
                            break;
                    }
                }
                break;
            }

            case GUI_CMD_CAPTURE:
            {
                if (app_t1sData.capture_active)
                {
                    app_t1sData.capture_active = false;
                    app_t1sData.ml_result_pending = false;
                    app_t1sData.auto_state = AUTO_STATE_HOME_CHECK;
                }
                
                else
                {
                    app_t1sData.capture_active = true;
                    app_t1sData.unknown_count = 0;
                    app_t1sData.auto_state = AUTO_STATE_HOME_CHECK;
                    
                    if (app_t1sData.mode == 1 && APP_ML_IsReady() && !app_t1sData.ml_result_pending)
                    {
                        APP_ML_TriggerInference();
                        app_t1sData.ml_result_pending = true;
                    }
                }
                
                break;
            }

            default:
                break;
        }

        if (app_t1sData.mode == 1 && app_t1sData.ml_result_pending && APP_ML_IsResultReady())
        {
            app_t1sData.ml_classification = APP_ML_GetClassificationResult();
            app_t1sData.ml_confidence = APP_ML_GetConfidence();
            APP_ML_AcknowledgeResult();
            app_t1sData.ml_result_pending = false;
        }

        memset(guiTxBuffer, 0, sizeof(guiTxBuffer));

        guiTxBuffer[0] = 1;
        guiTxBuffer[1] = app_t1sData.gui_seq_number++;
        guiTxBuffer[2] = app_t1sData.mode;

        guiTxBuffer[3] = app_t1sData.client0_ready ? 1 : 0;
        guiTxBuffer[4] = app_t1sData.client0_gateControl;
        guiTxBuffer[5] = app_t1sData.client0_gateStatus;
        guiTxBuffer[6] = (uint8_t)(app_t1sData.client0_objectCount & 0xFF);

        guiTxBuffer[7] = app_t1sData.client1_ready ? 1 : 0;
        guiTxBuffer[8] = app_t1sData.client1_gateControl;
        guiTxBuffer[9] = app_t1sData.client1_gateStatus;
        guiTxBuffer[10] = (uint8_t)(app_t1sData.client1_objectCount & 0xFF);

        guiTxBuffer[11] = app_t1sData.conveyor_state;
        guiTxBuffer[12] = app_t1sData.conveyor_speed;
        guiTxBuffer[13] = app_t1sData.dispenser_state;
        guiTxBuffer[14] = app_t1sData.dispenser_speed;

        guiTxBuffer[15] = APP_ML_IsReady() ? 1 : 0;
        guiTxBuffer[16] = app_t1sData.ml_result_pending ? 1 : 0;
        guiTxBuffer[17] = app_t1sData.ml_classification;
        guiTxBuffer[18] = app_t1sData.ml_confidence;
        guiTxBuffer[19] = (uint8_t)(app_t1sData.circle_count & 0xFF);
        guiTxBuffer[20] = (uint8_t)(app_t1sData.triangle_count & 0xFF);
        guiTxBuffer[21] = (uint8_t)(app_t1sData.square_count & 0xFF);
        guiTxBuffer[22] = app_t1sData.capture_active ? 1 : 0;

        if (TCPIP_TCP_PutIsReady(app_t1sData.gui_socket) >= T1S_GUI_DATA_SIZE)
        {
            TCPIP_TCP_ArrayPut(app_t1sData.gui_socket, guiTxBuffer, T1S_GUI_DATA_SIZE);
        }
    }
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_T1S_Initialize ( void )

  Remarks:
    See prototype in app_t1s.h.
 */

void APP_T1S_Initialize(void)
{
    app_t1sData.state = APP_T1S_STATE_INIT;
    app_t1sData.host_state = APP_T1S_HOST_SEND_READ_CLIENT0;
    app_t1sData.auto_state = AUTO_STATE_HOME_CHECK;
    app_t1sData.gui_socket = INVALID_SOCKET;
    app_t1sData.client0_socket = INVALID_SOCKET;
    app_t1sData.client1_socket = INVALID_SOCKET;
    app_t1sData.taskReady = false;
    app_t1sData.gui_ready = false;
    app_t1sData.client0_ready = false;
    app_t1sData.client1_ready = false;
    app_t1sData.ipAcquired = false;
    app_t1sData.ipAddr.Val = 0;
    app_t1sData.netH = NULL;
    app_t1sData.nNets = 0;

    app_t1sData.client0_gateControl = 0;
    app_t1sData.client0_gateStatus = 0;
    app_t1sData.client0_objectCount = 0;
    app_t1sData.client0_objectPassed = 0;
    app_t1sData.client1_gateControl = 0;
    app_t1sData.client1_gateStatus = 0;
    app_t1sData.client1_objectCount = 0;
    app_t1sData.client1_objectPassed = 0;

    app_t1sData.mode = 0;
    app_t1sData.conveyor_state = 0;
    app_t1sData.conveyor_speed = 0;
    app_t1sData.conveyor_direction = 0;
    app_t1sData.dispenser_state = 0;
    app_t1sData.dispenser_speed = 0;
    app_t1sData.dispenser_direction = 0;
    app_t1sData.client0_gate = 0;
    app_t1sData.client1_gate = 0;

    app_t1sData.ml_classification = CLASS_UNKNOWN;
    app_t1sData.ml_confidence = 0;
    app_t1sData.ml_ready = false;
    app_t1sData.ml_result_pending = false;
    app_t1sData.capture_active = false;

    app_t1sData.circle_count = 0;
    app_t1sData.triangle_count = 0;
    app_t1sData.square_count = 0;
    app_t1sData.unknown_count = 0;

    app_t1sData.auto_timeout_start = 0;
    app_t1sData.gui_seq_number = 0;
}


/******************************************************************************
  Function:
    void APP_T1S_Tasks ( void )

  Remarks:
    See prototype in app_t1s.h.
 */

void APP_T1S_Tasks(void)
{
    switch (app_t1sData.state)
    {
        case APP_T1S_STATE_INIT:
        {
            if (SYS_STATUS_READY == TCPIP_STACK_Status(sysObj.tcpip))
            {
                app_t1sData.state = APP_T1S_STATE_TCPIP_WAIT_FOR_IP;
            }
            else if (SYS_STATUS_UNINITIALIZED > TCPIP_STACK_Status(sysObj.tcpip))
            {
                app_t1sData.state = APP_T1S_STATE_ERROR;
            }
            break;
        }

        case APP_T1S_STATE_TCPIP_WAIT_FOR_IP:
        {
            app_t1sData.nNets = TCPIP_STACK_NumberOfNetworksGet();

            for (uint32_t i = 0; i < app_t1sData.nNets; i++)
            {
                app_t1sData.netH = TCPIP_STACK_IndexToNet(i);

                if (!TCPIP_STACK_NetIsReady(app_t1sData.netH))
                    break;

                if (!app_t1sData.ipAcquired)
                {
                    app_t1sData.ipAddr.Val = TCPIP_STACK_NetAddress(app_t1sData.netH);
                    SYS_CONSOLE_PRINT("Host IP Address: %d.%d.%d.%d\r\n",
                            app_t1sData.ipAddr.v[0], app_t1sData.ipAddr.v[1],
                            app_t1sData.ipAddr.v[2], app_t1sData.ipAddr.v[3]);
                    app_t1sData.ipAcquired = true;
                }

                app_t1sData.state = APP_T1S_STATE_CLIENT_CONNECT;
                break;
            }
            break;
        }

        case APP_T1S_STATE_CLIENT_CONNECT:
        {
            if (app_t1sData.gui_socket == INVALID_SOCKET)
            {
                app_t1sData.gui_socket = TCPIP_TCP_ServerOpen(IP_ADDRESS_TYPE_IPV4, T1S_GUI_PORT, 0);
            }

            if (app_t1sData.gui_socket != INVALID_SOCKET)
            {
                APP_T1S_GUI_Handle();

                if (app_t1sData.client0_socket == INVALID_SOCKET)
                {
                    app_t1sData.client0_socket = TCPIP_TCP_ServerOpen(IP_ADDRESS_TYPE_IPV4, T1S_CLIENT0_PORT, 0);
                }

                if (app_t1sData.client0_socket != INVALID_SOCKET)
                {
                    if (app_t1sData.client1_socket == INVALID_SOCKET)
                    {
                        app_t1sData.client1_socket = TCPIP_TCP_ServerOpen(IP_ADDRESS_TYPE_IPV4, T1S_CLIENT1_PORT, 0);
                    }

                    if (app_t1sData.client1_socket != INVALID_SOCKET)
                    {
                        app_t1sData.state = APP_T1S_STATE_CLIENT_ACKNOWLEDGE;
                    }
                }
            }
            break;
        }

        case APP_T1S_STATE_CLIENT_ACKNOWLEDGE:
        {
            app_t1sData.client0_ready = TCPIP_TCP_IsConnected(app_t1sData.client0_socket);
            app_t1sData.client1_ready = TCPIP_TCP_IsConnected(app_t1sData.client1_socket);

            if (app_t1sData.client0_ready && !app_t1sData.taskReady)
                LED0_Clear();

            if (app_t1sData.client1_ready && !app_t1sData.taskReady)
                LED1_Clear();

            APP_T1S_GUI_Handle();

            if (app_t1sData.client0_ready && app_t1sData.client1_ready)
            {
                app_t1sData.taskReady = true;
                app_t1sData.state = APP_T1S_STATE_CLIENT_WRITE_READ;
            }
            break;
        }

        case APP_T1S_STATE_CLIENT_WRITE_READ:
        {
            bool client0_connected = (app_t1sData.client0_socket != INVALID_SOCKET) &&
                                     TCPIP_TCP_IsConnected(app_t1sData.client0_socket);
            bool client1_connected = (app_t1sData.client1_socket != INVALID_SOCKET) &&
                                     TCPIP_TCP_IsConnected(app_t1sData.client1_socket);

            /* Handle Client0 disconnect */
            if (!client0_connected && app_t1sData.client0_ready)
            {
                SYS_CONSOLE_PRINT("Host: Client0 disconnected, closing socket\r\n");
                if (app_t1sData.client0_socket != INVALID_SOCKET)
                {
                    TCPIP_TCP_Close(app_t1sData.client0_socket);
                    app_t1sData.client0_socket = INVALID_SOCKET;
                }
                app_t1sData.client0_ready = false;
                app_t1sData.client0_gate = 0;
            }

            /* Handle Client1 disconnect */
            if (!client1_connected && app_t1sData.client1_ready)
            {
                SYS_CONSOLE_PRINT("Host: Client1 disconnected, closing socket\r\n");
                if (app_t1sData.client1_socket != INVALID_SOCKET)
                {
                    TCPIP_TCP_Close(app_t1sData.client1_socket);
                    app_t1sData.client1_socket = INVALID_SOCKET;
                }
                app_t1sData.client1_ready = false;
                app_t1sData.client1_gate = 0;
            }

            /* Reopen Client0 socket if needed */
            if (app_t1sData.client0_socket == INVALID_SOCKET)
            {
                app_t1sData.client0_socket = TCPIP_TCP_ServerOpen(IP_ADDRESS_TYPE_IPV4, T1S_CLIENT0_PORT, 0);
                if (app_t1sData.client0_socket != INVALID_SOCKET)
                {
                    SYS_CONSOLE_PRINT("Host: Client0 server socket reopened\r\n");
                }
            }

            /* Reopen Client1 socket if needed */
            if (app_t1sData.client1_socket == INVALID_SOCKET)
            {
                app_t1sData.client1_socket = TCPIP_TCP_ServerOpen(IP_ADDRESS_TYPE_IPV4, T1S_CLIENT1_PORT, 0);
                if (app_t1sData.client1_socket != INVALID_SOCKET)
                {
                    SYS_CONSOLE_PRINT("Host: Client1 server socket reopened\r\n");
                }
            }

            /* Check for client reconnection */
            if (!app_t1sData.client0_ready && app_t1sData.client0_socket != INVALID_SOCKET &&
                TCPIP_TCP_IsConnected(app_t1sData.client0_socket))
            {
                app_t1sData.client0_ready = true;
                SYS_CONSOLE_PRINT("Host: Client0 reconnected\r\n");
            }

            if (!app_t1sData.client1_ready && app_t1sData.client1_socket != INVALID_SOCKET &&
                TCPIP_TCP_IsConnected(app_t1sData.client1_socket))
            {
                app_t1sData.client1_ready = true;
                SYS_CONSOLE_PRINT("Host: Client1 reconnected\r\n");
            }

            APP_T1S_GUI_Handle();
            APP_T1S_Auto_Handle();
            APP_T1S_Data_Handle();

            break;
        }

        case APP_T1S_STATE_ERROR:
        {
            break;
        }

        default:
            break;
    }
}
