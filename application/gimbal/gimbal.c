#include "gimbal.h"
#include "robot_def.h"
#include "dji_motor.h"
#include "ins_task.h"
#include "message_center.h"


#define YAW_ALIGN_ECD 0

static attitude_t* Gimbal_IMU_data;                // 云台IMU数据
static dji_motor_instance* yaw_motor;                    // yaw电机
static dji_motor_instance *pitch_motor;                  // pitch电机

static Publisher_t* gimbal_pub;
static Gimbal_Upload_Data_s gimbal_feedback_data; // 回传给gimbal_cmd的云台状态信息
static Subscriber_t* gimbal_sub;
static Gimbal_Ctrl_Cmd_s gimbal_cmd_recv;         // 来自gimbal_cmd的控制信息


void GimbalInit()
{
    Gimbal_IMU_data=INS_Init();

    // YAW
    Motor_Init_Config_s yaw_config = {
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
            .other_angle_feedback_ptr=&Gimbal_IMU_data->YawTotalAngle,
        },
        .controller_setting_init_config = {
            .angle_feedback_source = MOTOR_FEED,
            .speed_feedback_source = MOTOR_FEED,
            .close_loop_type = ANGLE_LOOP | SPEED_LOOP,
            .reverse_flag = MOTOR_DIRECTION_REVERSE,
        },
        .motor_type = GM6020};
    // PITCH
    Motor_Init_Config_s pitch_config = {
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
        },
        .controller_setting_init_config = {
            .angle_feedback_source = MOTOR_FEED,
            .speed_feedback_source = MOTOR_FEED,
            .close_loop_type = ANGLE_LOOP | SPEED_LOOP,
            .reverse_flag = MOTOR_DIRECTION_REVERSE,
        },
        .motor_type = GM6020};

        yaw_motor=DJIMotorInit(&yaw_config);
        pitch_motor=DJIMotorInit(&pitch_config);

        gimbal_pub=PubRegister("gimbal_feed",sizeof(Gimbal_Upload_Data_s));
        gimbal_sub=SubRegister("gimbal_cmd",sizeof(Gimbal_Ctrl_Cmd_s));
}

void GimbalTask()
{
}