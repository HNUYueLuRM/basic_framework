#include "robot_def.h"
#include "chassis_cmd.h"
#include "referee.h"

#ifndef ONE_BOARD
#include "can_comm.h"
static CANCommInstance *chasiss_can_comm; // 双板通信CAN comm
#endif                                    // !ONE_BOARD

static Chassis_Ctrl_Cmd_s chassis_cmd;             // 发送给chassis应用的控制信息
static Chassis_Upload_Data_s chassis_fetch_data;   // chassis反馈的数据
static Chassis2Gimbal_Data_s data_to_gimbal_cmd;   // 发送给gimbal_cmd的数据,包括热量限制,底盘功率等
static Gimbal2Chassis_Data_s data_from_gimbal_cmd; // 来自gimbal_cmd的数据,主要是底盘控制量
static referee_info_t *referee_data;               // 裁判系统的数据

void ChassisCMDInit()
{
}

void ChassisCMDTask()
{
}
