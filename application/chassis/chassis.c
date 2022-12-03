#include "chassis.h"
#include "robot_def.h"
#include "dji_motor.h"
#include "super_cap.h"
#include "message_center.h"
#include "arm_math.h"

#define OFFSET_X_CENTER  //纵向轮距(前进后退方向)
#define OFFSET_Y_CENTER  //横向轮距(左右平移方向)
#define RADIUS_WHEEL     //轮子半径
#define PERIMETER_WHEEL
#define REDUCTION_RATIO 19

/* 底盘应用包含的模块和信息存储 */
#ifdef CHASSIS_BOARD // 使用板载IMU获取底盘转动角速度
#include "ins_task.h"
IMU_Data_t *Chassis_IMU_data;
#endif // CHASSIS_BOARD
// static SuperCAP cap;
static dji_motor_instance *lf; // left right forward back
static dji_motor_instance *rf;
static dji_motor_instance *lb;
static dji_motor_instance *rb;

static Publisher_t* chassis_pub;
static Chassis_Ctrl_Cmd_s chassis_cmd_recv;
static Subscriber_t* chassis_sub;
static Chassis_Upload_Data_s chassis_feedback_data;

// 将云台系的速度投影到底盘
static float chassis_vx,chassis_vy; 


void ChassisInit()
{
    // 左前轮
    Motor_Init_Config_s left_foward_config = {
        .can_init_config = {
            .can_handle = &hcan2,
            .tx_id = 1,
        },
        .controller_param_init_config = {
            .angle_PID = {
                .Kd = 10,
                .Ki = 1,
                .Kd = 2,
            },
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
            .angle_PID = {
                .Kd = 10,
                .Ki = 1,
                .Kd = 2,
            },
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
            .angle_PID = {
                .Kd = 10,
                .Ki = 1,
                .Kd = 2,
            },
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
            .angle_PID = {
                .Kd = 10,
                .Ki = 1,
                .Kd = 2,
            },
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

    lf = DJIMotorInit(&left_foward_config);
    rf = DJIMotorInit(&right_foward_config);
    lb = DJIMotorInit(&left_back_config);
    rb = DJIMotorInit(&right_back_config);

    // SupercapInit();

    chassis_sub=SubRegister("chassis_cmd",sizeof(Chassis_Ctrl_Cmd_s));
    chassis_pub=PubRegister("chassis_feed",sizeof(Chassis_Upload_Data_s));
}


/**
 * @brief 计算每个轮毂电机的输出,正运动学解算
 * 
 */
static void MecanumCalculate()
{

}

/**
 * @brief 根据每个轮子的速度反馈,计算底盘的实际运动速度,逆运动解算
 * 
 */
static void EstimateSpeed()
{

}
// chassis_cmd_recv chassis_feedback_data
void ChassisTask()
{
    // 后续增加没收到消息的处理
    // 获取新的控制信息
    SubGetMessage(chassis_sub,&chassis_cmd_recv);

    if(chassis_cmd_recv.chassis_cmd.chassis_mode==CHASSIS_ZERO_FORCE)
    {
        DJIMotorStop();
    }

    // 根据云台和底盘的角度offset将控制量映射到底盘坐标系上
    chassis_vx=chassis_cmd_recv.chassis_cmd.vx*arm_cos_f32(chassis_cmd_recv.chassis_cmd.offset_angle)-
               chassis_cmd_recv.chassis_cmd.vy*arm_sin_f32(chassis_cmd_recv.chassis_cmd.offset_angle);
    chassis_vy=chassis_cmd_recv.chassis_cmd.vx*arm_sin_f32(chassis_cmd_recv.chassis_cmd.offset_angle)-
               chassis_cmd_recv.chassis_cmd.vy*arm_cos_f32(chassis_cmd_recv.chassis_cmd.offset_angle);

    // 根据控制模式设定旋转速度
    switch (chassis_cmd_recv.chassis_cmd.chassis_mode)
    {
    case CHASSIS_NO_FOLLOW:
        chassis_cmd_recv.chassis_cmd.wz=0;
        break;
    case CHASSIS_FOLLOW_GIMBAL_YAW:
        chassis_cmd_recv.chassis_cmd.wz=0.05f*powf(chassis_cmd_recv.chassis_cmd.wz,2.0f); // 不开pid,以误差角平方为输出
        break;
    case CHASSIS_ROTATE:
        //chassis_cmd_recv.chassis_cmd.wz 当前维持定值,后续增加不规则的变速策略
        break;
    default:
        break;
    }

    //根据控制模式进行正运动学解算,计算底盘输出
    MecanumCalculate();

    //根据电机的反馈速度计算
    EstimateSpeed();

    PubPushMessage(chassis_pub,&chassis_feedback_data);
}