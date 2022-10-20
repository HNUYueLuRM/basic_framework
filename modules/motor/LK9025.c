#include"LK9025.h"

static driven_instance* driven_motor_info[LK_MOTOR_CNT];

static void DecodeDriven(can_instance* _instance)
{
    for (size_t i = 0; i < LK_MOTOR_CNT; i++)
    {
        if(&driven_motor_info[i]->motor_can_instance==_instance)
        {
            driven_motor_info[i]->last_ecd = driven_motor_info[i]->ecd;                                   
            driven_motor_info[i]->ecd = (uint16_t)((_instance->rx_buff[7]<<8) | _instance->rx_buff[6]);             
            driven_motor_info[i]->speed_rpm = (uint16_t)(_instance->rx_buff[5] << 8 | _instance->rx_buff[4]);      
            driven_motor_info[i]->given_current = (uint16_t)(_instance->rx_buff[3] << 8 | _instance->rx_buff[2]);  
            driven_motor_info[i]->temperate = _instance->rx_buff[1];
            break;
        }
    }
}

driven_instance* LKMotroInit(CAN_HandleTypeDef* _hcan,uint8_t tx_id,uint8_t rx_id)
{
    static uint8_t idx;
    driven_motor_info[idx]=(driven_instance*)malloc(sizeof(driven_instance));
    driven_motor_info[idx++]->motor_can_instance=CANRegister(tx_id,rx_id,_hcan,DecodeDriven);
}

void DrivenControl(int16_t motor1_current,int16_t motor2_current)
{
    LIMIT_MIN_MAX(motor1_current,  I_MIN,  I_MAX);
    LIMIT_MIN_MAX(motor2_current,  I_MIN,  I_MAX);
    driven_motor_info[0]->motor_can_instance->tx_buff[0] = motor1_current;
    driven_motor_info[0]->motor_can_instance->tx_buff[1] = motor1_current>>8;
    driven_motor_info[0]->motor_can_instance->tx_buff[2] = motor2_current;
    driven_motor_info[0]->motor_can_instance->tx_buff[3] = motor2_current>>8;
    CANTransmit(&driven_motor_info[0]->motor_can_instance);
}

void SetDrivenMode(driven_mode cmd,uint16_t motor_id)
{
    static uint8_t buf[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00};
    // code goes here ...
    
    // CANTransmit(driven_mode)
}

