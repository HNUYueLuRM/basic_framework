#include "motor_task.h"

static dji_motor_instance* dji_motor_info[DJI_MOTOR_CNT];
static joint_instance* joint_motor_info[HT_MOTOR_CNT];
static driven_instance* driven_motor_info[LK_MOTOR_CNT];


void RegisterMotor(Motor_Type_e motor_type,void* motor_instance)
{
    static uint8_t dji_idx,joint_idx,driven_idx;
    switch (motor_type)
    {
    case GM6020:
    case M3508:
    case M2006:
        dji_motor_info[dji_idx++]=(dji_motor_instance*)motor_instance;
        break;
    case LK9025:
        driven_motor_info[driven_idx++]=(driven_instance*)motor_instance;
        break;
    case HT04:
        joint_motor_info[joint_idx++]=(joint_instance*)motor_instance;
        break;
    }
}

void MotorControlTask()
{
    
}

