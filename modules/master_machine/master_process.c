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
#include "daemon.h"
#include "bsp_log.h"

static Vision_Recv_s recv_data;
// @todo:由于后续需要进行IMU-Cam的硬件触发采集控制,因此可能需要将发送设置为定时任务,或由IMU采集完成产生的中断唤醒的任务,
// 后者显然更nice,使得时间戳对齐. 因此,在send_data中设定其他的标志位数据,让ins_task填充姿态值.
// static Vision_Send_s send_data;
static USARTInstance *vision_usart_instance;
static DaemonInstance *vision_daemon_instance;

/**
 * @brief 接收解包回调函数,将在bsp_usart.c中被usart rx callback调用
 * @todo  1.提高可读性,将get_protocol_info的第四个参数增加一个float类型buffer
 *        2.添加标志位解码
 */
static void DecodeVision()
{
    uint16_t flag_register;
    DaemonReload(vision_daemon_instance); // 喂狗
    get_protocol_info(vision_usart_instance->recv_buff, &flag_register, (uint8_t *)&recv_data.pitch);
    // TODO: code to resolve flag_register;
}

/**
 * @brief 离线回调函数,将在daemon.c中被daemon task调用
 * @attention 由于HAL库的设计问题,串口开启DMA接收之后同时发送有概率出现__HAL_LOCK()导致的死锁,使得无法
 *            进入接收中断.通过daemon判断数据更新,重新调用服务启动函数以解决此问题.
 *
 * @param id vision_usart_instance的地址,此处没用.
 */
static void VisionOfflineCallback(void *id)
{
    USARTServiceInit(vision_usart_instance);
}

/* 视觉通信初始化 */
Vision_Recv_s *VisionInit(UART_HandleTypeDef *_handle)
{
    USART_Init_Config_s conf;
    conf.module_callback = DecodeVision;
    conf.recv_buff_size = VISION_RECV_SIZE;
    conf.usart_handle = _handle;
    vision_usart_instance = USARTRegister(&conf);

    // 为master process注册daemon,用于判断视觉通信是否离线
    Daemon_Init_Config_s daemon_conf = {
        .callback = VisionOfflineCallback, // 离线时调用的回调函数,会重启串口接收
        .owner_id = vision_usart_instance,
        .reload_count = 10,
    };
    vision_daemon_instance = DaemonRegister(&daemon_conf);

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
    uint16_t flag_register;
    uint8_t send_buff[VISION_SEND_SIZE];
    uint16_t tx_len;
    // TODO: code to set flag_register
    flag_register = 30<<8|0b00000001;
    // 将数据转化为seasky协议的数据包
    get_protocol_send_data(0x02, flag_register, &send->yaw, 3, send_buff, &tx_len);
    USARTSend(vision_usart_instance, send_buff, tx_len, USART_TRANSFER_DMA); // 和视觉通信使用IT,防止和接收使用的DMA冲突
    // 此处为HAL设计的缺陷,DMASTOP会停止发送和接收,导致再也无法进入接收中断.
    // 也可在发送完成中断中重新启动DMA接收,但较为复杂.因此,此处使用IT发送.
    // 若使用了daemon,则也可以使用DMA发送.
}