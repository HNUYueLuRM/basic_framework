#ifndef HC05_H
#define HC05_H

#include <stdint.h>
#include "main.h"
#include "usart.h"

#define  HC05_DATASIZE  4   // HC05接收和发送数据大小，根据需要修改

// HC05通信数据结构体，后续根据需要添加和修改
typedef struct 
{
    uint8_t  send_data[HC05_DATASIZE + 2];  // 发送数据，加上帧头和帧尾
    uint8_t  recv_data[HC05_DATASIZE];      // 接收数据
}HC05;

// HC05串口接收初始化
HC05 *HC05Init(UART_HandleTypeDef *hc05_usart_handle);

// HC05串口发送函数，一次最多发送HC05_DATASIZE个数据
void HC05_SendData( uint8_t *data, uint8_t data_num);

#endif
