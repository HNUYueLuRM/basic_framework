/**
 * @file master_process.c
 * @author neozng
 * @brief  module for recv&send vision data
 * @version beta
 * @date 2022-11-03
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "Master_process.h"

/* use usart1 as vision communication*/
static Vision_Recv_s recv_data;
static usart_instance vision_usart_instance;

/**
 * @brief 接收解包回调函数,将在bsp_usart.c中被usart rx callback调用
 * @todo  1.提高可读性,将get_protocol_info的第四个参数增加一个float buffer
 *        2.添加标志位解码
 */
static void DecodeVision()
{
    static uint16_t flag_register;
    get_protocol_info(vision_usart_instance.recv_buff, &flag_register, &recv_data.pitch);
    // TODO: code to resolve flag_register;
}

/* 视觉通信初始化 */
void VisionInit(UART_HandleTypeDef *handle)
{
    vision_usart_instance.module_callback = DecodeVision;
    vision_usart_instance.recv_buff_size = VISION_RECV_SIZE;
    vision_usart_instance.usart_handle = handle;
    USARTRegister(&vision_usart_instance);
}

/**
 * @brief 发送函数
 * @todo 1.提高可读性,将get_protocol_info的第四个参数增加一个float buffer
 *       2.添加标志位解码
 *
 * @param send 待发送数据
 */
void VisionSend(Vision_Send_s *send)
{
    static uint16_t flag_register;
    static uint8_t send_buff[VISION_SEND_SIZE];
    static uint16_t tx_len;
    // TODO: code to set flag_register

    get_protocol_send_data(0x02, flag_register, &send->yaw, 3, send_buff, &tx_len);
    USARTSend(&vision_usart_instance, send_buff, tx_len);
}
