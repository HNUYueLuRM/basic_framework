#include "HC05.h"
#include "bsp_usart.h"

#define HC05_BUFFERSIZE  HC05_DATASIZE+2  // HC05发送和接收数据buffer大小，不得大于256

#define FRAME_HEAD  0XAA    // 帧头
#define FRAME_END   0X55    // 帧尾

static HC05 hc05_msg;   // HC05通信数据
static USARTInstance *hc05_usart_instance;    // HC05串口通信实例

static uint8_t hc05_init_flag = 0;  // HC05初始化标志位

// *hc05_usart_instance串口回调函数
static void HC05RxCallback()
{
    uint8_t *rxbuff;
    rxbuff = hc05_usart_instance->recv_buff;

    // 帧头帧尾判断
    if(rxbuff[0] == FRAME_HEAD && rxbuff[HC05_BUFFERSIZE - 1] == FRAME_END)
    {
        for(int i = 0; i < HC05_DATASIZE; i++)
            hc05_msg.recv_data[i] = (uint8_t)rxbuff[i+1];
    }
    
    return;
}

// HC05串口接收初始化
HC05 *HC05Init(UART_HandleTypeDef *hc05_usart_handle)
{
    USART_Init_Config_s conf;
    conf.module_callback = HC05RxCallback;
    conf.usart_handle = hc05_usart_handle;
    conf.recv_buff_size = HC05_BUFFERSIZE;
    hc05_usart_instance = USARTRegister(&conf);

    hc05_init_flag = 1;
    return (HC05*)&hc05_msg;
}

// HC05串口发送函数，一次最多发送HC05_DATASIZE个数据
void HC05_SendData(uint8_t *data, uint8_t data_num)
{

    // 发送数据中加入帧头和帧尾
    hc05_msg.send_data[0] = FRAME_HEAD;

    for (int i = 0; i < data_num; i++)
        hc05_msg.send_data[i+1] = data[i];
    
    hc05_msg.send_data[HC05_BUFFERSIZE - 1] = FRAME_END;

    // 发送数据
    USARTSend(hc05_usart_instance, hc05_msg.send_data, data_num+2, USART_TRANSFER_IT); 

}