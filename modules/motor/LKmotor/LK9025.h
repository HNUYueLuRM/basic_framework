#ifndef LK9025_H
#define LK9025_H

#include "stdint.h"
#include "bsp_can.h"
#include "controller.h"
#include "motor_def.h"

#define LK_MOTOR_MX_CNT 4 // 最多允许4个LK电机使用多电机指令,挂载在一条总线上

#define I_MIN -2000
#define I_MAX 2000
#define CURRENT_SMOOTH_COEF 0.9f
#define SPEED_SMOOTH_COEF 0.85f
#define REDUCTION_RATIO_DRIVEN 1
#define ECD_ANGLE_COEF_LK (360.0f/65536.0f)

typedef struct // 9025
{
    uint16_t last_ecd;// 上一次读取的编码器值
    uint16_t ecd; // 
    float angle_single_round; // 单圈角度
    float speed_aps; // speed angle per sec(degree:°)
    int16_t real_current; // 实际电流
    uint8_t temperate; //温度,C°

    float total_angle; // 总角度
    int32_t total_round; //总圈数

} LKMotor_Measure_t;

typedef struct
{
    LKMotor_Measure_t measure;

    Motor_Control_Setting_s motor_settings;

    float *other_angle_feedback_ptr; // 其他反馈来源的反馈数据指针
    float *other_speed_feedback_ptr;
    float *speed_feedforward_ptr;
    float *current_feedforward_ptr;
    PIDInstance current_PID;
    PIDInstance speed_PID;
    PIDInstance angle_PID;
    float pid_ref;

    Motor_Working_Type_e stop_flag; // 启停标志

    CANInstance* motor_can_ins;
    
}LKMotorInstance;


LKMotorInstance *LKMotroInit(Motor_Init_Config_s* config);

void LKMotorSetRef(LKMotorInstance* motor,float ref);

void LKMotorControl();

void LKMotorStop(LKMotorInstance *motor);

void LKMotorEnable(LKMotorInstance *motor);

void LKMotorSetRef(LKMotorInstance *motor,float ref);


#endif // LK9025_H