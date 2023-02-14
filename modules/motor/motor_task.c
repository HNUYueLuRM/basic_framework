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

    /* 如果有对应的电机则取消注释,可以加入条件编译或者register对应的idx判断是否注册了电机 */
    LKMotorControl();

    HTMotorControl();

    ServeoMotorControl();

    //StepMotorControl();
}

