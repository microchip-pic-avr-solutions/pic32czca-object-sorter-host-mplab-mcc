/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app_t1s.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APP_T1S_Initialize" and "APP_T1S_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_T1S_STATES" definition).  Both
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

#ifndef _APP_T1S_H
#define _APP_T1S_H

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
#include "app_ml.h"

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

#define T1S_GUI_PORT        9760
#define T1S_CLIENT0_PORT    9761
#define T1S_CLIENT1_PORT    9762

#define T1S_TX_DATA_SIZE    8
#define T1S_RX_DATA_SIZE    8
#define T1S_GUI_DATA_SIZE   24

#define T1S_DATA_CMD        0x11
#define T1S_DATA_ACK        0x12

#define GUI_CMD_MODE        0x10
#define GUI_CMD_CONVEYOR    0x20
#define GUI_CMD_DISPENSER   0x30
#define GUI_CMD_DROP        0x40
#define GUI_CMD_GATE        0x50
#define GUI_CMD_CLASSIFY    0x60
#define GUI_CMD_CAPTURE     0x70

#define CLASS_SQUARE        0x01
#define CLASS_TRIANGLE      0x02
#define CLASS_CIRCLE        0x03
#define CLASS_UNKNOWN       0x04

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
    APP_T1S_STATE_INIT = 0,
    APP_T1S_STATE_TCPIP_WAIT_FOR_IP,
    APP_T1S_STATE_CLIENT_CONNECT,
    APP_T1S_STATE_CLIENT_ACKNOWLEDGE,
    APP_T1S_STATE_CLIENT_WRITE_READ,
    APP_T1S_STATE_ERROR
} APP_T1S_STATES;

typedef enum
{
    APP_T1S_HOST_SEND_READ_CLIENT0,
    APP_T1S_HOST_WAIT_REPLY_CLIENT0,
    APP_T1S_HOST_SEND_READ_CLIENT1,
    APP_T1S_HOST_WAIT_REPLY_CLIENT1
} APP_T1S_HOST_STATE;

typedef enum
{
    AUTO_STATE_HOME_CHECK = 0,
    AUTO_STATE_HOME_WAIT,
    AUTO_STATE_FORWARD,
    AUTO_STATE_FORWARD_WAIT,
    AUTO_STATE_HOME_CHECK_2,
    AUTO_STATE_HOME_WAIT_2,
    AUTO_STATE_CAPTURE,
    AUTO_STATE_CLASSIFY_WAIT,
    AUTO_STATE_GATE_OPEN,
    AUTO_STATE_GATE_OPEN_WAIT,
    AUTO_STATE_DROP,
    AUTO_STATE_DROP_WAIT,
    AUTO_STATE_CONVEYOR_START,
    AUTO_STATE_OBJECT_WAIT,
    AUTO_STATE_SQUARE_BIN_WAIT,
    AUTO_STATE_CONVEYOR_REVERSE,
    AUTO_STATE_GATE_CLOSE,
    AUTO_STATE_GATE_CLOSE_WAIT,
    AUTO_STATE_CONVEYOR_STOP
} AUTO_MODE_STATE;

#define AUTO_TIMEOUT_MS           30000
#define AUTO_OBJECT_WAIT_CIRCLE   2500
#define AUTO_OBJECT_WAIT_TRIANGLE 6000
#define AUTO_OBJECT_WAIT_SQUARE   10000
#define AUTO_BIN_WAIT_MS          3000
#define AUTO_REVERSE_WAIT_MS      3000
#define AUTO_MAX_UNKNOWN          3


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
    APP_T1S_STATES state;
    APP_T1S_HOST_STATE host_state;
    AUTO_MODE_STATE auto_state;

    TCP_SOCKET gui_socket;
    TCP_SOCKET client0_socket;
    TCP_SOCKET client1_socket;

    volatile bool ipAcquired;
    volatile bool taskReady;
    volatile bool gui_ready;
    volatile bool client0_ready;
    volatile bool client1_ready;

    uint16_t client0_objectCount;
    uint16_t client1_objectCount;
    uint8_t client0_gateControl;
    uint8_t client0_gateStatus;
    uint8_t client0_objectPassed;
    uint8_t client1_gateControl;
    uint8_t client1_gateStatus;
    uint8_t client1_objectPassed;

    uint8_t mode;
    uint8_t conveyor_state;
    uint8_t conveyor_speed;
    uint8_t conveyor_direction;
    uint8_t dispenser_state;
    uint8_t dispenser_speed;
    uint8_t dispenser_direction;
    uint8_t client0_gate;
    uint8_t client1_gate;

    uint8_t ml_classification;
    uint8_t ml_confidence;
    volatile bool ml_ready;
    volatile bool ml_result_pending;
    volatile bool capture_active;

    uint16_t circle_count;
    uint16_t triangle_count;
    uint16_t square_count;
    uint8_t unknown_count;

    uint32_t auto_timeout_start;
    uint8_t gui_seq_number;

    IPV4_ADDR ipAddr;
    TCPIP_NET_HANDLE netH;
    uint32_t nNets;
} APP_T1S_DATA;

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
    void APP_T1S_Initialize ( void )

  Summary:
     MPLAB Harmony application initialization routine.

  Description:
    This function initializes the Harmony application.  It places the
    application in its initial state and prepares it to run so that its
    APP_T1S_Tasks function can be called.

  Precondition:
    All other system initialization routines should be called before calling
    this routine (in "SYS_Initialize").

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    APP_T1S_Initialize();
    </code>

  Remarks:
    This routine must be called from the SYS_Initialize function.
*/

void APP_T1S_Initialize(void);
void APP_T1S_Tasks(void);

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* _APP_T1S_H */

/*******************************************************************************
 End of File
 */
