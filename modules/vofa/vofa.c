/*
 * @Descripttion: 
 * @version: 
 * @Author: Chenfu
 * @Date: 2022-12-05 12:39:07
 * @LastEditTime: 2022-12-05 14:15:53
 */
#include "vofa.h"

/*VOFA浮点协议*/
void vofa_justfloat_output(float *data, uint8_t num , UART_HandleTypeDef *huart )
{
    static uint8_t i = 0;
    send_float temp[num];			//定义缓冲区数组
    uint8_t send_data[4 * num + 4]; //定义通过串口传出去的数组，数量是所传数据的字节数加上4个字节的尾巴
    for (i = 0; i < num; i++)
    {
        temp[i].float_t = data[i]; //将所传数据移到缓冲区数组
    }
    for (i = 0; i < num; i++)
    {
        send_data[4 * i] = temp[i].uint8_t[0];
        send_data[4 * i + 1] = temp[i].uint8_t[1];
        send_data[4 * i + 2] = temp[i].uint8_t[2];
        send_data[4 * i + 3] = temp[i].uint8_t[3]; //将缓冲区数组内的浮点型数据转成4个字节的无符号整型，之后传到要通过串口传出的数组里
    }
    send_data[4 * num] = 0x00;
    send_data[4 * num + 1] = 0x00;
    send_data[4 * num + 2] = 0x80;
    send_data[4 * num + 3] = 0x7f; //加上协议要求的4个尾巴

    HAL_UART_Transmit(huart, (uint8_t *)send_data, 4 * num + 4, 100);
}
