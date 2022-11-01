/**
 * @file bsp_usart.c
 * @author neozng
 * @brief  串口bsp层的实现
 * @version beta
 * @date 2022-11-01
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "bsp_usart.h"
#include "stdlib.h"

/* usart service instance, modules' info would be recoreded here using USARTRegister() */
/* usart服务实例,所有注册了usart的模块信息会被保存在这里 */
static usart_instance *instance[DEVICE_USART_CNT];

/**
 * @brief usart service will start automatically, after each module registered
 *        串口服务会在每个实例注册之后自动启用
 *
 * @param _instance instance owned by module,模块拥有的串口实例
 */
static void USARTServiceInit(usart_instance *_instance)
{
    HAL_UARTEx_ReceiveToIdle_DMA(_instance->usart_handle, _instance->recv_buff, _instance->recv_buff_size);
    // 关闭dma half transfer中断防止两次进入HAL_UARTEx_RxEventCallback()
    __HAL_DMA_DISABLE_IT(_instance->usart_handle->hdmarx, DMA_IT_HT);
}

void USARTRegister(usart_instance *_instance)
{
    static instance_idx;
    USARTServiceInit(_instance);
    instance[instance_idx++] = _instance;
}

/* @todo 当前仅进行了形式上的封装,后续要进一步考虑是否将module的行为与bsp完全分离 */
void USARTSend(usart_instance *_instance, uint8_t *send_buf, uint16_t send_size)
{
    HAL_UART_Transmit_DMA(_instance->usart_handle, send_buf, send_size);
}

/**
 * @brief 每次dma/idle中断发生时，都会调用此函数.对于每个uart实例会调用对应的回调进行进一步的处理
 *        例如:视觉协议解析/遥控器解析/裁判系统解析
 *
 * @note  because DMA half transfer iterrupt(DMA_IT_HT) would call this callback function too, so we just
 *        disable it when transfer complete using macro: __HAL_DMA_DISABLE_IT(huart->hdmarx,DMA_IT_HT)
 *
 * @param huart uart handle indicate which uart is being handled 发生中断的串口
 * @param Size not used temporarily,此次接收到的总数居量,暂时没用
 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    for (uint8_t i = 0; i < 3; i++)
    {
        if (huart == instance[i]->usart_handle)
        {
            instance[i]->module_callback();
            HAL_UARTEx_ReceiveToIdle_DMA(instance[i]->usart_handle, instance[i]->recv_buff, instance[i]->recv_buff_size);
            __HAL_DMA_DISABLE_IT(instance[i]->usart_handle->hdmarx, DMA_IT_HT);
            break;
        }
    }
}

/**
 * @brief when error occurs in the process of send/receive,this function will be called
 *        then just simply restart send/receive.
 *
 * @note  most frequent error ex: parity/overrrun/frame error
 *
 * @param huart uart handle type, indicate where error comes from
 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    for (uint8_t i = 0; i < 3; i++)
    {
        if (huart == instance[i]->usart_handle)
        {
            HAL_UARTEx_ReceiveToIdle_DMA(instance[i]->usart_handle, instance[i]->recv_buff, instance[i]->recv_buff_size);
            __HAL_DMA_DISABLE_IT(instance[i]->usart_handle->hdmarx, DMA_IT_HT);
            break;
        }
    }
}