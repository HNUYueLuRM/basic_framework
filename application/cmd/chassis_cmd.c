#include "robot_def.h"
#include "chassis_cmd.h"
#include "referee.h"
#include "message_center.h"


/* chassis_cmd应用包含的模块和信息存储 */
static referee_info_t *referee_data;               // 裁判系统的数据
#ifndef ONE_BOARD
#include "can_comm.h"
static CANCommInstance *chasiss_can_comm; // 双板通信CAN comm
#endif // !ONE_BOARD
//和chassis应用的交互
static Publisher_t* chassis_cmd_pub;
static Chassis_Ctrl_Cmd_s chassis_cmd;             // 发送给chassis应用的控制信息
static Subscriber_t* chassis_feed_sub;
static Chassis_Upload_Data_s chassis_fetch_data;   // chassis反馈的数据
//和gimbal_cmd的交互
static Publisher_t* chassis2gimbal_pub;
static Chassis2Gimbal_Data_s data_to_gimbal_cmd;   // 发送给gimbal_cmd的数据,包括热量限制,底盘功率等
static Subscriber_t* gimbal2chassis_sub;
static Gimbal2Chassis_Data_s data_from_gimbal_cmd; // 来自gimbal_cmd的数据,主要是底盘控制量


void ChassisCMDInit()
{
    referee_data= RefereeInit(&huart6);
    
    chassis_cmd_pub=PubRegister("chassis_cmd",sizeof(Chassis_Ctrl_Cmd_s));
    chassis_feed_sub=SubRegister("chassis_feed",sizeof(Chassis_Upload_Data_s));
    chassis2gimbal_pub=PubRegister("chassis2gimbal",sizeof(Chassis2Gimbal_Data_s));
    gimbal2chassis_sub=SubRegister("gimbal2chassis",sizeof(Gimbal2Chassis_Data_s));
}

void ChassisCMDTask()
{

}
