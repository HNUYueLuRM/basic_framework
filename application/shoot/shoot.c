#include "shoot.h"
#include "robot_def.h"
#include "dji_motor.h"
#include "message_center.h"

/* 对于双发射机构的机器人,将下面的数据封装成结构体即可,生成两份shoot应用实例
 */
static dji_motor_instance *friction_l; // 左摩擦轮
static dji_motor_instance *friction_r; // 右摩擦轮
static dji_motor_instance *loader;     // 拨盘电机

static Publisher_t *shoot_pub;
static Shoot_Ctrl_Cmd_s shoot_cmd_recv; // 来自gimbal_cmd的发射控制信息
static Subscriber_t *shoot_sub;
static Shoot_Upload_Data_s shoot_feedback_data; // 来自gimbal_cmd的发射控制信息

void ShootInit()
{
    // 左摩擦轮
    Motor_Init_Config_s left_friction_config = {
        .can_init_config = {
            .can_handle = &hcan1,
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
    // 右摩擦轮
    Motor_Init_Config_s right_friction_config = {
        .can_init_config = {
            .can_handle = &hcan1,
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
    //  拨盘电机
    Motor_Init_Config_s loader_config = {
        .can_init_config = {
            .can_handle = &hcan1,
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
        .motor_type = M2006};

    friction_l = DJIMotorInit(&left_friction_config);
    friction_r = DJIMotorInit(&right_friction_config);
    loader = DJIMotorInit(&loader_config);

    shoot_pub = PubRegister("shoot_feed", sizeof(Shoot_Upload_Data_s));
    shoot_sub = SubRegister("shoot_cmd", sizeof(Shoot_Ctrl_Cmd_s));
}

void ShootTask()
{
}