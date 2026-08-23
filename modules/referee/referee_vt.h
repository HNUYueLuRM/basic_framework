#ifndef RM_REFEREE_VT_H
#define RM_REFEREE_VT_H
#include "usart.h"
#include "referee_protocol.h"
#include "bsp_usart.h"
#include "FreeRTOS.h"

#include "robot_def.h"
#pragma pack(1)
typedef struct
{
	uint8_t Robot_Color;		// 机器人颜色
	uint16_t Robot_ID;			// 本机器人ID
	uint16_t Cilent_ID;			// 本机器人对应的客户端ID
	uint16_t Receiver_Robot_ID; // 机器人车间通信时接收者的ID，必须和本机器人同颜色
} referee_vt_id_t;

typedef struct 
{
    referee_vt_id_t referee_id;
	xFrameHeader FrameHeader; // 接收到的帧头信息
	uint16_t CmdID;
    custom2robot_data_t Custom2Robot;	//自定义控制器发机器人
	client2robot_data_t SelfClient2Robot;

	uint8_t init_flag;

} referee_vt_info_t;

#pragma pack()
referee_vt_info_t *RefereeVtInit(UART_HandleTypeDef *referee_vt_usart_handle);

/**
 * @brief 裁判系统图传链路需要发送一些东西，所以这个发送函数，供外部调用，并与常规链路区分
 * @note 内部包含了一个实时系统的延时函数,这是因为裁判系统接收CMD数据至高位10Hz
 *
 * @param send 发送数据首地址
 * @param tx_len 发送长度
 */
void VT_SelfClient_RefereeSend(uint8_t *send, uint16_t tx_len);
void VT_Custom_RefereeSend(uint8_t *send, uint16_t tx_len);

#endif
