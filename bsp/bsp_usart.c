#include "bsp_usart.h"
#include "stdlib.h"

/* usart service instance,modules' info would be recoreded here using ModuleRegister() */
static usart_instance* instance[DEVICE_USART_CNT];

/**
 * @brief usart service will start automatically, after each module registered
 * 
 * @param _instance instance owned by module
 */
static void USARTServiceInit(usart_instance* _instance)
{
    HAL_UARTEx_ReceiveToIdle_DMA(_instance->usart_handle, _instance->recv_buff, _instance->recv_buff_size);
    __HAL_DMA_DISABLE_IT(_instance->usart_handle->hdmarx, DMA_IT_HT);
}

void USARTRegister(usart_instance* _instance)
{
    static instance_idx;
    _instance->recv_buff=(uint8_t*)malloc(_instance->recv_buff_size*sizeof(uint8_t));
    USARTServiceInit(_instance);
    instance[instance_idx++]=_instance;
}

void USARTSend(usart_instance* _instance,uint8_t* send_buf, uint16_t send_size)
{
    HAL_UART_Transmit_DMA(_instance->usart_handle, send_buf,send_size);
}

/**
 * @brief everytiem when dma/idle happens,this function will be called
 *        here, for each uart, specific callback is refered for further process
 *        etc:visual protocol resolve/remote control resolve/referee protocol resolve
 *
 * @note  because DMA half transfer iterrupt(DMA_IT_HT) would call this function too, so we just
 *        disable it when transfer complete using macro: __HAL_DMA_DISABLE_IT(huart->hdmarx,DMA_IT_HT)
 *
 * @param huart uart handle indicate which uart is being handled
 * @param Size not used temporarily
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
 * @param huart uart handle type,indicate where error comes from
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