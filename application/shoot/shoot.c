#include "shoot.h"
#include "robot_def.h"
#include "dji_motor.h"

static dji_motor_instance *friction_l;          // 左摩擦轮
static dji_motor_instance *friction_r;          // 右摩擦轮
static dji_motor_instance *loader;              // 拨盘电机
static Shoot_Ctrl_Cmd_s shoot_cmd_recv;         // 来自gimbal_cmd的发射控制信息
static Shoot_Upload_Data_s shoot_feedback_data; // 反馈回gimbal_cmd的发射状态信息

void ShootInit()
{
}

void ShootTask()
{
}