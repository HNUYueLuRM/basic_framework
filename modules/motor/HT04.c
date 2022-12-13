#include "HT04.h"
#include "memory.h"
#include "general_def.h"

static uint8_t idx;
HTMotorInstance *ht_motor_info[HT_MOTOR_CNT];

/**
 * @brief
 *
 * @param cmd
 * @param motor
 */
static void HTMotorSetMode(HTMotor_Mode_t cmd, HTMotorInstance *motor)
{
    static uint8_t buf[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00};
    buf[7] = (uint8_t)cmd;
    memcpy(motor->motor_can_instace->tx_buff, buf, sizeof(buf));
    CANTransmit(motor->motor_can_instace);
}

/* 两个用于将uint值和float值进行映射的函数,在设定发送值和解析反馈值时使用 */
static uint16_t float_to_uint(float x, float x_min, float x_max, uint8_t bits)
{
    float span = x_max - x_min;
    float offset = x_min;
    return (uint16_t)((x - offset) * ((float)((1 << bits) - 1)) / span);
}
static float uint_to_float(int x_int, float x_min, float x_max, int bits)
{
    float span = x_max - x_min;
    float offset = x_min;
    return ((float)x_int) * span / ((float)((1 << bits) - 1)) + offset;
}

/**
 * @brief 解析电机反馈值
 *
 * @param motor_can 收到
 */
static void HTMotorDecode(CANInstance *motor_can)
{
    static uint16_t tmp; // 用于暂存解析值,稍后转换成float数据,避免多次创建临时变量
    static HTMotor_Measure_t *measure;
    static uint8_t *rxbuff;

    rxbuff = motor_can->rx_buff;
    measure = &((HTMotorInstance *)motor_can->id)->motor_measure;

    measure->last_angle = measure->total_angle;

    tmp = (uint16_t)((rxbuff[1] << 8) | rxbuff[2]);
    measure->total_angle = RAD_2_ANGLE * uint_to_float(tmp, P_MAX, P_MIN, 16);

    tmp = (uint16_t)((rxbuff[3] << 4) | (rxbuff[4] >> 4));
    measure->speed_aps = RAD_2_ANGLE * SPEED_SMOOTH_COEF * uint_to_float(tmp, V_MAX, V_MIN, 12) +
                         (1 - SPEED_SMOOTH_COEF) * measure->speed_aps;

    tmp = (uint16_t)(((rxbuff[4] & 0x0f) << 8) | rxbuff[5]);
    measure->real_current = CURRENT_SMOOTH_COEF * uint_to_float(tmp, T_MAX, T_MIN, 12) +
                            (1 - CURRENT_SMOOTH_COEF) * measure->real_current;
}

HTMotorInstance *HTMotorInit(Motor_Init_Config_s *config)
{

    ht_motor_info[idx] = (HTMotorInstance *)malloc(sizeof(HTMotorInstance));
    memset(ht_motor_info[idx], 0, sizeof(HTMotorInstance));

    ht_motor_info[idx]->motor_settings = config->controller_setting_init_config;
    PID_Init(&ht_motor_info[idx]->current_PID, &config->controller_param_init_config.current_PID);
    PID_Init(&ht_motor_info[idx]->speed_PID, &config->controller_param_init_config.speed_PID);
    PID_Init(&ht_motor_info[idx]->angle_PID, &config->controller_param_init_config.angle_PID);
    ht_motor_info[idx]->other_angle_feedback_ptr = config->controller_param_init_config.other_angle_feedback_ptr;
    ht_motor_info[idx]->other_speed_feedback_ptr = config->controller_param_init_config.other_speed_feedback_ptr;

    config->can_init_config.can_module_callback = HTMotorDecode;
    config->can_init_config.id = ht_motor_info[idx];
    ht_motor_info[idx]->motor_can_instace = CANRegister(&config->can_init_config);

    HTMotorEnable(ht_motor_info[idx]);
    return ht_motor_info[idx++];
}

void HTMotorSetRef(HTMotorInstance *motor, float ref)
{
    motor->pid_ref = ref;
}

void HTMotorControl()
{
    static uint16_t tmp;
    for (size_t i = 0; i < idx; i++)
    {
        // tmp=float_to_uint()
        // _instance->motor_can_instace->rx_buff[6] = tmp >> 8;
        // _instance->motor_can_instace->rx_buff[7] = tmp & 0xff;
        // CANTransmit(_instance->motor_can_instace);
    }
    
}

void HTMotorStop(HTMotorInstance *motor)
{
    HTMotorSetMode(CMD_RESET_MODE, motor);
}

void HTMotorEnable(HTMotorInstance *motor)
{
    HTMotorSetMode(CMD_MOTOR_MODE, motor);
}

void HTMotorCalibEncoder(HTMotorInstance *motor)
{
    HTMotorSetMode(CMD_ZERO_POSITION, motor);
}
