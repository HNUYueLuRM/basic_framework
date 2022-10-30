#ifndef MOTOR_TASK_H
#define MOTOR_TASK_H

#include "LK9025.h"
#include "HT04.h"
#include "dji_motor.h"
#include "motor_def.h"



void MotorControlTask();

void RegisterMotor(Motor_Type_e motor_type,void* motor_instance);

#endif // !MOTOR_TASK_H

