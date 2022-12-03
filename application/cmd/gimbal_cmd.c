#include "robot_def.h"
#include "gimbal_cmd.h"
#include "remote_control.h"
#include "ins_task.h"
#include "master_process.h"
#include "message_center.h"


/* gimbal_cmd应用包含的模块实例指针和交互信息存储*/
#ifndef ONE_BOARD
#include "can_comm.h"
static CANCommInstance *chasiss_can_comm; //双板通信
#endif // !ONE_BOARD
static RC_ctrl_t *remote_control_data;              // 遥控器数据
static Vision_Recv_s *vision_recv_data;             // 视觉接收数据
static Vision_Send_s *vision_send_data;             // 视觉发送数据

static Publisher_t* gimbal_cmd_pub;
static Gimbal_Ctrl_Cmd_s gimbal_cmd_send;           // 传递给云台的控制信息
static Subscriber_t* gimbal_cmd_feed_sub;
static Gimbal_Upload_Data_s gimbal_fetch_data;      // 从云台获取的反馈信息

static Publisher_t* shoot_cmd_pub;
static Shoot_Ctrl_Cmd_s shoot_cmd_send;             // 传递给发射的控制信息
static Subscriber_t* shoot_cmd_feed_sub;
static Shoot_Upload_Data_s shoot_fetch_data;        // 从发射获取的反馈信息

static Publisher_t* gimbal2chassis_pub;
static Gimbal2Chassis_Data_s data_to_chassis_cmd;   // 发送给底盘CMD应用的控制信息,主要是遥控器和UI绘制相关
static Subscriber_t* chassis2gimbal_sub;
static Chassis2Gimbal_Data_s data_from_chassis_cmd; // 从底盘CMD应用接收的控制信息,底盘功率枪口热量等


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
    remote_control_data=RC_init(&huart3);
    vision_recv_data=VisionInit(&huart1);

    gimbal_cmd_pub=PubRegister("gimbal_cmd",sizeof(Gimbal_Ctrl_Cmd_s));
    gimbal_cmd_feed_sub=SubRegister("gimbal_feed",sizeof(Gimbal_Upload_Data_s));
    shoot_cmd_pub=PubRegister("shoot_cmd",sizeof(Shoot_Ctrl_Cmd_s));
    shoot_cmd_feed_sub=SubRegister("shoot_feed",sizeof(Shoot_Upload_Data_s));
    gimbal2chassis_pub=PubRegister("gimbal2chassis",sizeof(Gimbal2Chassis_Data_s));
    chassis2gimbal_sub=SubRegister("chassis2gimbal",sizeof(Chassis2Gimbal_Data_s));
}

void GimbalCMDTask()
{
}
