#include "bsp_pwm.h"
#include "stdlib.h"
#include "memory.h"

// 配合中断以及初始化
static uint8_t idx;
static PWMInstance *pwm_instance[PWM_DEVICE_CNT] = {NULL}; // 所有的pwm instance保存于此,用于callback时判断中断来源

/**
 * @brief pwm dma传输完成回调函数
 * 
 * @param htim 发生中断的定时器句柄
 */
void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
    for (uint8_t i = 0; i < idx; i++)
    { // 来自同一个定时器的中断且通道相同
        if (pwm_instance[i]->htim == htim && htim->Channel == pwm_instance[i]->channel)
        {
            if (pwm_instance[i]->callback) // 如果有回调函数
                pwm_instance[i]->callback(pwm_instance[i]);
            return; // 一次只能有一个通道的中断,所以直接返回
        }
    }
}

PWMInstance *PWMRegister(PWM_Init_Config_s *config)
{
    if (idx >= PWM_DEVICE_CNT) // 超过最大实例数,考虑增加或查看是否有内存泄漏
        while (1)
            ;
    PWMInstance *pwm = (PWMInstance *)malloc(sizeof(PWMInstance));
    memset(pwm, 0, sizeof(PWMInstance));

    pwm->htim = config->htim;
    pwm->channel = config->channel;
    pwm->period = config->period;
    pwm->pulse = config->pulse;
    pwm->callback = config->callback;
    pwm->id = config->id;
    // 启动PWM
    HAL_TIM_PWM_Start(pwm->htim, pwm->channel);                
    __HAL_TIM_SetCompare(pwm->htim, pwm->channel, pwm->pulse); // 设置占空比

    pwm_instance[idx++] = pwm;
    return pwm;
}

/* 只是对HAL的函数进行了形式上的封装 */
void PWMStart(PWMInstance *pwm)
{
    HAL_TIM_PWM_Start(pwm->htim, pwm->channel);
    __HAL_TIM_SetCompare(pwm->htim, pwm->channel, pwm->pulse);
}

/* 只是对HAL的函数进行了形式上的封装 */
void PWMStop(PWMInstance *pwm)
{
    HAL_TIM_PWM_Stop(pwm->htim, pwm->channel);
}

/* 只是对HAL的函数进行了形式上的封装 */
void PWMSetPulse(PWMInstance *pwm, uint32_t pulse)
{
    pwm->pulse = pulse;
    __HAL_TIM_SetCompare(pwm->htim, pwm->channel, pwm->pulse);
}

/* 只是对HAL的函数进行了形式上的封装 */
void PWMStartDMA(PWMInstance *pwm, uint32_t *pData, uint32_t Size)
{
    HAL_TIM_PWM_Start_DMA(pwm->htim, pwm->channel, pData, Size);
}