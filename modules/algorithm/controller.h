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

// PID 优化环节使能标志位
typedef enum
{
    NONE = 0b00000000,                        // 0000 0000
    Integral_Limit = 0b00000001,              // 0000 0001
    Derivative_On_Measurement = 0b00000010,   // 0000 0010
    Trapezoid_Intergral = 0b00000100,         // 0000 0100
    Proportional_On_Measurement = 0b00001000, // 0000 1000
    OutputFilter = 0b00010000,                // 0001 0000
    ChangingIntegrationRate = 0b00100000,     // 0010 0000
    DerivativeFilter = 0b01000000,            // 0100 0000
    ErrorHandle = 0b10000000,                 // 1000 0000
} PID_Improvement_e;

/* PID 报错类型枚举*/
typedef enum errorType_e
{
    PID_ERROR_NONE = 0x00U,
    Motor_Blocked = 0x01U
} ErrorType_e;

typedef struct
{
    uint64_t ERRORCount;
    ErrorType_e ERRORType;
} PID_ErrorHandler_t;

/* PID结构体 */
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
    float CoefA;         // For Changing Integral
    float CoefB;         // ITerm = Err*((A-abs(err)+B)/A)  when B<|err|<A+B
    float Output_LPF_RC; // RC = 1/omegac
    float Derivative_LPF_RC;

    PID_Improvement_e Improve;
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

    float MaxOut;        // 输出限幅
    float IntegralLimit; // 积分限幅
    float DeadBand;      // 死区
    float CoefA;         // For Changing Integral
    float CoefB;         // ITerm = Err*((A-abs(err)+B)/A)  when B<|err|<A+B
    float Output_LPF_RC; // RC = 1/omegac
    float Derivative_LPF_RC;

    PID_Improvement_e Improve;
} PID_Init_config_s;

/**
 * @brief 初始化PID实例
 *
 * @param pid    PID实例指针
 * @param config PID初始化配置
 */
void PID_Init(PID_t *pid, PID_Init_config_s *config);

/**
 * @brief 计算PID输出
 *
 * @param pid     PID实例指针
 * @param measure 反馈值
 * @param ref     设定值
 * @return float  PID计算输出
 */
float PID_Calculate(PID_t *pid, float measure, float ref);

#endif