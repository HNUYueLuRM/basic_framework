/**
 * @file dji_motor.h
 * @author neozng
 * @brief DJI智能电机头文件
 * @version 0.1
 * @date 2022-11-01
 * 
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
    uint16_t ecd;
    uint16_t last_ecd;
    int16_t speed_rpm;
    int16_t given_current;
    uint8_t temperate;
    int16_t total_round;
    int32_t total_angle;
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
 *        recommend: type xxxinitStructure = {
 *                                         .member1=xx,
 *                                         .member2=xx,
 *                                         ....};
 *        请注意不要在一条总线上挂载过多的电机(超过6个),若一定要这么做,请降低每个电机的反馈频率(设为500Hz),
 *        并减小DJIMotorControl()任务的运行频率.
 *
 * @attention M3508和M2006的反馈报文都是0x200+id,而GM6020的反馈是0x204+id,请注意前两者和后者的id不要冲突.
 *            如果产生冲突,在初始化电机的时候会进入IDcrash_Handler(),可以通过debug来判断是否出现冲突.
 *
 * @param config 电机can设置,对于DJI电机仅需要将tx_id设置为电调闪动次数(c620,615,610)或拨码开关的值(GM6020)
 *               你不需要自己查表计算发送和接收id,我们会帮你处理好!
 *
 * @param motor_setting 电机的控制设置,包括是否反转,闭环类型和是否使用编码器之外的反馈值
 *
 * @param controller_init 电机控制器的参数设置,包括其他的反馈来源数据指针和三环PID的参数.
 *                        如果不需要其他数据来源或不需要三个环,将不需要指针置为NULL即可
 *
 * @param type 电机的类型枚举,包括m2006,m3508和gm6020
 *
 * @return dji_motor_instance* 返回一个电机实例指针给应用,方便其对电机的参考值进行设置,即调用DJIMotorSetRef()
 */
dji_motor_instance *DJIMotorInit(can_instance_config config,
                                 Motor_Control_Setting_s motor_setting,
                                 Motor_Controller_Init_s controller_init,
                                 Motor_Type_e type);

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
