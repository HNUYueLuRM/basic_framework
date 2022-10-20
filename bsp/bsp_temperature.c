#include "bsp_temperature.h"

extern TIM_HandleTypeDef htim10;

void imu_pwm_set(uint16_t pwm)
{
    __HAL_TIM_SetCompare(&htim10, TIM_CHANNEL_1, pwm);
}

