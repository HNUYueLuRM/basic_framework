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
#endif                                    // !ONE_BOARD

static RC_ctrl_t *remote_control_data;  // 遥控器数据,初始化时返回
static Vision_Recv_s *vision_recv_data; // 视觉接收数据指针,初始化时返回
static Vision_Send_s vision_send_data;  // 视觉发送数据

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

static Robot_Status_e robot_state;

void GimbalCMDInit()
{
    remote_control_data = RemoteControlInit(&huart3); // 修改为对应串口,注意dbus协议串口需加反相器
    vision_recv_data = VisionInit(&huart1);           // 视觉通信串口

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
static void RemoteControlSet()
{
    // 控制底盘和云台运行模式,云台待添加,云台是否始终使用IMU数据?
    if (switch_is_down(remote_control_data[0].rc.s[1])) // 右侧开关状态[下],底盘跟随云台
        chassis_cmd_send.chassis_mode = CHASSIS_FOLLOW_GIMBAL_YAW;
    if (switch_is_mid(remote_control_data[0].rc.s[1])) // 右侧开关状态[中],底盘和云台分离,底盘保持不转动
        chassis_cmd_send.chassis_mode = CHASSIS_NO_FOLLOW;


    // 云台参数,确定云台控制数据
    if (switch_is_mid(remote_control_data[0].rc.s[0])) // 左侧开关状态为[中],视觉模式
    {
        // 待添加
        // ...
    }
    // 侧开关状态为[下],或视觉未识别到目标,纯遥控器拨杆控制
    if (switch_is_down(remote_control_data[0].rc.s[0]) || vision_recv_data->target_state == NO_TARGET)
    { // 按照摇杆的输出大小进行角度增量,增益系数需调整
        gimbal_cmd_send.yaw += 0.04f * (float)remote_control_data[0].rc.joystick[2];
        gimbal_cmd_send.pitch = 0.5f * (float)remote_control_data[0].rc.joystick[3];
        gimbal_cmd_send.gimbal_mode = GIMBAL_GYRO_MODE;
    }


    // 底盘参数,目前没有加入小陀螺(调试似乎没有必要),系数需要调整
    chassis_cmd_send.vx = 1.0f * (float)remote_control_data[0].rc.joystick[0];
    chassis_cmd_send.vy = 1.0f * (float)remote_control_data[0].rc.joystick[1];


    // 发射参数
    if (switch_is_up(remote_control_data[0].rc.s[1])) //右侧开关状态[上],弹舱打开
       ; //弹舱舵机控制,待添加servo_motor模块,开启
    else
        ;//弹舱舵机控制,待添加servo_motor模块,关闭
    // 摩擦轮控制,后续可以根据左侧拨轮的值大小切换射频
    if(remote_control_data[0].rc.joystick[4]>100)
        shoot_cmd_send.shoot_mode=FRICTION_ON;
    else
        shoot_cmd_send.shoot_mode=FRICTION_OFF;
    // 拨弹控制,目前固定为连发
    if(remote_control_data[0].rc.joystick[4]>500)
        shoot_cmd_send.load_mode=LOAD_BURSTFIRE;
    else
        shoot_cmd_send.load_mode=LOAD_STOP;
    
}

/**
 * @brief 输入为键鼠时模式和控制量设置
 *
 */
static void MouseKeySet()
{
}

/**
 * @brief  紧急停止,包括遥控器左上侧拨轮打满/重要模块离线/双板通信失效等
 *         '300'待修改成合适的值,或改为开关控制
 *
 */
static void EmergencyHandler()
{
    // 拨轮的向下拨超过一半
    if (remote_control_data[0].rc.joystick[4] < -300) // 还需添加重要应用和模块离线的判断
    {
        robot_state = ROBOT_STOP; // 遥控器左上侧拨轮打满,进入紧急停止模式
        gimbal_cmd_send.gimbal_mode == GIMBAL_ZERO_FORCE;
        chassis_cmd_send.chassis_mode == CHASSIS_ZERO_FORCE;
        shoot_cmd_send.shoot_mode == SHOOT_STOP;
        return;
    }
    // if(remote_control_data[0].rc.joystick[4]>300 && 各个模块正常)
    // {
    //     //恢复运行
    //     //...
    // }
}

void GimbalCMDTask()
{
    // 从其他应用获取回传数据
    SubGetMessage(chassis_feed_sub, &chassis_fetch_data);
    SubGetMessage(shoot_feed_sub, &shoot_fetch_data);
    SubGetMessage(gimbal_feed_sub, &gimbal_fetch_data);

    // 根据gimbal的反馈值计算云台和底盘正方向的夹角,不需要传参,通过私有变量完成
    CalcOffsetAngle();

    if (switch_is_down(remote_control_data[0].rc.s[0])) // 遥控器左侧开关状态为[下],遥控器控制
        RemoteControlSet();
    else if (switch_is_up(remote_control_data[0].rc.s[0])) // 遥控器左侧开关状态为[上],键盘控制
        MouseKeySet();

    EmergencyHandler(); // 处理模块离线和遥控器急停等紧急情况

    // 设置视觉发送数据
    vision_send_data.bullet_speed = chassis_fetch_data.bullet_speed;
    vision_send_data.enemy_color = chassis_fetch_data.enemy_color;
    vision_send_data.pitch = gimbal_fetch_data.gimbal_imu_data.Pitch;
    vision_send_data.yaw = gimbal_fetch_data.gimbal_imu_data.Yaw;
    vision_send_data.roll = gimbal_fetch_data.gimbal_imu_data.Roll;

    // 推送消息,双板通信,视觉通信等
    // 应用所需的控制数据在remotecontrolsetmode和mousekeysetmode中完成设置
    PubPushMessage(chassis_cmd_pub, &chassis_cmd_send);
    PubPushMessage(shoot_cmd_pub, &shoot_cmd_send);
    PubPushMessage(gimbal_cmd_pub, &gimbal_cmd_send);
    VisionSend(&vision_send_data);
}
