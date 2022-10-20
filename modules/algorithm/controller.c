/**
 ******************************************************************************
 * @file    controller.c
 * @author  Wang Hongxi
 * @version V1.1.3
 * @date    2021/7/3
 * @brief   DWT定时器用于计算控制周期 OLS用于提取信号微分
 ******************************************************************************
 * @attention
 *
 ******************************************************************************
 */
#include "controller.h"

/******************************* PID CONTROL *********************************/
// PID优化环节函数声明
static void f_Trapezoid_Intergral(PID_t *pid);
static void f_Integral_Limit(PID_t *pid);
static void f_Derivative_On_Measurement(PID_t *pid);
static void f_Changing_Integration_Rate(PID_t *pid);
static void f_Output_Filter(PID_t *pid);
static void f_Derivative_Filter(PID_t *pid);
static void f_Output_Limit(PID_t *pid);
static void f_Proportion_Limit(PID_t *pid);
static void f_PID_ErrorHandle(PID_t *pid);

/**
 * @brief          PID初始化   PID initialize
 * @param[in]      PID结构体   PID structure
 * @param[in]      略
 * @retval         返回空      null
 */
void PID_Init(
    PID_t *pid,
    float max_out,
    float intergral_limit,
    float deadband,

    float kp,
    float Ki,
    float Kd,

    float A,
    float B,

    float output_lpf_rc,
    float derivative_lpf_rc,

    uint16_t ols_order,

    uint8_t improve)
{
    pid->DeadBand = deadband;
    pid->IntegralLimit = intergral_limit;
    pid->MaxOut = max_out;
    pid->Ref = 0;

    pid->Kp = kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
    pid->ITerm = 0;

    // 变速积分参数
    // coefficient of changing integration rate
    pid->CoefA = A;
    pid->CoefB = B;

    pid->Output_LPF_RC = output_lpf_rc;

    pid->Derivative_LPF_RC = derivative_lpf_rc;

    // 最小二乘提取信号微分初始化
    // differential signal is distilled by OLS
    pid->OLS_Order = ols_order;
    OLS_Init(&pid->OLS, ols_order);

    // DWT定时器计数变量清零
    // reset DWT Timer count counter
    pid->DWT_CNT = 0;

    // 设置PID优化环节
    pid->Improve = improve;

    // 设置PID异常处理 目前仅包含电机堵转保护
    pid->ERRORHandler.ERRORCount = 0;
    pid->ERRORHandler.ERRORType = PID_ERROR_NONE;

    pid->Output = 0;
}

/**
 * @brief          PID计算
 * @param[in]      PID结构体
 * @param[in]      测量值
 * @param[in]      期望值
 * @retval         返回空
 */
float PID_Calculate(PID_t *pid, float measure, float ref)
{
    if (pid->Improve & ErrorHandle)
        f_PID_ErrorHandle(pid);

    pid->dt = DWT_GetDeltaT((void *)&pid->DWT_CNT);

    pid->Measure = measure;
    pid->Ref = ref;
    pid->Err = pid->Ref - pid->Measure;

    if (pid->User_Func1_f != NULL)
        pid->User_Func1_f(pid);

    if (abs(pid->Err) > pid->DeadBand)
    {

        pid->Pout = pid->Kp * pid->Err;
        pid->ITerm = pid->Ki * pid->Err * pid->dt;
        if (pid->OLS_Order > 2)
            pid->Dout = pid->Kd * OLS_Derivative(&pid->OLS, pid->dt, pid->Err);
        else
            pid->Dout = pid->Kd * (pid->Err - pid->Last_Err) / pid->dt;

        if (pid->User_Func2_f != NULL)
            pid->User_Func2_f(pid);

        // 梯形积分
        if (pid->Improve & Trapezoid_Intergral)
            f_Trapezoid_Intergral(pid);
        // 变速积分
        if (pid->Improve & ChangingIntegrationRate)
            f_Changing_Integration_Rate(pid);
        // 微分先行
        if (pid->Improve & Derivative_On_Measurement)
            f_Derivative_On_Measurement(pid);
        // 微分滤波器
        if (pid->Improve & DerivativeFilter)
            f_Derivative_Filter(pid);
        // 积分限幅
        if (pid->Improve & Integral_Limit)
            f_Integral_Limit(pid);

        pid->Iout += pid->ITerm;

        pid->Output = pid->Pout + pid->Iout + pid->Dout;

        // 输出滤波
        if (pid->Improve & OutputFilter)
            f_Output_Filter(pid);

        // 输出限幅
        f_Output_Limit(pid);

        // 无关紧要
        f_Proportion_Limit(pid);
    }

    pid->Last_Measure = pid->Measure;
    pid->Last_Output = pid->Output;
    pid->Last_Dout = pid->Dout;
    pid->Last_Err = pid->Err;
    pid->Last_ITerm = pid->ITerm;

    return pid->Output;
}

