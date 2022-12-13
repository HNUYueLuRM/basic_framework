#ifndef BSP_RC_H
#define BSP_RC_H

#include <stdint.h>
#include "main.h"

#define DEVICE_USART_CNT 3     // C板至多分配3个串口
#define USART_RXBUFF_LIMIT 256 // if your protocol needs bigger buff, modify here

/* application callback,which resolves specific protocol,解析协议的回调函数 */
typedef void (*usart_module_callback)();

/* USARTInstance struct,each app would have one instance */
typedef struct
{
    // 更新:弃用malloc方案,使用了固定大小的数组方便debug时查看
    uint8_t recv_buff[USART_RXBUFF_LIMIT]; // 预先定义的最大buff大小,如果太小请修改USART_RXBUFF_LIMIT
    uint8_t recv_buff_size;                // 模块接收一包数据的大小
    UART_HandleTypeDef *usart_handle;      // 实例对应的usart_handle
    usart_module_callback module_callback; // 解析收到的数据的回调函数
} USARTInstance;

/* usart 初始化配置结构体 */
typedef struct
{
    uint8_t recv_buff_size;                // 模块接收一包数据的大小
    UART_HandleTypeDef *usart_handle;      // 实例对应的usart_handle
    usart_module_callback module_callback; // 解析收到的数据的回调函数
} USART_Init_Config_s;

/**
 * @brief 注册一个串口实例.
 *
 * @param init_config 传入串口初始化结构体
 */
USARTInstance *USARTRegister(USART_Init_Config_s *init_config);

/**
 * @todo 是否需要进一步封装发送buff和size,并创建一个串口任务以一定频率自动发送?
 *       若采用此方法,则串口实例的拥有者仅需要在自己的任务中设置发送值,不需要关心发送buffer大小以及何时发送.
 *
 * @brief api for sending data through a specific serial port,indicated by the first parameter:id
 *        通过调用该函数可以发送一帧数据,需要传入一个usart实例,发送buff以及这一帧的长度
 *
 * @param id specify which usart would be used
 * @param send_size how many bytes to send
 */
void USARTSend(USARTInstance *_instance, uint8_t *send_buf, uint16_t send_size);

#endif
