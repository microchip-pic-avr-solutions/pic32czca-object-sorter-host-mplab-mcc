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

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

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
        else
        {
            timeout--;
        }

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
    uint32_t len1, len2, len3;

    len1 = APP_CAM_Camera_Read_Reg(FIFO_SIZE1);
    len2 = APP_CAM_Camera_Read_Reg(FIFO_SIZE2);
    len3 = APP_CAM_Camera_Read_Reg(FIFO_SIZE3);

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
    uint32_t i;
    uint16_t pixel;
    uint8_t r, g, b;

    for (i = 0; i < pixel_count; i++)
    {
        /* ArduCAM sends RGB565 in little-endian format: LSB first, MSB second */
        pixel = ((uint16_t)rgb565_buf[i * 2 + 1] << 8) | rgb565_buf[i * 2];

        r = (pixel >> 11) & 0x1F;
        g = (pixel >> 5) & 0x3F;
        b = pixel & 0x1F;

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
    app_camData.consoleHandle = SYS_CONSOLE_HANDLE_INVALID;
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
            app_camData.consoleHandle = SYS_CONSOLE_HandleGet(SYS_CONSOLE_INDEX_0);

            app_camData.spiSetup.baudRateInHz = 8000000;
            app_camData.spiSetup.clockPhase = DRV_SPI_CLOCK_PHASE_VALID_LEADING_EDGE;
            app_camData.spiSetup.clockPolarity = DRV_SPI_CLOCK_POLARITY_IDLE_LOW;
            app_camData.spiSetup.dataBits = DRV_SPI_DATA_BITS_8;
            app_camData.spiSetup.chipSelect = (SYS_PORT_PIN)MIKROBUS_SPI_CS_PIN;
            app_camData.spiSetup.csPolarity = DRV_SPI_CS_POLARITY_ACTIVE_LOW;

            app_camData.spiHandle = DRV_SPI_Open(DRV_SPI_INDEX_0, DRV_IO_INTENT_READWRITE);

            if (app_camData.spiHandle != DRV_HANDLE_INVALID)
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
                if (APP_CAM_Camera_Is_Ready() == true)
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
                if (APP_CAM_Camera_Is_Ready() == true)
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
                if (APP_CAM_Camera_Is_Ready() == true)
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
                if (APP_CAM_Camera_Is_Ready() == true)
                {
                    app_camData.state = APP_CAM_STATE_INPUT_COMMAND;
                    app_camData.cam_ready = true;
                    
                    BSP_LED0_On();
                }
                else
                {
                    app_camData.state = APP_CAM_STATE_ERROR;
                }
            }
            break;
        }

        case APP_CAM_STATE_INPUT_COMMAND:
        {
            int32_t nBytes = (int32_t)SYS_CONSOLE_ReadCountGet(app_camData.consoleHandle);
            
            if (nBytes > 0)
            {
                char ch;
                SYS_CONSOLE_Read(app_camData.consoleHandle, &ch, 1);

                if (ch == 'R' || ch == 'r')
                {
                    app_camData.state = APP_CAM_STATE_IMG_CAPTURE;
                }
            }
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

                app_camData.transferOffset = 0;
                app_camData.state = APP_CAM_STATE_SEND_IMAGE;
            }
            else
            {
                app_camData.state = APP_CAM_STATE_INPUT_COMMAND;
            }
            break;
        }

        case APP_CAM_STATE_SEND_IMAGE:
        {
            if (app_camData.consoleHandle != SYS_CONSOLE_HANDLE_INVALID)
            {
                int32_t freeSpace = (int32_t)SYS_CONSOLE_WriteFreeBufferCountGet(app_camData.consoleHandle);
                if (freeSpace > 0)
                {
                    uint32_t remaining = (CAP_IMG_WIDTH * CAP_IMG_HEIGHT) - app_camData.transferOffset;
                    uint32_t toWrite = (remaining < (uint32_t)freeSpace) ? remaining : (uint32_t)freeSpace;

                    int32_t written = (int32_t)SYS_CONSOLE_Write(app_camData.consoleHandle,
                            &app_camData.grayscaleBuffer[app_camData.transferOffset],
                            toWrite);

                    if (written > 0)
                    {
                        app_camData.transferOffset += (uint32_t)written;
                    }

                    if (app_camData.transferOffset >= (CAP_IMG_WIDTH * CAP_IMG_HEIGHT))
                    {
                        app_camData.state = APP_CAM_STATE_TRANSFER_WAIT;
                    }
                }
            }
            else
            {
                app_camData.state = APP_CAM_STATE_INPUT_COMMAND;
            }
            break;
        }

        case APP_CAM_STATE_TRANSFER_WAIT:
        {
            int32_t pending = (int32_t)SYS_CONSOLE_WriteCountGet(app_camData.consoleHandle);
            
            if (pending == 0)
            {
                app_camData.state = APP_CAM_STATE_INPUT_COMMAND;
            }
            
            break;
        }

        case APP_CAM_STATE_IDLE:
        {
            break;
        }

        case APP_CAM_STATE_ERROR:
        {
            BSP_LED1_On();
            
            app_camData.state = APP_CAM_STATE_IDLE;
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
