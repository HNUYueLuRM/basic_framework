/**
 * @file motor_def.h
 * @author neozng
 * @brief  电机通用的数据结构定义
 * @version beta
 * @date 2022-11-01
 *
 * @copyright Copyright (c) 2022 HNU YueLu EC all rights reserved
 *
 */

#ifndef MOTOR_DEF_H
#define MOTOR_DEF_H

#include "controller.h"

/**
 * @brief 闭环类型,如果需要多个闭环,则使用或运算
 *        例如需要速度环和电流环: CURRENT_LOOP|SPEED_LOOP
 */
typedef enum
{
    OPEN_LOOP = 0b0000,
    CURRENT_LOOP = 0b0001,
    SPEED_LOOP = 0b0010,
    ANGLE_LOOP = 0b0100,

    // only for checking
    _ = 0b0011,
    __ = 0b0110,
    ___ = 0b0111
} Closeloop_Type_e;

/* 反馈来源设定,若设为OTHER_FEED则需要指定数据来源指针,详见Motor_Controller_s*/
typedef enum
{
    MOTOR_FEED = 0,
    OTHER_FEED = 1
} Feedback_Source_e;

/* 电机正反转标志 */
typedef enum
{
    MOTOR_DIRECTION_NORMAL = 0,
    MOTOR_DIRECTION_REVERSE = 1
} Reverse_Flag_e;

/* 电机控制设置,包括闭环类型,反转标志和反馈来源 */
typedef struct
{
    Closeloop_Type_e close_loop_type;
    Reverse_Flag_e reverse_flag;
    Feedback_Source_e angle_feedback_source;
    Feedback_Source_e speed_feedback_source;

} Motor_Control_Setting_s;

/* 电机控制器,包括其他来源的反馈数据指针,3环控制器和电机的参考输入*/
typedef struct
{
    float *other_angle_feedback_ptr;
    float *other_speed_feedback_ptr;

    PID_t current_PID;
    PID_t speed_PID;
    PID_t angle_PID;

    float pid_ref; // 将会作为每个环的输入和输出顺次通过串级闭环
} Motor_Controller_s;

/* 电机类型枚举 */
typedef enum
{
    GM6020 = 0,
    M3508 = 1,
    M2006 = 2,
    LK9025 = 3,
    HT04 = 4
} Motor_Type_e;

/**
 * @brief 电机控制器初始化结构体,包括三环PID的配置以及两个反馈数据来源指针
 *        如果不需要某个控制环,可以不设置对应的pid config
 *        需要其他数据来源进行反馈闭环,不仅要设置这里的指针还需要在Motor_Control_Setting_s启用其他数据来源标志
 */
typedef struct
{
    float *other_angle_feedback_ptr;
    float *other_speed_feedback_ptr;

    PID_Init_config_s current_PID;
    PID_Init_config_s speed_PID;
    PID_Init_config_s angle_PID;

} Motor_Controller_Init_s;

/* 用于初始化CAN电机的结构体,各类电机通用 */
typedef struct
{
    Motor_Controller_Init_s controller_param_init_config;
    Motor_Control_Setting_s controller_setting_init_config;
    Motor_Type_e motor_type;
    can_instance_config_s can_init_config;
} Motor_Init_Config_s;

#endif // !MOTOR_DEF_H
