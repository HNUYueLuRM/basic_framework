#ifndef CHASSIS_H
#define CHASSIS_H

#include "robot_def.h"
#include "dji_motor.h"
#include "super_cap.h"

#ifdef CHASSIS_BOARD //使用板载IMU获取底盘转动角速度
#include "ins_task.h"
#endif // CHASSIS_BOARD

typedef struct 
{
#ifdef CHASSIS_BOARD
    IMU_Data_t Chassis_IMU_data;
#endif // CHASSIS_BOARD
    // SuperCAP cap;
    dji_motor_instance lf; //left right forward 
    dji_motor_instance rf;
    dji_motor_instance lb;
    dji_motor_instance rb;
    
} chassis;

void ChassisInit();

void ChassisTask();

#endif //CHASSIS_H