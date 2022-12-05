#include "robot_def.h"
#include "robot_cmd.h"
#include "remote_control.h"
#include "ins_task.h"
#include "master_process.h"
#include "message_center.h"
#include "general_def.h"
#include "dji_motor.h"

// 自动将编码器转换成角度值
#define YAW_ALIGN_ANGLE (YAW_CHASSIS_ALIGN_ECD * ECD_ANGLE_COEF)
#define PTICH_HORIZON_ANGLE (PITCH_HORIZON_ECD * ECD_ANGLE_COEF)

/* gimbal_cmd应用包含的模块实例指针和交互信息存储*/
#ifndef ONE_BOARD
#include "can_comm.h"
static CANCommInstance *chasiss_can_comm; // 双板通信
#endif // !ONE_BOARD

static RC_ctrl_t *remote_control_data;    // 遥控器数据,初始化时返回
static Vision_Recv_s *vision_recv_data;   // 视觉接收数据指针,初始化时返回
static Vision_Send_s vision_send_data;    // 视觉发送数据

static Publisher_t *gimbal_cmd_pub;
static Gimbal_Ctrl_Cmd_s gimbal_cmd_send; // 传递给云台的控制信息
static Subscriber_t *gimbal_feed_sub;
static Gimbal_Upload_Data_s gimbal_fetch_data; // 从云台获取的反馈信息

static Publisher_t *shoot_cmd_pub;
static Shoot_Ctrl_Cmd_s shoot_cmd_send; // 传递给发射的控制信息
static Subscriber_t *shoot_feed_sub;
static Shoot_Upload_Data_s shoot_fetch_data; // 从发射获取的反馈信息

static Publisher_t *chassis_cmd_pub;
static Chassis_Ctrl_Cmd_s chassis_cmd_send; // 发送给底盘应用的信息,包括控制信息和UI绘制相关
static Subscriber_t *chassis_feed_sub;
static Chassis_Upload_Data_s chassis_fetch_data; // 从底盘应用接收的反馈信息信息,底盘功率枪口热量与底盘运动状态等

void GimbalCMDInit()
{
    remote_control_data = RC_init(&huart3); // 修改为对应串口,注意dbus协议串口需加反相器
    vision_recv_data = VisionInit(&huart1); // 视觉通信串口

    gimbal_cmd_pub = PubRegister("gimbal_cmd", sizeof(Gimbal_Ctrl_Cmd_s));
    gimbal_feed_sub = SubRegister("gimbal_feed", sizeof(Gimbal_Upload_Data_s));
    shoot_cmd_pub = PubRegister("shoot_cmd", sizeof(Shoot_Ctrl_Cmd_s));
    shoot_feed_sub = SubRegister("shoot_feed", sizeof(Shoot_Upload_Data_s));
    chassis_cmd_pub = PubRegister("gimbal2chassis", sizeof(Chassis_Ctrl_Cmd_s));
    chassis_feed_sub = SubRegister("chassis2gimbal", sizeof(Chassis_Upload_Data_s));
}

/**
 * @brief 根据gimbal app传回的当前电机角度计算和零位的误差
 *        单圈绝对角度的范围是0~360,说明文档中有图示
 *
 */
static void CalcOffsetAngle()
{
    static float angle; // 提高可读性,不然太长了不好看,虽然基本不会动这个函数
    angle = gimbal_fetch_data.yaw_motor_single_round_angle;
#if YAW_ECD_GREATER_THAN_4096 // 如果大于180度
    if (angle > YAW_ALIGN_ANGLE && angle <= 180.0f + YAW_ALIGN_ANGLE)
        chassis_cmd_send.offset_angle = angle - YAW_ALIGN_ANGLE;
    else if (angle > 180.0f + YAW_ALIGN_ANGLE)
        chassis_cmd_send.offset_angle = angle - YAW_ALIGN_ANGLE - 360.0f;
    else
        chassis_cmd_send.offset_angle = angle - YAW_ALIGN_ANGLE;
#else // 小于180度
    if (angle > YAW_ALIGN_ANGLE)
        chassis_cmd_send.offset_angle = angle - YAW_ALIGN_ANGLE;
    else if (angle <= YAW_ALIGN_ANGLE && angle >= YAW_ALIGN_ANGLE - 180.0f)
        chassis_cmd_send.offset_angle = angle - YAW_ALIGN_ANGLE;
    else
        chassis_cmd_send.offset_angle = angle - YAW_ALIGN_ANGLE + 360.0f;
#endif
}

/**
 * @brief 输入为遥控器(调试时)的模式和控制量设置
 *
 */
static void RemoteControlSetMode()
{
}

/**
 * @brief 输入为键鼠时模式和控制量设置
 *
 */
static void MouseKeySetMode()
{
}

void GimbalCMDTask()
{
    // 从其他应用获取回传数据
    SubGetMessage(chassis_feed_sub, &chassis_fetch_data);
    SubGetMessage(shoot_feed_sub, &shoot_fetch_data);
    SubGetMessage(gimbal_feed_sub, &gimbal_fetch_data);

    // 根据gimbal的反馈值计算云台和底盘正方向的夹角,不需要传参,通过私有变量完成
    CalcOffsetAngle();

    if (1) // 遥控器控制
        RemoteControlSetMode();
    else if (0) // 键盘控制
        MouseKeySetMode();

    // 设置视觉发送数据,work_mode在前一部分设置
    vision_send_data.bullet_speed=chassis_fetch_data.bullet_speed;
    vision_send_data.enemy_color=chassis_fetch_data.enemy_color;
    vision_send_data.pitch=gimbal_fetch_data.gimbal_imu_data.Pitch;
    vision_send_data.yaw=gimbal_fetch_data.gimbal_imu_data.Yaw;
    vision_send_data.roll=gimbal_fetch_data.gimbal_imu_data.Roll;

    // 推送消息,双板通信,视觉通信等
    // 应用所需的控制数据在remotecontrolsetmode和mousekeysetmode中完成设置
    PubPushMessage(chassis_cmd_pub, &chassis_cmd_send);
    PubPushMessage(shoot_cmd_pub, &shoot_cmd_send);
    PubPushMessage(gimbal_cmd_pub, &gimbal_cmd_send);
    VisionSend(&vision_send_data);
}
