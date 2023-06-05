/**
 * @file controller.c
 * @author wanghongxi
 * @author modified by neozng
 * @brief  PID控制器定义
 * @version beta
 * @date 2022-11-01
 *
 * @copyrightCopyright (c) 2022 HNU YueLu EC all rights reserved
 */
#include "controller.h"
#include "memory.h"

/* ----------------------------下面是pid优化环节的实现---------------------------- */

// 梯形积分
static void f_Trapezoid_Intergral(PIDInstance *pid)
{
    // 计算梯形的面积,(上底+下底)*高/2
    pid->ITerm = pid->Ki * ((pid->Err + pid->Last_Err) / 2) * pid->dt;
}

// 变速积分(误差小时积分作用更强)
static void f_Changing_Integration_Rate(PIDInstance *pid)
{
    if (pid->Err * pid->Iout > 0)
    {
        // 积分呈累积趋势
        if (abs(pid->Err) <= pid->CoefB)
            return; // Full integral
        if (abs(pid->Err) <= (pid->CoefA + pid->CoefB))
            pid->ITerm *= (pid->CoefA - abs(pid->Err) + pid->CoefB) / pid->CoefA;
        else // 最大阈值,不使用积分
            pid->ITerm = 0;
    }
}

static void f_Integral_Limit(PIDInstance *pid)
{
    static float temp_Output, temp_Iout;
    temp_Iout = pid->Iout + pid->ITerm;
    temp_Output = pid->Pout + pid->Iout + pid->Dout;
    if (abs(temp_Output) > pid->MaxOut)
    {
        if (pid->Err * pid->Iout > 0) // 积分却还在累积
        {
            pid->ITerm = 0; // 当前积分项置零
        }
    }

    if (temp_Iout > pid->IntegralLimit)
    {
        pid->ITerm = 0;
        pid->Iout = pid->IntegralLimit;
    }
    if (temp_Iout < -pid->IntegralLimit)
    {
        pid->ITerm = 0;
        pid->Iout = -pid->IntegralLimit;
    }
}

// 微分先行(仅使用反馈值而不计参考输入的微分)
static void f_Derivative_On_Measurement(PIDInstance *pid)
{
    pid->Dout = pid->Kd * (pid->Last_Measure - pid->Measure) / pid->dt;
}

// 微分滤波(采集微分时,滤除高频噪声)
static void f_Derivative_Filter(PIDInstance *pid)
{
    pid->Dout = pid->Dout * pid->dt / (pid->Derivative_LPF_RC + pid->dt) +
                pid->Last_Dout * pid->Derivative_LPF_RC / (pid->Derivative_LPF_RC + pid->dt);
}

// 输出滤波
static void f_Output_Filter(PIDInstance *pid)
{
    pid->Output = pid->Output * pid->dt / (pid->Output_LPF_RC + pid->dt) +
                  pid->Last_Output * pid->Output_LPF_RC / (pid->Output_LPF_RC + pid->dt);
}

// 输出限幅
static void f_Output_Limit(PIDInstance *pid)
{
    if (pid->Output > pid->MaxOut)
    {
        pid->Output = pid->MaxOut;
    }
    if (pid->Output < -(pid->MaxOut))
    {
        pid->Output = -(pid->MaxOut);
    }
}

// 电机堵转检测
static void f_PID_ErrorHandle(PIDInstance *pid)
{
    /*Motor Blocked Handle*/
    if (fabsf(pid->Output) < pid->MaxOut * 0.001f || fabsf(pid->Ref) < 0.0001f)
        return;

    if ((fabsf(pid->Ref - pid->Measure) / fabsf(pid->Ref)) > 0.95f)
    {
        // Motor blocked counting
        pid->ERRORHandler.ERRORCount++;
    }
    else
    {
        pid->ERRORHandler.ERRORCount = 0;
    }

    if (pid->ERRORHandler.ERRORCount > 500)
    {
        // Motor blocked over 1000times
        pid->ERRORHandler.ERRORType = PID_MOTOR_BLOCKED_ERROR;
    }
}

/* ---------------------------下面是PID的外部算法接口--------------------------- */

/**
 * @brief 初始化PID,设置参数和启用的优化环节,将其他数据置零
 *
 * @param pid    PID实例
 * @param config PID初始化设置
 */
void PIDInit(PIDInstance *pid, PID_Init_Config_s *config)
{
    // config的数据和pid的部分数据是连续且相同的的,所以可以直接用memcpy
    // @todo: 不建议这样做,可扩展性差,不知道的开发者可能会误以为pid和config是同一个结构体
    // 后续修改为逐个赋值
    memset(pid, 0, sizeof(PIDInstance));
    // utilize the quality of struct that its memeory is continuous
    memcpy(pid, config, sizeof(PID_Init_Config_s));
    // set rest of memory to 0
    DWT_GetDeltaT(&pid->DWT_CNT);
}

/**
 * @brief          PID计算
 * @param[in]      PID结构体
 * @param[in]      测量值
 * @param[in]      期望值
 * @retval         返回空
 */
float PIDCalculate(PIDInstance *pid, float measure, float ref)
{
    // 堵转检测
    if (pid->Improve & PID_ErrorHandle)
        f_PID_ErrorHandle(pid);

    pid->dt = DWT_GetDeltaT(&pid->DWT_CNT); // 获取两次pid计算的时间间隔,用于积分和微分

    // 保存上次的测量值和误差,计算当前error
    pid->Measure = measure;
    pid->Ref = ref;
    pid->Err = pid->Ref - pid->Measure;

    // 如果在死区外,则计算PID
    if (abs(pid->Err) > pid->DeadBand)
    {
        // 基本的pid计算,使用位置式
        pid->Pout = pid->Kp * pid->Err;
        pid->ITerm = pid->Ki * pid->Err * pid->dt;
        pid->Dout = pid->Kd * (pid->Err - pid->Last_Err) / pid->dt;

        // 梯形积分
        if (pid->Improve & PID_Trapezoid_Intergral)
            f_Trapezoid_Intergral(pid);
        // 变速积分
        if (pid->Improve & PID_ChangingIntegrationRate)
            f_Changing_Integration_Rate(pid);
        // 微分先行
        if (pid->Improve & PID_Derivative_On_Measurement)
            f_Derivative_On_Measurement(pid);
        // 微分滤波器
        if (pid->Improve & PID_DerivativeFilter)
            f_Derivative_Filter(pid);
        // 积分限幅
        if (pid->Improve & PID_Integral_Limit)
            f_Integral_Limit(pid);

        pid->Iout += pid->ITerm;                         // 累加积分
        pid->Output = pid->Pout + pid->Iout + pid->Dout; // 计算输出

        // 输出滤波
        if (pid->Improve & PID_OutputFilter)
            f_Output_Filter(pid);

        // 输出限幅
        f_Output_Limit(pid);
    }
    else // 进入死区, 则清空积分和输出
    {
        pid->Output = 0;
        pid->ITerm = 0;
    }

    // 保存当前数据,用于下次计算
    pid->Last_Measure = pid->Measure;
    pid->Last_Output = pid->Output;
    pid->Last_Dout = pid->Dout;
    pid->Last_Err = pid->Err;
    pid->Last_ITerm = pid->ITerm;

    return pid->Output;
}