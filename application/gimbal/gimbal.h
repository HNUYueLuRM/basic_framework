#ifndef GIMBAL_H
#define GIMBAL_H

#include "robot_def.h"
#include "dji_motor.h"
#include "ins_task.h"

typedef struct 
{
    IMU_Data_t Gimbal_IMU_data;
    dji_motor_instance yaw; 
    dji_motor_instance pitch;
} gimbal;

void GimbalInit();

void GimbalTask();

#endif //GIMBAL_H