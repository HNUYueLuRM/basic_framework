/**
 * @file bsp_pwm.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2023-02-14
 * @todo 目前的实现有比较大的问题,是否允许module修改tim的分频器和自动重装载寄存器?
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef BSP_PWM_H
#define BSP_PWM_H

#include "tim.h"
#include "stdint.h"

#define PWM_DEVICE_CNT 16 // 最大支持的PWM实例数量

/* pwm实例结构体 */
typedef struct pwm_ins_temp
{
    TIM_HandleTypeDef *htim;                 // TIM句柄
    uint32_t channel;                        // 通道
    uint32_t period;                         // 周期
    uint32_t pulse;                          // 脉宽
    void (*callback)(struct pwm_ins_temp *); // DMA传输完成回调函数
    void *id;                                // 实例ID

    // 后续还要添加更多的参数,以提供更直观的封装,比如直接按照百分比设置占空比,直接设置频率等
    // ...
} PWMInstance;

typedef struct
{
    TIM_HandleTypeDef *htim;                 // TIM句柄
    uint32_t channel;                        // 通道
    uint32_t period;                         // 周期
    uint32_t pulse;                          // 脉宽
    void (*callback)(PWMInstance*); // DMA传输完成回调函数
    void *id;                                // 实例ID
} PWM_Init_Config_s;

/**
 * @brief 注册一个pwm实例
 *
 * @param config 初始化配置
 * @return PWMInstance*
 */
PWMInstance *PWMRegister(PWM_Init_Config_s *config);

/**
 * @brief 启动pwm
 *
 * @param pwm pwm实例
 */
void PWMStart(PWMInstance *pwm);

/**
 * @brief 停止pwm
 *
 * @param pwm pwm实例
 */
void PWMStop(PWMInstance *pwm);


// @todo 这三个函数还需要进一步封装,务必协调好三者之间的关系
/**
 * @brief 设置pwm脉宽
 *
 * @param pwm pwm实例
 * @param pulse 脉宽
 */
void PWMSetPulse(PWMInstance *pwm, uint32_t pulse);
void PWMSetPeriod(PWMInstance *pwm, uint32_t period);       // 未实现
void PWMSetPrescaler(PWMInstance *pwm, uint32_t prescaler); // 未实现

/**
 * @brief 启动pwm dma传输
 *
 * @param pwm pwm实例
 * @param pData 数据首地址指针,注意数据的位数必须和CubeMX配置的DMA传输位数(字长)一致
 * @param Size 数据长度
 * @note 如果使用此函数,则需要在CubeMX中配置DMA传输位数为对应位数
 *       例如:使用16位数据,则需要配置DMA传输位数为16位(half word),配置错误会导致指针越界或数据错误
 */
void PWMStartDMA(PWMInstance *pwm, uint32_t *pData, uint32_t Size);

#endif // BSP_PWM_H