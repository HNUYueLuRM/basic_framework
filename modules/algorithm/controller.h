/**
  ******************************************************************************
  * @file	 controller.h
  * @author  Wang Hongxi
  * @version V1.1.3
  * @date    2021/7/3
  * @brief   
  ******************************************************************************
  * @attention 
  *
  ******************************************************************************
  */
#ifndef _CONTROLLER_H
#define _CONTROLLER_H


#include "main.h"
#include "stdint.h"
#include "string.h"
#include "stdlib.h"
#include "bsp_dwt.h"
#include "arm_math.h"
#include <math.h>

#ifndef abs
#define abs(x) ((x > 0) ? x : -x)
#endif


/******************************* PID CONTROL *********************************/
typedef enum pid_Improvement_e
{
    NONE = 0X00,                        //0000 0000
    Integral_Limit = 0x01,              //0000 0001
    Derivative_On_Measurement = 0x02,   //0000 0010
    Trapezoid_Intergral = 0x04,         //0000 0100
    Proportional_On_Measurement = 0x08, //0000 1000
    OutputFilter = 0x10,                //0001 0000
    ChangingIntegrationRate = 0x20,     //0010 0000
    DerivativeFilter = 0x40,            //0100 0000
    ErrorHandle = 0x80,                 //1000 0000
} PID_Improvement_e;

typedef enum errorType_e
{
    PID_ERROR_NONE = 0x00U,
    Motor_Blocked = 0x01U
} ErrorType_e;

typedef  struct
{
    uint64_t ERRORCount;
    ErrorType_e ERRORType;
} PID_ErrorHandler_t;

typedef struct 
{
//---------------------------------- init config block
    // config parameter
    float Kp;
    float Ki;
    float Kd;

    float MaxOut;
    float IntegralLimit;
    float DeadBand;
    float CoefA;         //For Changing Integral
    float CoefB;         //ITerm = Err*((A-abs(err)+B)/A)  when B<|err|<A+B
    float Output_LPF_RC; // RC = 1/omegac
    float Derivative_LPF_RC;

    uint8_t Improve;
//-----------------------------------
    // for calculating
    float Measure;
    float Last_Measure;
    float Err;
    float Last_Err;
    float Last_ITerm;

    float Pout;
    float Iout;
    float Dout;
    float ITerm;

    float Output;
    float Last_Output;
    float Last_Dout;

    float Ref;

    uint32_t DWT_CNT;
    float dt;

    PID_ErrorHandler_t ERRORHandler;
} PID_t;

/* 用于PID初始化的结构体*/
typedef struct 
{
    // config parameter
    float Kp;
    float Ki;
    float Kd;

    float MaxOut;
    float IntegralLimit;
    float DeadBand;
    float CoefA;         //For Changing Integral
    float CoefB;         //ITerm = Err*((A-abs(err)+B)/A)  when B<|err|<A+B
    float Output_LPF_RC; // RC = 1/omegac
    float Derivative_LPF_RC;

    uint8_t Improve;

} PID_Init_config_s;



void PID_Init(PID_t* pid,PID_Init_config_s* config);
float PID_Calculate(PID_t *pid, float measure, float ref);


/*************************** Tracking Differentiator ***************************/
typedef  struct
{
    float Input;

    float h0;
    float r;

    float x;
    float dx;
    float ddx;

    float last_dx;
    float last_ddx;

    uint32_t DWT_CNT;
    float dt;
} TD_t;

void TD_Init(TD_t *td, float r, float h0);
float TD_Calculate(TD_t *td, float input);

#endif
