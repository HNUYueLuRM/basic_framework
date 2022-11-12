/**
 * @file dji_motor.h
 * @author neozng
 * @brief DJI智能电机头文件
 * @version 0.2
 * @date 2022-11-01
 *
 * @todo  1. 给不同的电机设置不同的低通滤波器惯性系数而不是统一使用宏
          2. 为M2006和M3508增加开环的零位校准函数

 * @copyright Copyright (c) 2022 HNU YueLu EC all rights reserved
 *
 */

#ifndef DJI_MOTOR_H
#define DJI_MOTOR_H

#include "bsp_can.h"
#include "controller.h"
#include "motor_def.h"

#define DJI_MOTOR_CNT 12
#define SPEED_SMOOTH_COEF 0.85f   // better to be greater than 0.8
#define CURRENT_SMOOTH_COEF 0.98f // this coef must be greater than 0.95

/* DJI电机CAN反馈信息*/
typedef struct
{
    uint16_t ecd; // 0-8192
    uint16_t last_ecd;
    int16_t speed_rpm;     // rounds per minute
    int16_t given_current; // 实际电流
    uint8_t temperate;
    int16_t total_round; // 总圈数,注意方向
    int32_t total_angle; // 总角度,注意方向
} dji_motor_measure;

/**
 * @brief DJI intelligent motor typedef
 *
 */
typedef struct
{
    /* motor measurement recv from CAN feedback */
    dji_motor_measure motor_measure;

    /* basic config of a motor*/
    Motor_Control_Setting_s motor_settings;

    /* controller used in the motor (3 loops)*/
    Motor_Controller_s motor_controller;

    /* the CAN instance own by motor instance*/
    can_instance motor_can_instance;

    /* sender assigment*/
    uint8_t sender_group;
    uint8_t message_num;

    Motor_Type_e motor_type;

} dji_motor_instance;

/**
 * @brief 调用此函数注册一个DJI智能电机,需要传递较多的初始化参数,请在application初始化的时候调用此函数
 *        推荐传参时像标准库一样构造initStructure然后传入此函数.
 *        recommend: type xxxinitStructure = {.member1=xx,
 *                                            .member2=xx,
 *                                             ....};
 *        请注意不要在一条总线上挂载过多的电机(超过6个),若一定要这么做,请降低每个电机的反馈频率(设为500Hz),
 *        并减小DJIMotorControl()任务的运行频率.
 *
 * @attention M3508和M2006的反馈报文都是0x200+id,而GM6020的反馈是0x204+id,请注意前两者和后者的id不要冲突.
 *            如果产生冲突,在初始化电机的时候会进入IDcrash_Handler(),可以通过debug来判断是否出现冲突.
 *
 * @param config 电机初始化结构体,包含了电机控制设置,电机PID参数设置,电机类型以及电机挂载的CAN设置
 *
 * @return dji_motor_instance*
 */
dji_motor_instance *DJIMotorInit(Motor_Init_Config_s config);

/**
 * @brief 被application层的应用调用,给电机设定参考值.
 *        对于应用,可以将电机视为传递函数为1的设备,不需要关心底层的闭环
 *
 * @param motor 要设置的电机
 * @param ref 设定参考值
 */
void DJIMotorSetRef(dji_motor_instance *motor, float ref);

/**
 * @brief 切换反馈的目标来源,如将角速度和角度的来源换为IMU(小陀螺模式常用)
 *
 * @param motor 要切换反馈数据来源的电机
 * @param loop  要切换反馈数据来源的控制闭环
 * @param type  目标反馈模式
 */
void DJIMotorChangeFeed(dji_motor_instance *motor, Closeloop_Type_e loop, Feedback_Source_e type);

/**
 * @brief 该函数被motor_task调用运行在rtos上,motor_stask内通过osDelay()确定控制频率
 * @todo  增加前馈功能
 */
void DJIMotorControl();

#endif // !DJI_MOTOR_H
