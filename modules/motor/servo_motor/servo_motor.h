#ifndef SERVO_MOTOR_H
#define SERVO_MOTOR_H

#include "main.h"
#include "tim.h"
#include <stdint-gcc.h>
#include "bsp_pwm.h"
#include "bsp_usart.h"

#define SERVO_MOTOR_CNT 7
#define Servo_Frame_First 0x55
#define Servo_Frame_Second 0x55
#define Servo_MAX_BUFF 10
#define SERVO_MOVE_CMD 0x03
#define SERVO_UNLOAD_CMD 0x14
#define SERVO_POS_READ_CMD 0x15
typedef enum
{
    Servo_None_Type = 0,
    Bus_Servo = 1,
    PWM_Servo = 2,
}ServoType_e;


/* 用于初始化不同舵机的结构体,各类舵机通用 */
typedef struct
{
    PWM_Init_Config_s pwm_init_config;
    ServoType_e servo_type;
    UART_HandleTypeDef *_handle;
    uint8_t servo_id;
}Servo_Init_Config_s;
typedef struct
{   
    uint8_t servo_id;
    float angle;
    uint16_t recv_angle;
    PWMInstance *pwm_instance;
    USARTInstance *usart_instance;
    ServoType_e servo_type;
}ServoInstance;

ServoInstance *ServoInit(Servo_Init_Config_s *Servo_Init_Config);
void ServoSetAngle(ServoInstance *servo, float angle);
#endif // SERVO_MOTOR_H