#include "gimbal.h"
#include "robot_def.h"
#include "dji_motor.h"
#include "ins_task.h"
#include "message_center.h"
#include "general_def.h"

/* 根据每个机器人进行设定,若对云台有改动需要修改 */
#define YAW_CHASSIS_ALIGN_ECD 0 // 云台和底盘对齐指向相同方向时的电机编码器值
#define PITCH_HORIZON_ECD 0     // 云台处于水平位置时编码器值

// 自动将编码器转换成角度值
#define YAW_CHASSIS_ALIGN_ANGLE
#define PTICH_HORIZON_ALIGN_ANGLE

static attitude_t *Gimbal_IMU_data;     // 云台IMU数据
static dji_motor_instance *yaw_motor;   // yaw电机
static dji_motor_instance *pitch_motor; // pitch电机

static Publisher_t *gimbal_pub;
static Gimbal_Upload_Data_s gimbal_feedback_data; // 回传给gimbal_cmd的云台状态信息
static Subscriber_t *gimbal_sub;
static Gimbal_Ctrl_Cmd_s gimbal_cmd_recv; // 来自gimbal_cmd的控制信息

void GimbalInit()
{
    Gimbal_IMU_data = INS_Init(); // IMU先初始化,获取姿态数据指针赋给yaw电机的其他数据来源

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
            .other_angle_feedback_ptr = &Gimbal_IMU_data->YawTotalAngle,
            // 还需要增加角速度额外反馈指针
            // .other_speed_feedback_ptr=&Gimbal_IMU_data.wz;
        },
        .controller_setting_init_config = {
            .angle_feedback_source = MOTOR_FEED,
            .speed_feedback_source = MOTOR_FEED,
            .outer_loop_type = ANGLE_LOOP,
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
            .other_angle_feedback_ptr = &Gimbal_IMU_data->Pitch,
            // 还需要增加角速度额外反馈指针
            // .other_speed_feedback_ptr=&Gimbal_IMU_data.wy,
        },
        .controller_setting_init_config = {
            .angle_feedback_source = MOTOR_FEED,
            .speed_feedback_source = MOTOR_FEED,
            .outer_loop_type = ANGLE_LOOP,
            .close_loop_type = ANGLE_LOOP | SPEED_LOOP,
            .reverse_flag = MOTOR_DIRECTION_REVERSE,
        },
        .motor_type = GM6020};

    yaw_motor = DJIMotorInit(&yaw_config);
    pitch_motor = DJIMotorInit(&pitch_config);

    gimbal_pub = PubRegister("gimbal_feed", sizeof(Gimbal_Upload_Data_s));
    gimbal_sub = SubRegister("gimbal_cmd", sizeof(Gimbal_Ctrl_Cmd_s));
}

// /**
//  * @brief
//  *
//  */
// static void TransitionMode()
// {

// }

void GimbalTask()
{
    // 获取云台控制数据
    // 后续增加未收到数据的处理
    SubGetMessage(gimbal_sub, &gimbal_cmd_recv);

    // 根据控制模式进行电机反馈切换和过渡,视觉模式在robot_cmd模块就已经设置好,gimbal只看yaw_ref和pitch_ref
    //  是否要增加不同模式之间的过渡?
    switch (gimbal_cmd_recv.gimbal_mode)
    {
    case GIMBAL_ZERO_FORCE:
        DJIMotorStop(yaw_motor);
        DJIMotorStop(pitch_motor);
        break;
    case GIMBAL_GYRO_MODE:
        DJIMotorChangeFeed(yaw_motor, ANGLE_LOOP, OTHER_FEED);
        DJIMotorChangeFeed(yaw_motor, SPEED_LOOP, OTHER_FEED);
        DJIMotorChangeFeed(pitch_motor, ANGLE_LOOP, OTHER_FEED);
        DJIMotorChangeFeed(pitch_motor, SPEED_LOOP, OTHER_FEED);
        DJIMotorSetRef(yaw_motor, gimbal_cmd_recv.yaw);
        DJIMotorSetRef(pitch_motor, gimbal_cmd_recv.pitch);
        break;
    // 是否考虑直接让云台和底盘重合,其他模式都使用陀螺仪反馈?
    case GIMBAL_FREE_MODE:
        DJIMotorChangeFeed(yaw_motor, ANGLE_LOOP, MOTOR_FEED);
        DJIMotorChangeFeed(yaw_motor, SPEED_LOOP, MOTOR_FEED);
        DJIMotorChangeFeed(pitch_motor, ANGLE_LOOP, MOTOR_FEED);
        DJIMotorChangeFeed(pitch_motor, SPEED_LOOP, MOTOR_FEED);
        DJIMotorSetRef(yaw_motor, YAW_ALIGN_ECD);
        DJIMotorSetRef(pitch_motor, PITCH_HORIZON_ECD);
        break;
    default:
        break;
    }
    // 过渡示例:
    /* 需要给每个case增加如下判断,并添加一个过渡行为函数和过渡标志位
    case xxx:
        if(last_mode!=xxx)
        {
            transition_flag=1;
        }
        break;

    void TransitMode()
    {
        motor_output=lpf_coef * last_output+(1 - lpf_coef) * concur_output;
    }
    */

    // 获取反馈数据
    gimbal_feedback_data.gimbal_imu_data = *Gimbal_IMU_data;
    gimbal_feedback_data.yaw_motor_ecd = pitch_motor->motor_measure.ecd;

    // 推送消息
    PubPushMessage(gimbal_pub, &gimbal_feedback_data);
}