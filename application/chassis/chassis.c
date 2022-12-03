#include "chassis.h"
#include "robot_def.h"
#include "dji_motor.h"
#include "super_cap.h"
#include "message_center.h"

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
static Chassis_Upload_Data_s chassis_feedback_data;
static Subscriber_t* chassis_sub;
static Chassis_Ctrl_Cmd_s chassis_cmd_recv;


static void mecanum_calculate()
{

}

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

void ChassisTask()
{

}