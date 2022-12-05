#ifndef __BSP_IMU_PWM_H
#define __BSP_IMU_PWM_H

#include "stdint.h"
#include "tim.h"

void IMUTempInit();
extern void IMUPWMSet(uint16_t pwm);

#endif
