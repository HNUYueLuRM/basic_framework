#include "bsp_buzzer.h"
#include "main.h"

extern TIM_HandleTypeDef htim4;
static uint8_t tmp_warning_level=0;

void buzzer_on(uint16_t psc, uint16_t pwm,uint8_t level)
{
    if(level>tmp_warning_level)
    {
        tmp_warning_level=level;
        __HAL_TIM_PRESCALER(&htim4, psc);
        __HAL_TIM_SetCompare(&htim4, TIM_CHANNEL_3, pwm);
    }
}
void buzzer_off(void)
{
    __HAL_TIM_SetCompare(&htim4, TIM_CHANNEL_3, 0);
    tmp_warning_level=0;
}
