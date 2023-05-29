#include "motor_task.h"
#include "LK9025.h"
#include "HT04.h"
#include "dji_motor.h"
#include "step_motor.h"
#include "servo_motor.h"

void MotorControlTask()
{
    // static uint8_t cnt = 0; 设定不同电机的任务频率
    // if(cnt%5==0) //200hz
    // if(cnt%10==0) //100hz
    DJIMotorControl();

    /* 如果有对应的电机则取消注释,可以加入条件编译或者register对应的idx判断是否注册了电机 */
    LKMotorControl();

    // legacy support
    // 由于ht04电机的反馈方式为接收到一帧消息后立刻回传,以此方式连续发送可能导致总线拥塞
    // 为了保证高频率控制,HTMotor中提供了以任务方式启动控制的接口,可通过宏定义切换
    // HTMotorControl();
    // 将所有的CAN设备集中在一处发送,最高反馈频率仅能达到500Hz,为了更好的控制效果,应使用新的HTMotorControlInit()接口

    ServeoMotorControl();

    // StepMotorControl();
}
