#include "bsp_temperature.h"

#warning this is a legacy support file, please use the new version

extern TIM_HandleTypeDef htim10;

void IMUTempInit()
{
    HAL_TIM_PWM_Start(&htim10, TIM_CHANNEL_1);
}

void IMUPWMSet(uint16_t pwm)
{
    __HAL_TIM_SetCompare(&htim10, TIM_CHANNEL_1, pwm);
}
