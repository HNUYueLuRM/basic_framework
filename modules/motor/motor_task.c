#include "motor_task.h"
#include "LK9025.h"
#include "HT04.h"
#include "dji_motor.h"
#include "step_motor.h"
#include "servo_motor.h"


void MotorControlTask()
{
    // static uint8_t cnt = 0; 设定任务频率
    // if(cnt%5==0) //200hz
    // if(cnt%10==0) //100hz
    
    DJIMotorControl();

    //LKMotorControl();

    //HTMotorControl();

    //ServeoMotorControl();

    //StepMotorControl();
}

