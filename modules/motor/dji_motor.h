#ifndef DJI_MOTOR_H
#define DJI_MOTOR_H

#define DJI_MOTOR_CNT 8

#include "bsp_can.h"
#include "controller.h"
#include "motor_def.h"

/**
 * @brief DJI intelligent motor typedef
 * 
 */
typedef struct
{
    /* motor measurement recv from CAN feedback */
    struct 
    {
        uint16_t ecd;
        uint16_t last_ecd;
        int16_t speed_rpm;
        int16_t given_current;
        uint8_t temperate;
        int16_t total_round;
        int32_t total_angle;
    } motor_measure;

    /* basic config of a motor*/
    Motor_Control_Setting_s motor_settings;

    /* controller used in the motor (3 loops)*/
    Motor_Controller_s motor_controller;

    /* the CAN instance own by motor instance*/
    can_instance *motor_can_instance;

    /* sender assigment*/
    uint8_t sender_group;
    uint8_t message_num;

    Motor_Type_e motor_type;

} dji_motor_instance;


dji_motor_instance* DJIMotorInit(can_instance_config config,
                                 Motor_Controller_s controller_config,
                                 Motor_Control_Setting_s motor_setting,
                                 Motor_Controller_Init_s controller_init,
                                 Motor_Type_e type);

void DJIMotorSetRef();

void DJIMotorControl();




#endif // !DJI_MOTOR_H
