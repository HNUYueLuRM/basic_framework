#ifndef MOTOR_TASK_H
#define MOTOR_TASK_H

#include "LK9025.h"
#include "HT04.h"
#include "dji_motor.h"

typedef enum 
{
    GM6020=0,
    M3508=1,
    M2006=2,
    LK9025=3,
    HT04=4
} Motor_Type_e;

void MotorControlTask();

void RegisterMotor(Motor_Type_e motor_type,void* motor_instance);

#endif // !MOTOR_TASK_H

