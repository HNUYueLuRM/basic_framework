#include "dji_motor.h"

static dji_motor_instance* dji_motor_info[DJI_MOTOR_CNT];


static void DecodeDJIMotor(can_instance* _instance)
{
    for (size_t i = 0; i < DJI_MOTOR_CNT; i++)
    {
        if(dji_motor_info[i]->motor_can_instance==_instance)
        {
            dji_motor_info[i]->last_ecd = dji_motor_info[i]->ecd;                                   
            dji_motor_info[i]->ecd = (uint16_t)(_instance->rx_buff[0] << 8 | _instance->rx_buff[1]);            
            dji_motor_info[i]->speed_rpm = (uint16_t)(_instance->rx_buff[2] << 8 | _instance->rx_buff[3]);      
            dji_motor_info[i]->given_current = (uint16_t)(_instance->rx_buff[4] << 8 | _instance->rx_buff[5]);  
            dji_motor_info[i]->temperate = _instance->rx_buff[6];
            break;
        }
    }
}

dji_motor_instance* DJIMotorInit(CAN_HandleTypeDef* _hcan,uint16_t tx_id,uint16_t rx_id)
{
    static uint8_t idx;
    dji_motor_info[idx]=(dji_motor_instance*)malloc(sizeof(dji_motor_instance));
    dji_motor_info[idx++]->motor_can_instance=CANRegister(tx_id,rx_id,_hcan,DecodeDJIMotor);
}

void DJIMotorControl()
{
    
}