static void f_Trapezoid_Intergral(PID_t *pid)
{

    pid->ITerm = pid->Ki * ((pid->Err + pid->Last_Err) / 2) * pid->dt;
    
}

static void f_Changing_Integration_Rate(PID_t *pid)
{
    if (pid->Err * pid->Iout > 0)
    {
        // 积分呈累积趋势
        // Integral still increasing
        if (abs(pid->Err) <= pid->CoefB)
            return; // Full integral
        if (abs(pid->Err) <= (pid->CoefA + pid->CoefB))
            pid->ITerm *= (pid->CoefA - abs(pid->Err) + pid->CoefB) / pid->CoefA;
        else
            pid->ITerm = 0;
    }
}

static void f_Integral_Limit(PID_t *pid)
{
    static float temp_Output, temp_Iout;
    temp_Iout = pid->Iout + pid->ITerm;
    temp_Output = pid->Pout + pid->Iout + pid->Dout;
    if (abs(temp_Output) > pid->MaxOut)
    {
        if (pid->Err * pid->Iout > 0)
        {
            // 积分呈累积趋势
            // Integral still increasing
            pid->ITerm = 0;
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

static void f_Derivative_On_Measurement(PID_t *pid)
{
    if (pid->OLS_Order > 2)
        pid->Dout = pid->Kd * OLS_Derivative(&pid->OLS, pid->dt, -pid->Measure);
    else
        pid->Dout = pid->Kd * (pid->Last_Measure - pid->Measure) / pid->dt;

}

static void f_Derivative_Filter(PID_t *pid)
{
    pid->Dout = pid->Dout * pid->dt / (pid->Derivative_LPF_RC + pid->dt) +
                pid->Last_Dout * pid->Derivative_LPF_RC / (pid->Derivative_LPF_RC + pid->dt);
}

static void f_Output_Filter(PID_t *pid)
{
    pid->Output = pid->Output * pid->dt / (pid->Output_LPF_RC + pid->dt) +
                  pid->Last_Output * pid->Output_LPF_RC / (pid->Output_LPF_RC + pid->dt);
}

static void f_Output_Limit(PID_t *pid)
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

static void f_Proportion_Limit(PID_t *pid)
{
    if (pid->Pout > pid->MaxOut)
    {
        pid->Pout = pid->MaxOut;
    }
    if (pid->Pout < -(pid->MaxOut))
    {
        pid->Pout = -(pid->MaxOut);
    }
}

// PID ERRORHandle Function
static void f_PID_ErrorHandle(PID_t *pid)
{
    /*Motor Blocked Handle*/
    if (pid->Output < pid->MaxOut * 0.001f || fabsf(pid->Ref) < 0.0001f)
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
        pid->ERRORHandler.ERRORType = Motor_Blocked;
    }
}

/*************************** FEEDFORWARD CONTROL *****************************/
/**
 * @brief          前馈控制初始化
 * @param[in]      前馈控制结构体
 * @param[in]      略
 * @retval         返回空
 */
void Feedforward_Init(
    Feedforward_t *ffc,
    float max_out,
    float *c,
    float lpf_rc,
    uint16_t ref_dot_ols_order,
    uint16_t ref_ddot_ols_order)
{
    ffc->MaxOut = max_out;

    // 设置前馈控制器参数 详见前馈控制结构体定义
    // set parameters of feed-forward controller (see struct definition)
    if (c != NULL && ffc != NULL)
    {
        ffc->c[0] = c[0];
        ffc->c[1] = c[1];
        ffc->c[2] = c[2];
    }
    else
    {
        ffc->c[0] = 0;
        ffc->c[1] = 0;
        ffc->c[2] = 0;
        ffc->MaxOut = 0;
    }

    ffc->LPF_RC = lpf_rc;

    // 最小二乘提取信号微分初始化
    // differential signal is distilled by OLS
    ffc->Ref_dot_OLS_Order = ref_dot_ols_order;
    ffc->Ref_ddot_OLS_Order = ref_ddot_ols_order;
    if (ref_dot_ols_order > 2)
        OLS_Init(&ffc->Ref_dot_OLS, ref_dot_ols_order);
    if (ref_ddot_ols_order > 2)
        OLS_Init(&ffc->Ref_ddot_OLS, ref_ddot_ols_order);

    ffc->DWT_CNT = 0;

    ffc->Output = 0;
}

/**
 * @brief          PID计算
 * @param[in]      PID结构体
 * @param[in]      测量值
 * @param[in]      期望值
 * @retval         返回空
 */
float Feedforward_Calculate(Feedforward_t *ffc, float ref)
{
    ffc->dt = DWT_GetDeltaT((void *)&ffc->DWT_CNT);

    ffc->Ref = ref * ffc->dt / (ffc->LPF_RC + ffc->dt) +
               ffc->Ref * ffc->LPF_RC / (ffc->LPF_RC + ffc->dt);

    // 计算一阶导数
    // calculate first derivative
    if (ffc->Ref_dot_OLS_Order > 2)
        ffc->Ref_dot = OLS_Derivative(&ffc->Ref_dot_OLS, ffc->dt, ffc->Ref);
    else
        ffc->Ref_dot = (ffc->Ref - ffc->Last_Ref) / ffc->dt;

    // 计算二阶导数
    // calculate second derivative
    if (ffc->Ref_ddot_OLS_Order > 2)
        ffc->Ref_ddot = OLS_Derivative(&ffc->Ref_ddot_OLS, ffc->dt, ffc->Ref_dot);
    else
        ffc->Ref_ddot = (ffc->Ref_dot - ffc->Last_Ref_dot) / ffc->dt;

    // 计算前馈控制输出
    // calculate feed-forward controller output
    ffc->Output = ffc->c[0] * ffc->Ref + ffc->c[1] * ffc->Ref_dot + ffc->c[2] * ffc->Ref_ddot;

    ffc->Output = float_constrain(ffc->Output, -ffc->MaxOut, ffc->MaxOut);

    ffc->Last_Ref = ffc->Ref;
    ffc->Last_Ref_dot = ffc->Ref_dot;

    return ffc->Output;
}

/*************************LINEAR DISTURBANCE OBSERVER *************************/
void LDOB_Init(
    LDOB_t *ldob,
    float max_d,
    float deadband,
    float *c,
    float lpf_rc,
    uint16_t measure_dot_ols_order,
    uint16_t measure_ddot_ols_order)
{
    ldob->Max_Disturbance = max_d;

    ldob->DeadBand = deadband;

    // 设置线性扰动观测器参数 详见LDOB结构体定义
    // set parameters of linear disturbance observer (see struct definition)
    if (c != NULL && ldob != NULL)
    {
        ldob->c[0] = c[0];
        ldob->c[1] = c[1];
        ldob->c[2] = c[2];
    }
    else
    {
        ldob->c[0] = 0;
        ldob->c[1] = 0;
        ldob->c[2] = 0;
        ldob->Max_Disturbance = 0;
    }

    // 设置Q(s)带宽  Q(s)选用一阶惯性环节
    // set bandwidth of Q(s)    Q(s) is chosen as a first-order low-pass form
    ldob->LPF_RC = lpf_rc;

    // 最小二乘提取信号微分初始化
    // differential signal is distilled by OLS
    ldob->Measure_dot_OLS_Order = measure_dot_ols_order;
    ldob->Measure_ddot_OLS_Order = measure_ddot_ols_order;
    if (measure_dot_ols_order > 2)
        OLS_Init(&ldob->Measure_dot_OLS, measure_dot_ols_order);
    if (measure_ddot_ols_order > 2)
        OLS_Init(&ldob->Measure_ddot_OLS, measure_ddot_ols_order);

    ldob->DWT_CNT = 0;

    ldob->Disturbance = 0;
}

float LDOB_Calculate(LDOB_t *ldob, float measure, float u)
{
    ldob->dt = DWT_GetDeltaT((void *)&ldob->DWT_CNT);

    ldob->Measure = measure;

    ldob->u = u;

    // 计算一阶导数
    // calculate first derivative
    if (ldob->Measure_dot_OLS_Order > 2)
        ldob->Measure_dot = OLS_Derivative(&ldob->Measure_dot_OLS, ldob->dt, ldob->Measure);
    else
        ldob->Measure_dot = (ldob->Measure - ldob->Last_Measure) / ldob->dt;

    // 计算二阶导数
    // calculate second derivative
    if (ldob->Measure_ddot_OLS_Order > 2)
        ldob->Measure_ddot = OLS_Derivative(&ldob->Measure_ddot_OLS, ldob->dt, ldob->Measure_dot);
    else
        ldob->Measure_ddot = (ldob->Measure_dot - ldob->Last_Measure_dot) / ldob->dt;

    // 估计总扰动
    // estimate external disturbances and internal disturbances caused by model uncertainties
    ldob->Disturbance = ldob->c[0] * ldob->Measure + ldob->c[1] * ldob->Measure_dot + ldob->c[2] * ldob->Measure_ddot - ldob->u;
    ldob->Disturbance = ldob->Disturbance * ldob->dt / (ldob->LPF_RC + ldob->dt) +
                        ldob->Last_Disturbance * ldob->LPF_RC / (ldob->LPF_RC + ldob->dt);

    ldob->Disturbance = float_constrain(ldob->Disturbance, -ldob->Max_Disturbance, ldob->Max_Disturbance);

    // 扰动输出死区
    // deadband of disturbance output
    if (abs(ldob->Disturbance) > ldob->DeadBand * ldob->Max_Disturbance)
        ldob->Output = ldob->Disturbance;
    else
        ldob->Output = 0;

    ldob->Last_Measure = ldob->Measure;
    ldob->Last_Measure_dot = ldob->Measure_dot;
    ldob->Last_Disturbance = ldob->Disturbance;

    return ldob->Output;
}

/*************************** Tracking Differentiator ***************************/
void TD_Init(TD_t *td, float r, float h0)
{
    td->r = r;
    td->h0 = h0;

    td->x = 0;
    td->dx = 0;
    td->ddx = 0;
    td->last_dx = 0;
    td->last_ddx = 0;
}
float TD_Calculate(TD_t *td, float input)
{
    static float d, a0, y, a1, a2, a, fhan;

    td->dt = DWT_GetDeltaT((void *)&td->DWT_CNT);

    if (td->dt > 0.5f)
        return 0;

    td->Input = input;

    d = td->r * td->h0 * td->h0;
    a0 = td->dx * td->h0;
    y = td->x - td->Input + a0;
    a1 = sqrt(d * (d + 8 * abs(y)));
    a2 = a0 + sign(y) * (a1 - d) / 2;
    a = (a0 + y) * (sign(y + d) - sign(y - d)) / 2 + a2 * (1 - (sign(y + d) - sign(y - d)) / 2);
    fhan = -td->r * a / d * (sign(a + d) - sign(a - d)) / 2 -
           td->r * sign(a) * (1 - (sign(a + d) - sign(a - d)) / 2);

    td->ddx = fhan;
    td->dx += (td->ddx + td->last_ddx) * td->dt / 2;
    td->x += (td->dx + td->last_dx) * td->dt / 2;

    td->last_ddx = td->ddx;
    td->last_dx = td->dx;

    return td->x;
}
