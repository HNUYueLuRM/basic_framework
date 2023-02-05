/**
 * @file master_process.c
 * @author neozng
 * @brief  module for recv&send vision data
 * @version beta
 * @date 2022-11-03
 * @todo 增加对串口调试助手协议的支持,包括vofa和serial debug
 * @copyright Copyright (c) 2022
 *
 */
#include "master_process.h"
#include "bsp_usart.h"
#include "usart.h"
#include "seasky_protocol.h"

static Vision_Recv_s recv_data;
// @todo:由于后续需要进行IMU-Cam的硬件触发采集控制,因此可能需要将发送设置为定时任务,或由IMU采集完成产生的中断唤醒的任务,
// 后者显然更nice,使得时间戳对齐. 因此,在send_data中设定其他的标志位数据,让ins_task填充姿态值.
// static Vision_Send_s send_data;
static USARTInstance *vision_usart_instance;

/**
 * @brief 接收解包回调函数,将在bsp_usart.c中被usart rx callback调用
 * @todo  1.提高可读性,将get_protocol_info的第四个参数增加一个float类型buffer
 *        2.添加标志位解码
 */
static void DecodeVision()
{
    static uint16_t flag_register;
    get_protocol_info(vision_usart_instance->recv_buff, &flag_register, (uint8_t *)&recv_data.pitch);
    // TODO: code to resolve flag_register;
}

/* 视觉通信初始化 */
Vision_Recv_s *VisionInit(UART_HandleTypeDef *_handle)
{
    USART_Init_Config_s conf;
    conf.module_callback = DecodeVision;
    conf.recv_buff_size = VISION_RECV_SIZE;
    conf.usart_handle = _handle;
    
    vision_usart_instance = USARTRegister(&conf);
    return &recv_data;
}

/**
 * @brief 发送函数
 * @todo 1.提高可读性,将get_protocol_send_data的第6个参数增加一个float buffer
 *       2.添加标志位解码
 *       3.由于后续需要进行IMU-Cam的硬件触发采集控制,因此可能需要将发送设置为定时任务
 *         或由IMU采集完成产生的中断唤醒的任务,使得时间戳对齐.
 *
 * @param send 待发送数据
 */
void VisionSend(Vision_Send_s *send)
{
    static uint16_t flag_register;
    static uint8_t send_buff[VISION_SEND_SIZE];
    static uint16_t tx_len;
    // TODO: code to set flag_register
    
    // 将数据转化为seasky协议的数据包
    get_protocol_send_data(0x02, flag_register, &send->yaw, 3, send_buff, &tx_len);
    USARTSend(vision_usart_instance, send_buff, tx_len);
}