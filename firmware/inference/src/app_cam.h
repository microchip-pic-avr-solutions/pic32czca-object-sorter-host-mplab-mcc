/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app_cam.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APP_CAM_Initialize" and "APP_CAM_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_CAM_STATES" definition).  Both
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

#ifndef _APP_CAM_H
#define _APP_CAM_H

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
#include "arducam.h"

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

#define CAP_IMG_WIDTH   96
#define CAP_IMG_HEIGHT  96

#define CAP_BUF_SIZE    (CAP_IMG_WIDTH * CAP_IMG_HEIGHT * 2) + 1
#define GRAY_BUF_SIZE   (CAP_IMG_WIDTH * CAP_IMG_HEIGHT)

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
    APP_CAM_STATE_IDLE = 0,
    APP_CAM_STATE_INIT,
    APP_CAM_STATE_CAMERA_RESET,
    APP_CAM_STATE_CHECK_CAMERA,
    APP_CAM_STATE_SET_BMP_FORMAT,
    APP_CAM_STATE_SET_RESOLUTION,
    APP_CAM_STATE_SERVICE_TASKS,
    APP_CAM_STATE_IMG_CAPTURE,
    APP_CAM_STATE_CAPTURE_WAIT,
    APP_CAM_STATE_IMG_READ,
    APP_CAM_STATE_IMG_READY,
    APP_CAM_STATE_ERROR
} APP_CAM_STATES;


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
    APP_CAM_STATES state;

    SYS_CONSOLE_HANDLE consoleHandle;

    DRV_SPI_TRANSFER_SETUP spiSetup;

    DRV_HANDLE spiHandle;

    DRV_SPI_TRANSFER_HANDLE transferHandle;

    volatile bool isTransferComplete;
    volatile bool transferStatus;
    volatile bool cam_ready;
    volatile bool image_ready;

    uint8_t writeReg[2];
    uint8_t readReg[4];

    uint8_t captureBuffer[CAP_BUF_SIZE];
    uint8_t grayscaleBuffer[GRAY_BUF_SIZE];

    uint32_t imageSize;
} APP_CAM_DATA;

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
    void APP_CAM_Initialize ( void )

  Summary:
     MPLAB Harmony application initialization routine.

  Description:
    This function initializes the Harmony application.  It places the
    application in its initial state and prepares it to run so that its
    APP_CAM_Tasks function can be called.

  Precondition:
    All other system initialization routines should be called before calling
    this routine (in "SYS_Initialize").

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    APP_CAM_Initialize();
    </code>

  Remarks:
    This routine must be called from the SYS_Initialize function.
*/

extern APP_CAM_DATA app_camData;

void APP_CAM_Initialize(void);
bool APP_CAM_TaskIsReady(void);
bool APP_CAM_ImageIsReady(void);
uint8_t* APP_CAM_GetGrayscaleBuffer(void);
void APP_CAM_TriggerCapture(void);
void APP_CAM_Task_State_Set(int task_state);
int APP_CAM_Get_Service_Tasks_State(void);

/*******************************************************************************
  Function:
    void APP_CAM_Tasks ( void )

  Summary:
    MPLAB Harmony Demo application tasks function

  Description:
    This routine is the Harmony Demo application's tasks function.  It
    defines the application's state machine and core logic.

  Precondition:
    The system and application initialization ("SYS_Initialize") should be
    called before calling this.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    APP_CAM_Tasks();
    </code>

  Remarks:
    This routine must be called from SYS_Tasks() routine.
 */

void APP_CAM_Tasks(void);

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* _APP_CAM_H */

/*******************************************************************************
 End of File
 */
