#ifndef BSP_CAN_H
#define BSP_CAN_H

#include <stdint-gcc.h>
#include "can.h"

#define MX_REGISTER_DEVICE_CNT 12  // maximum number of device can be registered to CAN service
                                   // this number depends on the load of CAN bus.
#define MX_CAN_FILTER_CNT (2 * 14) // temporarily useless
#define DEVICE_CAN_CNT 2           // CAN1,CAN2

/* can instance typedef, every module registered to CAN should have this variable */
#pragma pack(1)
typedef struct _
{
    CAN_HandleTypeDef *can_handle; // can句柄
    CAN_TxHeaderTypeDef txconf;    // CAN报文发送配置
    uint32_t tx_id;                // 发送id
    uint32_t tx_mailbox;           // CAN消息填入的邮箱号
    uint8_t tx_buff[8];            // 发送缓存,最大为8
    uint8_t rx_buff[8];            // 接收缓存
    uint32_t rx_id;                // 接收id
    uint8_t rx_len;                // 接收长度,可能为0-8
    // 接收的回调函数,用于解析接收到的数据
    void (*can_module_callback)(struct _ *); // callback needs an instance to tell among registered ones
} CANInstance;
#pragma pack()

/* this structure is used for initialization */
typedef struct
{
    CAN_HandleTypeDef *can_handle;
    uint32_t tx_id;
    uint32_t rx_id;
    void (*can_module_callback)(CANInstance *);
} CAN_Init_Config_s;

/**
 * @brief 修改CAN发送报文的数据帧长度;注意最大长度为8,在没有进行修改的时候,默认长度为8
 *
 * @param _instance 要修改长度的can实例
 * @param length    设定长度
 */
void CANSetDLC(CANInstance *_instance, uint8_t length);

/**
 * @brief transmit mesg through CAN device,通过can实例发送消息
 *        发送前需要向CAN实例的tx_buff写入发送数据
 *
 * @param _instance* can instance owned by module
 */
void CANTransmit(CANInstance *_instance);

/**
 * @brief Register a module to CAN service,remember to call this before using a CAN device
 *        注册(初始化)一个can实例,需要传入初始化配置的指针.
 * @param config init config
 * @return CANInstance* can instance owned by module
 */
CANInstance *CANRegister(CAN_Init_Config_s *config);

#endif
