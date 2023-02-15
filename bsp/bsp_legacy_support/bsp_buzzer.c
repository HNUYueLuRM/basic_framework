#include "bsp_buzzer.h"
#include "main.h"

#warning this is a legacy support file, please use the new version

extern TIM_HandleTypeDef htim4;
static uint8_t tmp_warning_level = 0;

void BuzzerInit()
{
    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
}

void BuzzerOn(uint16_t psc, uint16_t pwm, uint8_t level)
{
    if (level > tmp_warning_level)
    {
        tmp_warning_level = level;
        __HAL_TIM_PRESCALER(&htim4, psc);
        __HAL_TIM_SetCompare(&htim4, TIM_CHANNEL_3, pwm);
    }
}

void BuzzerOff(void)
{
    __HAL_TIM_SetCompare(&htim4, TIM_CHANNEL_3, 0);
    tmp_warning_level = 0;
}
