/**
 * @file chassis.c
 * @author NeoZeng neozng1@hnu.edu.cn
 * @brief 底盘应用,负责接收robot_cmd的控制命令并根据命令进行运动学解算,得到输出
 *        注意底盘采取右手系,对于平面视图,底盘纵向运动的正前方为x正方向;横向运动的右侧为y正方向
 *
 * @version 0.1
 * @date 2022-12-04
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "chassis.h"
#include "robot_def.h"
#include "dji_motor.h"
#include "super_cap.h"
#include "message_center.h"
#include "referee.h"

#include "arm_math.h"

/* 需要根据机器人底盘修改的参数,单位为mm(毫米) */
#define WHEEL_BASE 300           // 纵向轴距(前进后退方向)
#define TRACK_WIDTH 300          // 横向轮距(左右平移方向)
#define CENTER_GIMBAL_OFFSET_X 0 // 云台旋转中心距底盘几何中心的距离,前后方向,云台位于正中心时默认设为0
#define CENTER_GIMBAL_OFFSET_Y 0 // 云台旋转中心距底盘几何中心的距离,左右方向,云台位于正中心时默认设为0
#define RADIUS_WHEEL 60          // 轮子半径
#define REDUCTION_RATIO 19       // 电机减速比,因为编码器量测的是转子的速度而不是输出轴的速度故需进行转换

/* 自动计算的参数 */
#define HALF_WHEEL_BASE (WHEEL_BASE / 2.0f)
#define HALF_TRACK_WIDTH (TRACK_WIDTH / 2.0f)
#define PERIMETER_WHEEL (RADIUS_WHEEL * 2 * PI)

/* 底盘应用包含的模块和信息存储,底盘是单例模式,因此不需要为底盘建立单独的结构体 */
#ifdef CHASSIS_BOARD // 如果是底盘板,使用板载IMU获取底盘转动角速度
#include "can_comm.h"
#include "ins_task.h"
static CANCommInstance *chasiss_can_comm; // 双板通信CAN comm
IMU_Data_t *Chassis_IMU_data;
#endif // CHASSIS_BOARD
static referee_info_t *referee_data; // 裁判系统的数据
// static SuperCAP* cap; 尚未增加超级电容
static dji_motor_instance *motor_lf; // left right forward back
static dji_motor_instance *motor_rf; 
static dji_motor_instance *motor_lb;
static dji_motor_instance *motor_rb;

/* chassis 包含的信息交互模块和数据*/
static Publisher_t *chassis_pub;
static Chassis_Ctrl_Cmd_s chassis_cmd_recv;
static Subscriber_t *chassis_sub;
static Chassis_Upload_Data_s chassis_feedback_data;

/* 私有函数计算的中介变量,设为静态避免参数传递的开销 */
static float chassis_vx, chassis_vy;     // 将云台系的速度投影到底盘
static float vt_lf, vt_rf, vt_lb, vt_rb; // 底盘速度解算后的临时输出,待进行限幅

void ChassisInit()
{
    // 左前轮
    Motor_Init_Config_s left_foward_config = {
        .can_init_config = {
            .can_handle = &hcan2,
            .tx_id = 1,
        },
        .controller_param_init_config = {
            .speed_PID = {

            },
            .current_PID = {

            },
        },
        .controller_setting_init_config = {
            .angle_feedback_source = MOTOR_FEED,
            .speed_feedback_source = MOTOR_FEED,
            .close_loop_type = SPEED_LOOP | CURRENT_LOOP,
            .reverse_flag = MOTOR_DIRECTION_REVERSE,
        },
        .motor_type = M3508};
    // 右前轮
    Motor_Init_Config_s right_foward_config = {
        .can_init_config = {
            .can_handle = &hcan2,
            .tx_id = 2,
        },
        .controller_param_init_config = {
            .speed_PID = {

            },
            .current_PID = {

            },
        },
        .controller_setting_init_config = {
            .angle_feedback_source = MOTOR_FEED,
            .speed_feedback_source = MOTOR_FEED,
            .close_loop_type = SPEED_LOOP | CURRENT_LOOP,
            .reverse_flag = MOTOR_DIRECTION_REVERSE,
        },
        .motor_type = M3508};
    // 左后轮
    Motor_Init_Config_s left_back_config = {
        .can_init_config = {
            .can_handle = &hcan2,
            .tx_id = 3,
        },
        .controller_param_init_config = {
            .speed_PID = {

            },
            .current_PID = {

            },
        },
        .controller_setting_init_config = {
            .angle_feedback_source = MOTOR_FEED,
            .speed_feedback_source = MOTOR_FEED,
            .close_loop_type = SPEED_LOOP | CURRENT_LOOP,
            .reverse_flag = MOTOR_DIRECTION_REVERSE,
        },
        .motor_type = M3508};
    // 右后轮
    Motor_Init_Config_s right_back_config = {
        .can_init_config = {
            .can_handle = &hcan2,
            .tx_id = 4,
        },
        .controller_param_init_config = {
            .speed_PID = {

            },
            .current_PID = {

            },
        },
        .controller_setting_init_config = {
            .angle_feedback_source = MOTOR_FEED,
            .speed_feedback_source = MOTOR_FEED,
            .close_loop_type = SPEED_LOOP | CURRENT_LOOP,
            .reverse_flag = MOTOR_DIRECTION_REVERSE,
        },
        .motor_type = M3508};

    motor_lf = DJIMotorInit(&left_foward_config);
    motor_rf = DJIMotorInit(&right_foward_config);
    motor_lb = DJIMotorInit(&left_back_config);
    motor_rb = DJIMotorInit(&right_back_config);

    referee_data = RefereeInit(&huart6);
    // SupercapInit();

    chassis_sub = SubRegister("chassis_cmd", sizeof(Chassis_Ctrl_Cmd_s));
    chassis_pub = PubRegister("chassis_feed", sizeof(Chassis_Upload_Data_s));
}

