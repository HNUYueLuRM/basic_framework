#ifndef HT04_H
#define HT04_H

#include <stdint.h>
#include "bsp_can.h"
#include "controller.h"
#include "motor_def.h"

#define HT_MOTOR_CNT 4
#define CURRENT_SMOOTH_COEF 0.9f
#define SPEED_SMOOTH_COEF 0.85f

#define P_MIN -95.5f // Radians
#define P_MAX 95.5f
#define V_MIN -45.0f // Rad/s
#define V_MAX 45.0f
#define T_MIN -18.0f // N·m
#define T_MAX 18.0f

typedef struct // HT04
{
    float last_angle;
    float total_angle; // 角度为多圈角度,范围是-95.5~95.5,单位为rad
    float speed_rads;
    float real_current;
} HTMotor_Measure_t;

/* HT电机类型定义*/
typedef struct
{
    HTMotor_Measure_t measure;

    Motor_Control_Setting_s motor_settings;

    PIDInstance current_PID;
    PIDInstance speed_PID;
    PIDInstance angle_PID;
    float *other_angle_feedback_ptr;
    float *other_speed_feedback_ptr;
    float *speed_feedforward_ptr;
    float *current_feedforward_ptr;
    float pid_ref;

    Motor_Working_Type_e stop_flag; // 启停标志

    CANInstance *motor_can_instace;
} HTMotorInstance;

/* HT电机模式,初始化时自动进入CMD_MOTOR_MODE*/
typedef enum
{
    CMD_MOTOR_MODE = 0xfc,   // 使能,会响应指令
    CMD_RESET_MODE = 0xfd,   // 停止
    CMD_ZERO_POSITION = 0xfe // 将当前的位置设置为编码器零位
} HTMotor_Mode_t;

/**
 * @brief
 *
 * @param config
 * @return HTMotorInstance*
 */
HTMotorInstance *HTMotorInit(Motor_Init_Config_s *config);

/**
 * @brief 设定电机的参考值
 *
 * @param motor 要设定的电机
 * @param current   设定值
 */
void HTMotorSetRef(HTMotorInstance *motor, float ref);

/**
 * @brief 给所有的HT电机发送控制指令
 *
 */
void HTMotorControl();

/**
 * @brief 停止电机,之后电机不会响应HTMotorSetRef设定的值
 *
 * @param motor
 */
void HTMotorStop(HTMotorInstance *motor);

/**
 * @brief 启动电机
 *
 * @param motor 要启动的电机
 */
void HTMotorEnable(HTMotorInstance *motor);

/**
 * @brief 校准电机编码器
 * @attention 注意,校准时务必将电机和其他机构分离,电机会旋转360°!
 *            注意,校准时务必将电机和其他机构分离,电机会旋转360°!
 *            注意,校准时务必将电机和其他机构分离,电机会旋转360°!
 *            注意,校准时务必将电机和其他机构分离,电机会旋转360°!
 *
 * @param motor
 */
void HTMotorCalibEncoder(HTMotorInstance *motor);

#endif // !HT04_H#define HT04_H