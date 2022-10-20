#ifndef BSP_RC_H
#define BSP_RC_H

#include "struct_typedef.h"
#include "main.h"

#define DEVICE_USART_CNT 3

/* application callback,which resolves specific protocol */
typedef void (*usart_module_callback)();

/* usart_instance struct,each app would have one instance */
typedef struct
{
    uint8_t *recv_buff;
    uint8_t recv_buff_size;
    UART_HandleTypeDef *usart_handle;
    usart_module_callback module_callback;
} usart_instance;

/**
 * @brief calling this func would regiter a module to usart_service
 *        this function provides completely wrap and abstract for those module who use usart as commu method
 *
 * @param id module id.attention,each id is mapped to a specific usart port(ex:remote control->usart3)
 * @param prbuff pointer to where the raw recv data stored
 * @param rbuffsize size of recv data buff
 * @param psbuff pointer to where the data to be sent stored
 * @param callback func pointer,this func would be called in USART_service.c/HAL_UARTEx_RxEventCallback()
 *        to resolve raw data using protocol provides by specific app calling Moduleregister()
 *
 */
void USARTRegister(usart_instance* _instance);

/**
 * @brief api for sending data through a specific serial port,indicated by the first parameter:id
 *
 * @param id specify which usart would be used
 * @param send_size how many bytes to send
 */
void USARTSend(usart_instance* _instance, uint8_t* send_buf,uint16_t send_size);

#endif
