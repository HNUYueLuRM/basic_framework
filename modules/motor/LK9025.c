#include "LK9025.h"
#include "stdlib.h"

static uint8_t idx;
static LKMotorInstance* lkmotor_instance[LK_MOTOR_MX_CNT]={NULL};

static void LKMotorDecode(CANInstance *_instance)
{
    static LKMotor_Measure_t* measure;
    static uint8_t* rx_buff;
    rx_buff=_instance->rx_buff;
    measure=&((LKMotorInstance*)_instance)->measure;
    
    measure->last_ecd=measure->ecd;
    measure->ecd=(uint16_t)((rx_buff[7] << 8) | rx_buff[6]);
    measure->angle_single_round=ECD_ANGLE_COEF*measure->ecd;
    measure->speed_aps=(1-SPEED_SMOOTH_COEF)*measure->speed_aps+
                       SPEED_SMOOTH_COEF*(float)((int16_t)(rx_buff[5] << 8 | rx_buff[4]));
    measure->real_current=(1-CURRENT_SMOOTH_COEF)*measure->real_current+
                          CURRENT_SMOOTH_COEF*(float)((int16_t)(rx_buff[3] << 8 | rx_buff[2]));
    measure->temperate=rx_buff[1];

    //计算多圈角度
    if (measure->ecd - measure->last_ecd > 32678)
        measure->total_round--;
    else if (measure->ecd - measure->last_ecd < -32678)
        measure->total_round++;
    measure->total_angle = measure->total_round * 360 + measure->angle_single_round;
}


void LKMotorControl()
{
    for (size_t i = 0; i < idx; i++)
    {
        
    }
}

LKMotorInstance *LKMotroInit(Motor_Init_Config_s *config)
{
    lkmotor_instance[idx]=(LKMotorInstance*)malloc(sizeof(LKMotorInstance));
    memset(lkmotor_instance[idx],0,sizeof(LKMotorInstance));

    lkmotor_instance[idx]->motor_settings=config->controller_setting_init_config;
    PID_Init(&lkmotor_instance[idx]->current_PID,&config->controller_param_init_config.current_PID);
    PID_Init(&lkmotor_instance[idx]->current_PID,&config->controller_param_init_config.current_PID);
    PID_Init(&lkmotor_instance[idx]->current_PID,&config->controller_param_init_config.current_PID);
    lkmotor_instance[idx]->other_angle_feedback_ptr=config->controller_param_init_config.other_angle_feedback_ptr;
    lkmotor_instance[idx]->other_speed_feedback_ptr=config->controller_param_init_config.other_speed_feedback_ptr;

    config->can_init_config.can_module_callback=LKMotorDecode;
    config->can_init_config.rx_id=0x140+config->can_init_config.tx_id;
    config->can_init_config.tx_id=config->can_init_config.tx_id+0x240;
    lkmotor_instance[idx]->motor_can_ins=CANRegister(&config->can_init_config);

    LKMotorEnable(lkmotor_instance[idx]);
    return lkmotor_instance[idx++];
}


void LKMotorStop(LKMotorInstance *motor)
{
    motor->stop_flag=MOTOR_STOP;
}

void LKMotorEnable(LKMotorInstance *motor)
{
    motor->stop_flag=MOTOR_ENALBED;
}

void LKMotorSetRef(LKMotorInstance *motor, float ref)
{
    motor->pid_ref=ref;
}
