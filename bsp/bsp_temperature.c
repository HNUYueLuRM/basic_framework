#include "bsp_temperature.h"

extern TIM_HandleTypeDef htim10;

void IMUTempInit()
{
    HAL_TIM_PWM_Start(&htim10, TIM_CHANNEL_1);
}

void imu_pwm_set(uint16_t pwm)
{
    __HAL_TIM_SetCompare(&htim10, TIM_CHANNEL_1, pwm);
}
