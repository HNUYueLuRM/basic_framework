#include "dji_motor.h"

static dji_motor_instance* dji_motor_info[DJI_MOTOR_CNT]={NULL};

// can1: [0]:0x1FF,[1]:0x200,[2]:0x2FF
// can2: [0]:0x1FF,[1]:0x200,[2]:0x2FF
static can_instance sender_assignment[6]=
{
    [0]={.can_handle=&hcan1,.txconf.StdId=0x1ff,.txconf.IDE=CAN_ID_STD,.txconf.RTR=CAN_RTR_DATA,.txconf.DLC=0x08,.tx_buff={0}},
    [1]={.can_handle=&hcan1,.txconf.StdId=0x200,.txconf.IDE=CAN_ID_STD,.txconf.RTR=CAN_RTR_DATA,.txconf.DLC=0x08,.tx_buff={0}},
    [2]={.can_handle=&hcan1,.txconf.StdId=0x2ff,.txconf.IDE=CAN_ID_STD,.txconf.RTR=CAN_RTR_DATA,.txconf.DLC=0x08,.tx_buff={0}},
    [3]={.can_handle=&hcan2,.txconf.StdId=0x1ff,.txconf.IDE=CAN_ID_STD,.txconf.RTR=CAN_RTR_DATA,.txconf.DLC=0x08,.tx_buff={0}},
    [4]={.can_handle=&hcan2,.txconf.StdId=0x200,.txconf.IDE=CAN_ID_STD,.txconf.RTR=CAN_RTR_DATA,.txconf.DLC=0x08,.tx_buff={0}},
    [5]={.can_handle=&hcan2,.txconf.StdId=0x2ff,.txconf.IDE=CAN_ID_STD,.txconf.RTR=CAN_RTR_DATA,.txconf.DLC=0x08,.tx_buff={0}},
};


/**
 * @brief 
 * 
 * @param idx 
 */
static void MotorSenderGrouping(uint8_t idx,can_instance_config config)
{
    uint8_t motor_id=config.tx_id;
    uint8_t motor_rx_id;
    uint8_t motor_send_num;
    uint8_t motor_grouping;
    switch (dji_motor_info[idx]->motor_type)
    {
        case M2006:
        case M3508:
            if(motor_id<5)
            {

            }
            else
            {

            }
            break;
            
        case GM6020:
            if(motor_id<5)
            {
                
            }
            else
            {

            }
            break;
    }
}

static void DecodeDJIMotor(can_instance* _instance)
{
    for (size_t i = 0; i < DJI_MOTOR_CNT; i++)
    {
        if(dji_motor_info[i]->motor_can_instance==_instance)
        {
            dji_motor_info[i]->motor_measure.last_ecd = dji_motor_info[i]->motor_measure.ecd;                                   
            dji_motor_info[i]->motor_measure.ecd = (uint16_t)(_instance->rx_buff[0] << 8 | _instance->rx_buff[1]);            
            dji_motor_info[i]->motor_measure.speed_rpm = (uint16_t)(_instance->rx_buff[2] << 8 | _instance->rx_buff[3]);      
            dji_motor_info[i]->motor_measure.given_current = (uint16_t)(_instance->rx_buff[4] << 8 | _instance->rx_buff[5]);  
            dji_motor_info[i]->motor_measure.temperate = _instance->rx_buff[6];
            break;
        }
    }
}


dji_motor_instance* DJIMotorInit(can_instance_config config,
                                 Motor_Controller_s controller_config,
                                 Motor_Control_Setting_s motor_setting,
                                 Motor_Controller_Init_s controller_init,
                                 Motor_Type_e type)
{
    static uint8_t idx; // register idx
    dji_motor_info[idx]=(dji_motor_instance*)malloc(sizeof(dji_motor_instance));
    // motor setting
    dji_motor_info[idx]->motor_type=type;
    dji_motor_info[idx]->motor_settings=motor_setting;

    // motor controller init @todo : PID init
    dji_motor_info[idx]->motor_settings.angle_feedback_source=motor_setting.angle_feedback_source;
    dji_motor_info[idx]->motor_settings.speed_feedback_source=motor_setting.speed_feedback_source;
    // group motors, because 4 motors share the same CAN control message 
    MotorSenderGrouping(idx,config);
    // register motor to CAN bus
    dji_motor_info[idx]->motor_can_instance=CANRegister(config);
    
    return dji_motor_info[idx++];
}


void DJIMotorSetRef()
{


}


void DJIMotorControl()
{
    
    for (size_t i = 0; i < DJI_MOTOR_CNT; i++)
    {

    }
    
}