/*******************************************************************************
    arducam Header File

  Company:
    Microchip Technology Inc.

  File Name:
    arducam.h

  Summary:

  Description:

 *******************************************************************************/

/*******************************************************************************
* Copyright (C) 2025 Microchip Technology Inc. and its subsidiaries.
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

#ifndef ARDUCAM_H
#define ARDUCAM_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#define ARDUCHIP_FRAMES                             0x01
#define ARDUCHIP_TEST1                              0x00
#define ARDUCHIP_FIFO                               0x04
#define ARDUCHIP_FIFO_2                             0x07

#define FIFO_START_MASK                             0x02
#define FIFO_RDPTR_RST_MASK                         0x10
#define FIFO_WRPTR_RST_MASK                         0x20
#define FIFO_CLEAR_MASK                             0x01

#define ARDUCHIP_TRIG                               0x44
#define VSYNC_MASK                                  0x01
#define SHUTTER_MASK                                0x02
#define CAP_DONE_MASK                               0x04

#define FIFO_SIZE1                                  0x45
#define FIFO_SIZE2                                  0x46
#define FIFO_SIZE3                                  0x47

#define BURST_FIFO_READ                             0x3C
#define SINGLE_FIFO_READ                            0x3D

#define CAM_REG_SENSOR_RESET                        0x07
#define CAM_REG_FORMAT                              0x20
#define CAM_REG_CAPTURE_RESOLUTION                  0x21
#define CAM_REG_SENSOR_ID                           0x40
#define CAM_REG_SENSOR_STATE                        0x44
#define CAM_REG_DEBUG_DEVICE_ADDRESS                0x0A

#define CAM_REG_SENSOR_STATE_IDLE                   (1 << 1)
#define CAM_SENSOR_RESET_ENABLE                     (1 << 6)
#define CAM_SET_CAPTURE_MODE                        (0 << 7)

#define CAM_IMAGE_PIX_FMT_RGB565                    0x02

#define CAMERA_DEVICE_ADDRESS                       0x78
#define MEGA_5MP_ID                                 0x81
#define MEGA_3MP_ID                                 0x82

typedef enum
{
    CAM_IMAGE_MODE_QQVGA    = 0x00,     /* 160x120 */
    CAM_IMAGE_MODE_QVGA     = 0x01,     /* 320x240 */
    CAM_IMAGE_MODE_VGA      = 0x02,     /* 640x480 */
    CAM_IMAGE_MODE_96x96    = 0x0a,     /* 96x96 */
} CAM_IMAGE_MODE;

#ifdef  __cplusplus
}
#endif

#endif  /* ARDUCAM_H */
