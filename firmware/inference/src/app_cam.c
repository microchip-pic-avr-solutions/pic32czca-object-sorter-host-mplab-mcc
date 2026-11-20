/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_cam.c

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

#include "app_cam.h"
#include "system/console/sys_console.h"

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
    This structure should be initialized by the APP_CAM_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

APP_CAM_DATA app_camData;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/*
 * Function: APP_CAM_SPI_Event_Handler
 * -----------------------------------
 * SPI transfer event callback handler for the camera interface.
 * Sets the transfer completion flag or updates the state on error.
 *
 * Parameters:
 *   event          - The SPI transfer event type.
 *   transferHandle - The handle identifying the SPI transfer instance.
 *   context        - User-defined context value (not used).
 *
 * Returns:
 *   None.
 */
static void APP_CAM_SPI_Event_Handler(
    DRV_SPI_TRANSFER_EVENT event,
    DRV_SPI_TRANSFER_HANDLE transferHandle,
    uintptr_t context)
{
    if (event == DRV_SPI_TRANSFER_EVENT_COMPLETE)
    {
        app_camData.isTransferComplete = true;
    }
    else
    {
        app_camData.isTransferComplete = false;
        app_camData.state = APP_CAM_STATE_ERROR;
    }
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************

/*
 * Function: APP_CAM_Task_State_Set
 * --------------------------------
 * Sets the current state of the camera application's state machine.
 *
 * Parameters:
 *   task_state - The new state to set for the camera application.
 *
 * Returns:
 *   None.
 */
void APP_CAM_Task_State_Set(int task_state)
{
    app_camData.state = (APP_CAM_STATES)task_state;
}


/*
 * Function: APP_CAM_Get_Service_Tasks_State
 * -----------------------------------------
 * Gets the service tasks state value.
 *
 * Parameters:
 *   None.
 *
 * Returns:
 *   The service tasks state value.
 */
int APP_CAM_Get_Service_Tasks_State(void)
{
    return (int)APP_CAM_STATE_SERVICE_TASKS;
}


/*
 * Function: APP_CAM_TaskIsReady
 * -----------------------------
 * Checks if the camera task is ready to process.
 *
 * Parameters:
 *   None.
 *
 * Returns:
 *   true if the camera task is ready, false otherwise.
 */
bool APP_CAM_TaskIsReady(void)
{
    return app_camData.cam_ready;
}


/*
 * Function: APP_CAM_ImageIsReady
 * ------------------------------
 * Checks if the camera image is ready for processing or retrieval.
 *
 * Parameters:
 *   None.
 *
 * Returns:
 *   true if the image is ready, false otherwise.
 */
bool APP_CAM_ImageIsReady(void)
{
    return app_camData.image_ready;
}


/*
 * Function: APP_CAM_GetGrayscaleBuffer
 * ------------------------------------
 * Gets pointer to the grayscale image buffer.
 *
 * Parameters:
 *   None.
 *
 * Returns:
 *   Pointer to the grayscale buffer.
 */
uint8_t* APP_CAM_GetGrayscaleBuffer(void)
{
    return app_camData.grayscaleBuffer;
}


/*
 * Function: APP_CAM_TriggerCapture
 * --------------------------------
 * Triggers a new image capture if camera is ready.
 *
 * Parameters:
 *   None.
 *
 * Returns:
 *   None.
 */
void APP_CAM_TriggerCapture(void)
{
    if (app_camData.cam_ready && app_camData.state == APP_CAM_STATE_SERVICE_TASKS)
    {
        app_camData.image_ready = false;
        app_camData.state = APP_CAM_STATE_IMG_CAPTURE;
    }
}


/*
 * Function: APP_CAM_DelayMS
 * -------------------------
 * Introduces a blocking delay for the specified number of milliseconds.
 *
 * Parameters:
 *   ms - The number of milliseconds to delay.
 *
 * Returns:
 *   None.
 */
static void APP_CAM_DelayMS(int ms)
{
    SYS_TIME_HANDLE timer = SYS_TIME_HANDLE_INVALID;

    if (SYS_TIME_DelayMS(ms, &timer) != SYS_TIME_SUCCESS)
        return;

    while (SYS_TIME_DelayIsComplete(timer) == false);
}


/*
 * Function: APP_CAM_Camera_Write_Reg
 * ----------------------------------
 * Writes a value to a camera register over SPI.
 *
 * Parameters:
 *   reg  - The camera register address to write to.
 *   data - The data byte to write to the register.
 *
 * Returns:
 *   true if the write transfer is successful.
 */
static bool APP_CAM_Camera_Write_Reg(uint8_t reg, uint8_t data)
{
    app_camData.writeReg[0] = reg | 0x80;
    app_camData.writeReg[1] = data;

    app_camData.isTransferComplete = false;

    DRV_SPI_WriteTransferAdd(app_camData.spiHandle,
            app_camData.writeReg, 2, &app_camData.transferHandle);

    APP_CAM_DelayMS(1);

    while (!app_camData.isTransferComplete);

    return true;
}


/*
 * Function: APP_CAM_Camera_Read_Reg
 * ---------------------------------
 * Reads a value from a camera register over SPI.
 *
 * Parameters:
 *   reg - The camera register address to read from.
 *
 * Returns:
 *   The byte value read from the specified register.
 */
static uint8_t APP_CAM_Camera_Read_Reg(uint8_t reg)
{
    app_camData.writeReg[0] = reg & 0x7F;

    app_camData.isTransferComplete = false;

    DRV_SPI_WriteReadTransferAdd(app_camData.spiHandle,
            app_camData.writeReg, 1, app_camData.readReg, 3, &app_camData.transferHandle);

    APP_CAM_DelayMS(1);

    while (!app_camData.isTransferComplete);

    return app_camData.readReg[2];
}


/*
 * Function: APP_CAM_Camera_Is_Ready
 * ---------------------------------
 * Checks if the camera is ready by polling its status register.
 *
 * Parameters:
 *   None.
 *
 * Returns:
 *   true if the camera is ready, false otherwise.
 */
static bool APP_CAM_Camera_Is_Ready(void)
{
    uint8_t timeout = 50;

    while (timeout != 0)
    {
        if ((APP_CAM_Camera_Read_Reg(CAM_REG_SENSOR_STATE) & 0x03) == CAM_REG_SENSOR_STATE_IDLE)
        {
            return true;
        }

        timeout--;
        APP_CAM_DelayMS(2);
    }

    return false;
}


/*
 * Function: APP_CAM_Read_Image_Size
 * ---------------------------------
 * Reads the size of the image data from the camera FIFO registers.
 *
 * Parameters:
 *   None.
 *
 * Returns:
 *   The image size in bytes (24-bit value).
 */
static uint32_t APP_CAM_Read_Image_Size(void)
{
    uint32_t len1 = APP_CAM_Camera_Read_Reg(FIFO_SIZE1);
    uint32_t len2 = APP_CAM_Camera_Read_Reg(FIFO_SIZE2);
    uint32_t len3 = APP_CAM_Camera_Read_Reg(FIFO_SIZE3);

    return ((len3 << 16) | (len2 << 8) | len1) & 0xffffff;
}


/*
 * Function: APP_CAM_Read_Image
 * ----------------------------
 * Reads image data from the camera's FIFO in burst mode over SPI.
 *
 * Parameters:
 *   buf    - Pointer to the buffer where the image data will be stored.
 *   length - Number of bytes to read from the camera.
 *
 * Returns:
 *   None.
 */
static void APP_CAM_Read_Image(uint8_t* buf, uint32_t length)
{
    app_camData.writeReg[0] = BURST_FIFO_READ;
    app_camData.writeReg[1] = 0x00;

    app_camData.isTransferComplete = false;

    DRV_SPI_WriteReadTransferAdd(app_camData.spiHandle, app_camData.writeReg,
            1, buf, length, &app_camData.transferHandle);

    while (!app_camData.isTransferComplete);
}


/*
 * Function: APP_CAM_RGB565_To_Grayscale
 * -------------------------------------
 * Converts RGB565 image data to grayscale.
 *
 * Parameters:
 *   rgb565_buf  - Pointer to the input RGB565 image data.
 *   gray_buf    - Pointer to the output grayscale buffer.
 *   pixel_count - Number of pixels to convert.
 *
 * Returns:
 *   None.
 */
static void APP_CAM_RGB565_To_Grayscale(uint8_t* rgb565_buf, uint8_t* gray_buf, uint32_t pixel_count)
{
    for (uint32_t i = 0; i < pixel_count; i++)
    {
        uint16_t pixel = ((uint16_t)rgb565_buf[i * 2 + 1] << 8) | rgb565_buf[i * 2];

        uint8_t r = (pixel >> 11) & 0x1F;
        uint8_t g = (pixel >> 5) & 0x3F;
        uint8_t b = pixel & 0x1F;

        r = (r << 3) | (r >> 2);
        g = (g << 2) | (g >> 4);
        b = (b << 3) | (b >> 2);

        gray_buf[i] = (uint8_t)((r * 77 + g * 150 + b * 29) >> 8);
    }
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_CAM_Initialize ( void )

  Remarks:
    See prototype in app_cam.h.
 */

void APP_CAM_Initialize(void)
{
    app_camData.state = APP_CAM_STATE_INIT;
    app_camData.transferStatus = false;
    app_camData.cam_ready = false;
    app_camData.image_ready = false;

    LED0_Clear();
}


/******************************************************************************
  Function:
    void APP_CAM_Tasks ( void )

  Remarks:
    See prototype in app_cam.h.
 */

void APP_CAM_Tasks(void)
{
    switch (app_camData.state)
    {
        case APP_CAM_STATE_INIT:
        {
            app_camData.spiSetup.baudRateInHz = 8000000;
            app_camData.spiSetup.clockPhase = DRV_SPI_CLOCK_PHASE_VALID_LEADING_EDGE;
            app_camData.spiSetup.clockPolarity = DRV_SPI_CLOCK_POLARITY_IDLE_LOW;
            app_camData.spiSetup.dataBits = DRV_SPI_DATA_BITS_8;
            app_camData.spiSetup.chipSelect = (SYS_PORT_PIN)MIKROBUS_SPI_CS_PIN;
            app_camData.spiSetup.csPolarity = DRV_SPI_CS_POLARITY_ACTIVE_LOW;

            app_camData.spiHandle = DRV_SPI_Open(DRV_SPI_INDEX_0, DRV_IO_INTENT_READWRITE);
            app_camData.consoleHandle = SYS_CONSOLE_HandleGet(SYS_CONSOLE_INDEX_0);

            if ((app_camData.spiHandle != DRV_HANDLE_INVALID) &&
                (app_camData.consoleHandle != SYS_CONSOLE_HANDLE_INVALID))
            {
                if (DRV_SPI_TransferSetup(app_camData.spiHandle, &app_camData.spiSetup) == true)
                {
                    DRV_SPI_TransferEventHandlerSet(app_camData.spiHandle,
                        APP_CAM_SPI_Event_Handler, (uintptr_t)0);

                    app_camData.state = APP_CAM_STATE_CAMERA_RESET;
                }
                else
                {
                    app_camData.state = APP_CAM_STATE_ERROR;
                }
            }
            else
            {
                app_camData.state = APP_CAM_STATE_ERROR;
            }
            break;
        }

        case APP_CAM_STATE_CAMERA_RESET:
        {
            if (APP_CAM_Camera_Write_Reg(CAM_REG_SENSOR_RESET, CAM_SENSOR_RESET_ENABLE))
            {
                if (APP_CAM_Camera_Is_Ready())
                {
                    app_camData.state = APP_CAM_STATE_CHECK_CAMERA;
                }
                else
                {
                    app_camData.state = APP_CAM_STATE_ERROR;
                }
            }
            else
            {
                app_camData.state = APP_CAM_STATE_ERROR;
            }
            break;
        }

        case APP_CAM_STATE_CHECK_CAMERA:
        {
            uint8_t cam_id = APP_CAM_Camera_Read_Reg(CAM_REG_SENSOR_ID);

            if ((cam_id == MEGA_5MP_ID) || (cam_id == MEGA_3MP_ID))
            {
                if (APP_CAM_Camera_Is_Ready())
                {
                    app_camData.state = APP_CAM_STATE_SET_BMP_FORMAT;
                }
                else
                {
                    app_camData.state = APP_CAM_STATE_ERROR;
                }
            }
            else
            {
                APP_CAM_DelayMS(100);
            }
            break;
        }

        case APP_CAM_STATE_SET_BMP_FORMAT:
        {
            if (APP_CAM_Camera_Write_Reg(CAM_REG_FORMAT, CAM_IMAGE_PIX_FMT_RGB565))
            {
                if (APP_CAM_Camera_Is_Ready())
                {
                    app_camData.state = APP_CAM_STATE_SET_RESOLUTION;
                }
                else
                {
                    app_camData.state = APP_CAM_STATE_ERROR;
                }
            }
            break;
        }

        case APP_CAM_STATE_SET_RESOLUTION:
        {
            if (APP_CAM_Camera_Write_Reg(CAM_REG_CAPTURE_RESOLUTION,
                    CAM_SET_CAPTURE_MODE | CAM_IMAGE_MODE_96x96))
            {
                if (APP_CAM_Camera_Is_Ready())
                {
                    app_camData.state = APP_CAM_STATE_SERVICE_TASKS;
                    app_camData.cam_ready = true;

                    LED0_Set();
                }
                else
                {
                    app_camData.state = APP_CAM_STATE_ERROR;
                }
            }
            break;
        }

        case APP_CAM_STATE_SERVICE_TASKS:
        {
            break;
        }

        case APP_CAM_STATE_IMG_CAPTURE:
        {
            if (APP_CAM_Camera_Write_Reg(ARDUCHIP_FIFO, FIFO_CLEAR_MASK))
            {
                if (APP_CAM_Camera_Write_Reg(ARDUCHIP_FIFO, FIFO_START_MASK))
                {
                    app_camData.state = APP_CAM_STATE_CAPTURE_WAIT;
                }
                else
                {
                    app_camData.state = APP_CAM_STATE_ERROR;
                }
            }
            else
            {
                app_camData.state = APP_CAM_STATE_ERROR;
            }
            break;
        }

        case APP_CAM_STATE_CAPTURE_WAIT:
        {
            APP_CAM_Camera_Read_Reg(ARDUCHIP_TRIG);

            if ((app_camData.readReg[2] & CAP_DONE_MASK) == CAP_DONE_MASK)
            {
                app_camData.imageSize = APP_CAM_Read_Image_Size();
                app_camData.state = APP_CAM_STATE_IMG_READ;
            }
            break;
        }

        case APP_CAM_STATE_IMG_READ:
        {
            if (app_camData.imageSize > 0)
            {
                APP_CAM_Read_Image(app_camData.captureBuffer,
                        sizeof(app_camData.captureBuffer));

                APP_CAM_RGB565_To_Grayscale(&app_camData.captureBuffer[1],
                        app_camData.grayscaleBuffer,
                        CAP_IMG_WIDTH * CAP_IMG_HEIGHT);

                app_camData.image_ready = true;
                app_camData.state = APP_CAM_STATE_IMG_READY;
            }
            break;
        }

        case APP_CAM_STATE_IMG_READY:
        {
            break;
        }

        case APP_CAM_STATE_IDLE:
        {
            break;
        }

        case APP_CAM_STATE_ERROR:
        {
            LED0_Toggle();
            {
                volatile uint32_t i;
                for (i = 0; i < 1000000; i++);
            }
            break;
        }

        default:
        {
            break;
        }
    }
}


/*******************************************************************************
 End of File
 */
