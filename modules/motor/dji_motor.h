#ifndef DJI_MOTOR_H
#define DJI_MOTOR_H

#define DJI_MOTOR_CNT 8

#include "bsp_can.h"
#include "controller.h"

typedef struct
{
    uint16_t ecd;
    int16_t speed_rpm;
    int16_t given_current;
    uint8_t temperate;
    int16_t last_ecd;

    PID_t motor_pid;
    can_instance motor_can_instance;

} dji_motor_instance;

void DJIMotorInit(dji_motor_instance* motor_instace,CAN_HandleTypeDef* _hcan,uint16_t tx_id,uint16_t rx_id);

void DJIMotorControl();




#endif // !DJI_MOTOR_H