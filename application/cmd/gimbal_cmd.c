#include "robot_def.h"
#include "gimbal_cmd.h"
#include "remote_control.h"
#include "ins_task.h"
#include "master_process.h"

#ifndef ONE_BOARD
#include "can_comm.h"
static CANCommInstance *chasiss_can_comm;
#endif // !ONE_BOARD

static Gimbal_Ctrl_Cmd_s gimbal_cmd_send;           // 传递给云台的控制信息
static Shoot_Ctrl_Cmd_s shoot_cmd_send;             // 传递给发射的控制信息
static Gimbal_Upload_Data_s gimbal_fetch_data;      // 从云台获取的反馈信息
static Shoot_Upload_Data_s shoot_fetch_data;        // 从发射获取的反馈信息
static Gimbal2Chassis_Data_s data_to_chassis_cmd;   // 发送给底盘CMD应用的控制信息,主要是遥控器和UI绘制相关
static Chassis2Gimbal_Data_s data_from_chassis_cmd; // 从底盘CMD应用接收的控制信息,底盘功率枪口热量等
static RC_ctrl_t *remote_control_data;              // 遥控器数据
static Vision_Recv_s *vision_recv_data;             // 视觉接收数据
static Vision_Send_s *vision_send_data;             // 视觉发送数据

static void CalcOffsetAngle()
{
}

static void SetRobotMode()
{
}

static void SetCtrlData()
{
}

static void SetCtrlMessage()
{
}

void GimbalCMDInit()
{
}

void GimbalCMDTask()
{
}
