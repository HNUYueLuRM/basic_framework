/*
 * @Descripttion: 
 * @version: 
 * @Author: Chenfu
 * @Date: 2022-12-05 12:39:18
 * @LastEditTime: 2022-12-05 13:37:36
 */
#ifndef VOFA_H
#define VOFA_H
#include <stdint.h>
#include "bsp_usart.h"
#include "usart.h"

typedef union
{
    float float_t;
    uint8_t uint8_t[4];
} send_float;
void vofa_justfloat_output(float *data, uint8_t num , UART_HandleTypeDef *huart);

#endif // !1#define 