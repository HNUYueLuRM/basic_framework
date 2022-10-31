#ifndef MOTOR_DEF_H
#define MOTOR_DEF_H

typedef enum
{
    CURRENT_LOOP = 0b0001,
    SPEED_LOOP = 0b0010,
    ANGLE_LOOP = 0b0100,

    // only for check
    _ = 0b0011,
    __ = 0b0110,
    ___ = 0b0111
} Closeloop_Type_e;

typedef enum
{
    MOTOR_FEED = 0,
    OTHER_FEED = 1
} Feedback_Source_e;

typedef enum
{
    MOTOR_DIRECTION_NORMAL = 0,
    MOTOR_DIRECTION_REVERSE = 1
} Reverse_Flag_e;

typedef struct
{
    Closeloop_Type_e close_loop_type;
    Reverse_Flag_e reverse_flag;
    Feedback_Source_e angle_feedback_source;
    Feedback_Source_e speed_feedback_source;

} Motor_Control_Setting_s;

typedef struct
{
    float *other_angle_feedback_ptr;
    float *other_speed_feedback_ptr;

    PID_t current_PID;
    PID_t speed_PID;
    PID_t angle_PID;

    // 将会作为每个环的输入和输出
    float pid_ref;
} Motor_Controller_s;

typedef enum
{
    GM6020 = 0,
    M3508 = 1,
    M2006 = 2,
    LK9025 = 3,
    HT04 = 4
} Motor_Type_e;

typedef struct
{
    float *other_angle_feedback_ptr;
    float *other_speed_feedback_ptr;

    PID_Init_config_s current_PID;
    PID_Init_config_s speed_PID;
    PID_Init_config_s angle_PID;

} Motor_Controller_Init_s;

#endif // !MOTOR_DEF_H
