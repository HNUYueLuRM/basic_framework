#ifndef MASTER_PROCESS_H
#define MASTER_PROCESS_H

#include "bsp_usart.h"
#include "usart.h"
#include "seasky_protocol.h"

#define VISION_RECV_SIZE 36u
#define VISION_SEND_SIZE 36u


typedef struct
{
    
};



/**
 * @brief 调用此函数初始化和视觉的串口通信
 * 
 * @param handle 用于和视觉通信的串口handle(C板上一般为USART1,丝印为USART2,4pin)
 */
Vision_Recv_s* VisionInit(UART_HandleTypeDef* handle);

/**
 * @brief 发送视觉视觉
 * 
 * @param send 视觉需要的数据
 */
void VisionSend(Vision_Send_s *send);


#endif // !MASTER_PROCESS_H