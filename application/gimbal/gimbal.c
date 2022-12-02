#include "gimbal.h"
#include "robot_def.h"
#include "dji_motor.h"
#include "ins_task.h"

static IMU_Data_t Gimbal_IMU_data;                // 云台IMU数据
static dji_motor_instance yaw;                    // yaw电机
static dji_motor_instance pitch;                  // pitch电机
static Gimbal_Ctrl_Cmd_s gimbal_cmd_recv;         // 来自gimbal_cmd的控制信息
static Gimbal_Upload_Data_s gimbal_feedback_data; // 回传给gimbal_cmd的云台状态信息

void GimbalInit()
{
}

void GimbalTask()
{
}