#define LF_CENTER ((HALF_TRACK_WIDTH + CENTER_GIMBAL_OFFSET_X + HALF_WHEEL_BASE - CENTER_GIMBAL_OFFSET_Y) * ANGLE_2_RAD)
#define RF_CENTER ((HALF_TRACK_WIDTH - CENTER_GIMBAL_OFFSET_X + HALF_WHEEL_BASE - CENTER_GIMBAL_OFFSET_Y) * ANGLE_2_RAD)
#define LB_CENTER ((HALF_TRACK_WIDTH + CENTER_GIMBAL_OFFSET_X + HALF_WHEEL_BASE + CENTER_GIMBAL_OFFSET_Y) * ANGLE_2_RAD)
#define RB_CENTER ((HALF_TRACK_WIDTH - CENTER_GIMBAL_OFFSET_X + HALF_WHEEL_BASE + CENTER_GIMBAL_OFFSET_Y) * ANGLE_2_RAD)
/**
 * @brief 计算每个轮毂电机的输出,正运动学解算
 *        用宏进行预替换减小开销
 */
static void MecanumCalculate()
{
    vt_lf = -chassis_vx - chassis_vy - chassis_cmd_recv.wz * LF_CENTER;
    vt_rf = -chassis_vx + chassis_vy - chassis_cmd_recv.wz * RF_CENTER;
    vt_lb = chassis_vx + chassis_vy - chassis_cmd_recv.wz * LB_CENTER;
    vt_rb = chassis_vx - chassis_vy - chassis_cmd_recv.wz * RB_CENTER;
}

/**
 * @brief 根据裁判系统和电容剩余容量对输出进行限制并设置电机参考值
 *
 */
static void LimitChassisOutput()
{
    // 限制待添加
    // referee_data->PowerHeatData.chassis_power;
    // referee_data->PowerHeatData.chassis_power_buffer;

    DJIMotorSetRef(motor_lf, vt_lf);
    DJIMotorSetRef(motor_rf, vt_rf);
    DJIMotorSetRef(motor_lb, vt_lb);
    DJIMotorSetRef(motor_rb, vt_rb);
}

/**
 * @brief 根据每个轮子的速度反馈,计算底盘的实际运动速度,逆运动解算
 *        对于双板的情况,考虑增加来自底盘板IMU的数据
 *
 */
static void EstimateSpeed()
{
    // 根据电机速度和imu的速度解算
    // chassis_feedback_data.vx vy wz
    //  ...
    
}

void ChassisTask()
{
    // 后续增加没收到消息的处理
    // 获取新的控制信息
    SubGetMessage(chassis_sub, &chassis_cmd_recv);

    // 根据控制模式设定旋转速度
    switch (chassis_cmd_recv.chassis_mode)
    {
    case CHASSIS_NO_FOLLOW:
        chassis_cmd_recv.wz = 0; // 云台任意移动模式,不旋转,一般用于调整云台姿态
        break;
    case CHASSIS_FOLLOW_GIMBAL_YAW:
        chassis_cmd_recv.wz = 0.05f * powf(chassis_cmd_recv.wz, 2.0f); // 跟随,不开pid,以误差角平方为输出
        break;
    case CHASSIS_ROTATE:
        // chassis_cmd_recv.wz // 当前维持定值,后续增加不规则的变速策略
        break;
    case CHASSIS_ZERO_FORCE:
        DJIMotorStop(); // 如果出现重要模块离线或遥控器设置为急停,让电机停止
        break;
    default:
        break;
    }

    // 根据云台和底盘的角度offset将控制量映射到底盘坐标系上
    // 底盘逆时针旋转为角度正方向;云台命令的方向以云台指向的方向为x,采用右手系(x指向正北时y在正东)
    chassis_vx = chassis_cmd_recv.vx * arm_cos_f32(chassis_cmd_recv.offset_angle) -
                 chassis_cmd_recv.vy * arm_sin_f32(chassis_cmd_recv.offset_angle);
    chassis_vy = chassis_cmd_recv.vx * arm_sin_f32(chassis_cmd_recv.offset_angle) -
                 chassis_cmd_recv.vy * arm_cos_f32(chassis_cmd_recv.offset_angle);

    // 根据控制模式进行正运动学解算,计算底盘输出
    MecanumCalculate();

    // 根据裁判系统的反馈数据和电容数据对输出限幅
    LimitChassisOutput();

    // 根据电机的反馈速度计算
    EstimateSpeed();

    //获取裁判系统数据
    // 我方颜色id小于7是红色,大于7是蓝色,注意这里发送的是对方的颜色, 0:blue , 1:red
    chassis_feedback_data.enemy_color = referee_data->GameRobotStat.robot_id > 7 ? 1 : 0; // 
    chassis_feedback_data.bullet_speed = referee_data->GameRobotStat.shooter_id1_17mm_speed_limit;
    chassis_feedback_data.rest_heat=referee_data->PowerHeatData.shooter_heat0;

    // 推送反馈消息
    PubPushMessage(chassis_pub, &chassis_feedback_data);
}