#ifndef DJI_MOTOR_H
#define DJI_MOTOR_H

#define DJI_MOTOR_CNT 8

#include "bsp_can.h"
#include "controller.h"
#include "motor_def.h"

typedef struct
{
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

    struct 
    {
        closeloop_e close_loop_type;
        reverse_flag_e reverse_flag;
        feedback_source_e angle_feedback_source;
        feedback_source_e speed_feedback_source;
    } controll_state;

    float* other_angle_feedback_ptr;
    float* other_speed_feedback_ptr;

    PID_t* current_PID;
    PID_t* speed_PID;
    PID_t* angle_PID;

    can_instance *motor_can_instance;
} dji_motor_instance;

dji_motor_instance* DJIMotorInit(CAN_HandleTypeDef* _hcan,uint16_t tx_id,uint16_t rx_id);

void DJIMotorSetRef();

void DJIMotorControl();




#endif // !DJI_MOTOR_H
