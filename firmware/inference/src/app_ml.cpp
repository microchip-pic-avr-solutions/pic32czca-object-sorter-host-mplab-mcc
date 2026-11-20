/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_ml.cpp

  Summary:
    TFLite Micro ML inference for object classification.
*******************************************************************************/

#include "app_ml.h"
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include "peripheral/pio/plib_pio.h"
#include "system/console/sys_console.h"
#include "system/time/sys_time.h"

#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "model_data.h"

extern "C" {
    bool APP_CAM_TaskIsReady(void);
    bool APP_CAM_ImageIsReady(void);
    uint8_t* APP_CAM_GetGrayscaleBuffer(void);
    void APP_CAM_TriggerCapture(void);
    void APP_CAM_Task_State_Set(int task_state);
    int APP_CAM_Get_Service_Tasks_State(void);
}

APP_ML_DATA app_mlData;


static const tflite::Model* model = nullptr;
static tflite::ErrorReporter* error_reporter = nullptr;
static tflite::MicroInterpreter* interpreter = nullptr;

constexpr int kTensorArenaSize = 256 * 1024;
static uint8_t tensor_arena[kTensorArenaSize] __ALIGNED(16);

static bool tflite_initialized = false;

static void tflite_setup(void)
{
    static tflite::MicroErrorReporter micro_error_reporter;
    error_reporter = &micro_error_reporter;

    model = tflite::GetModel(model_data);
    if (model->version() != TFLITE_SCHEMA_VERSION) 
    {        
        TF_LITE_REPORT_ERROR(error_reporter,
            "Model schema version %d != supported %d",
            model->version(), TFLITE_SCHEMA_VERSION);
        return;
    }
    
    static tflite::AllOpsResolver micro_op_resolver;

    static tflite::MicroInterpreter static_interpreter(
        model, micro_op_resolver, tensor_arena, kTensorArenaSize, error_reporter);
    
    interpreter = &static_interpreter;
    
    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    
    if (allocate_status != kTfLiteOk) 
    {        
        TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors() failed");
        return;
    }

    tflite_initialized = true;
}

static int8_t classScores[4];
static int8_t lastMaxValue = -128;
static uint32_t inferenceStartTime = 0;

static const char* classNames[] = {"Circle", "Square", "Triangle", "Unknown"};

static uint8_t tflite_runInference(uint8_t* imageData)
{
    int8_t maxValue = -128;
    int8_t shapeIndex = 3;

    if (!tflite_initialized || interpreter == nullptr) 
    {        
        lastMaxValue = -128;
        return ML_CLASS_UNKNOWN;
    }

    TfLiteTensor* input = interpreter->input(0);
    TfLiteTensor* output = interpreter->output(0);

    for (uint32_t i = 0; i < (uint32_t)input->bytes; i++) 
    {
        input->data.int8[i] = (int8_t)(imageData[i] - 128);
    }

    TfLiteStatus invoke_status = interpreter->Invoke();

    if (invoke_status != kTfLiteOk) 
    {
        TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed");
        lastMaxValue = -128;
        return ML_CLASS_UNKNOWN;
    }

    for (int i = 0; i < 4; i++) 
    {
        classScores[i] = output->data.int8[i];

        if (classScores[i] > maxValue) 
        {
            shapeIndex = i;
            maxValue = classScores[i];
        }
    }

    lastMaxValue = maxValue;

    SYS_CONSOLE_PRINT("scores=[%d,%d,%d,%d], result=%s",
           (int)classScores[0], (int)classScores[1],
           (int)classScores[2], (int)classScores[3],
           classNames[shapeIndex]);

    switch (shapeIndex) 
    {
        case 0: return ML_CLASS_CIRCLE;
        
        case 1: return ML_CLASS_SQUARE;
        
        case 2: return ML_CLASS_TRIANGLE;
        
        default: return ML_CLASS_UNKNOWN;
    }
}

#define ML_MAX_INIT_RETRIES 3

extern "C" void APP_ML_Initialize(void)
{
    app_mlData.state = APP_ML_STATE_INIT;
    app_mlData.ml_ready = false;
    app_mlData.result_ready = false;
    app_mlData.classification_result = ML_CLASS_UNKNOWN;
    app_mlData.confidence = 0;
    app_mlData.init_retry_count = 0;

    tflite_setup();

    if (tflite_initialized) 
    {
        app_mlData.ml_ready = true;
    } 
}

extern "C" void APP_ML_Tasks(void)
{
    switch (app_mlData.state)
    {
        case APP_ML_STATE_INIT:
        {
            if (!tflite_initialized && app_mlData.init_retry_count < ML_MAX_INIT_RETRIES)
            {
                app_mlData.init_retry_count++;
                
                tflite_setup();
                
                if (tflite_initialized) 
                {
                    app_mlData.ml_ready = true;
                    
                }
            }
            
            app_mlData.state = APP_ML_STATE_WAIT_CAM_READY;
            break;
        }

        case APP_ML_STATE_WAIT_CAM_READY:
        {
            if (APP_CAM_TaskIsReady())
            {
                app_mlData.ml_ready = tflite_initialized;
                
                app_mlData.state = APP_ML_STATE_SERVICE_TASKS;
            }
            break;
        }

        case APP_ML_STATE_SERVICE_TASKS:
        {
            break;
        }

        case APP_ML_STATE_WAIT_IMAGE:
        {
            if (APP_CAM_ImageIsReady())
            {                
                app_mlData.state = APP_ML_STATE_RUN_INFERENCE;
            }
            
            break;
        }

        case APP_ML_STATE_RUN_INFERENCE:
        {
            uint8_t* grayscale_buffer = APP_CAM_GetGrayscaleBuffer();

            uint8_t raw_result = tflite_runInference(grayscale_buffer);

            SYS_CONSOLE_PRINT("[ML] Circle:%d Square:%d Triangle:%d Unknown:%d\r\n",
                   (int)classScores[0], (int)classScores[1],
                   (int)classScores[2], (int)classScores[3]);

            app_mlData.classification_result = raw_result;
            app_mlData.confidence = (lastMaxValue > 0) ? (uint8_t)lastMaxValue : 0;
            app_mlData.result_ready = true;

            app_mlData.state = APP_ML_STATE_INFERENCE_COMPLETE;
            break;
        }

        case APP_ML_STATE_INFERENCE_COMPLETE:
        {
            break;
        }

        case APP_ML_STATE_ERROR:
        {           
            app_mlData.ml_ready = false;
            break;
        }

        default:
        {
            break;
        }
    }
}

extern "C" bool APP_ML_IsReady(void)
{
    return app_mlData.ml_ready;
}

extern "C" bool APP_ML_IsResultReady(void)
{
    return app_mlData.result_ready;
}

extern "C" uint8_t APP_ML_GetClassificationResult(void)
{
    return app_mlData.classification_result;
}

extern "C" uint8_t APP_ML_GetConfidence(void)
{
    return app_mlData.confidence;
}

extern "C" void APP_ML_TriggerInference(void)
{
    if (app_mlData.ml_ready && app_mlData.state == APP_ML_STATE_SERVICE_TASKS)
    {
        app_mlData.result_ready = false;
        
        APP_CAM_TriggerCapture();
        
        app_mlData.state = APP_ML_STATE_WAIT_IMAGE;
    }
}

extern "C" void APP_ML_AcknowledgeResult(void)
{
    app_mlData.result_ready = false;
    
    APP_CAM_Task_State_Set(APP_CAM_Get_Service_Tasks_State());
    
    app_mlData.state = APP_ML_STATE_SERVICE_TASKS;
}

