#ifndef RM_REFEREE_H
#define RM_REFEREE_H

#include "usart.h"
#include "referee_def.h"
#include "bsp_usart.h"
extern uint8_t UI_Seq; 

#pragma pack(1)
typedef struct
{
	uint8_t Robot_Color;   //机器人颜色
	uint16_t Robot_ID;   //本机器人ID
	uint16_t Cilent_ID; //本机器人对应的客户端ID
	uint16_t Receiver_Robot_ID; //机器人车间通信时接收者的ID，必须和本机器人同颜色
} referee_id_t;

// 此结构体包含裁判系统接收数据以及UI绘制与机器人车间通信的相关信息
typedef struct
{	
	referee_id_t referee_id;

	xFrameHeader FrameHeader; // 接收到的帧头信息
	uint16_t CmdID;
	ext_game_state_t GameState;							   // 0x0001
	ext_game_result_t GameResult;						   // 0x0002
	ext_game_robot_HP_t GameRobotHP;					   // 0x0003
	ext_event_data_t EventData;							   // 0x0101
	ext_supply_projectile_action_t SupplyProjectileAction; // 0x0102
	ext_game_robot_state_t GameRobotState;				   // 0x0201
	ext_power_heat_data_t PowerHeatData;				   // 0x0202
	ext_game_robot_pos_t GameRobotPos;					   // 0x0203
	ext_buff_musk_t BuffMusk;							   // 0x0204
	aerial_robot_energy_t AerialRobotEnergy;			   // 0x0205
	ext_robot_hurt_t RobotHurt;							   // 0x0206
	ext_shoot_data_t ShootData;							   // 0x0207

	//自定义交互数据的接收
	Communicate_ReceiveData_t ReceiveData;

} referee_info_t;

#pragma pack()

/**
 * @brief 初始化裁判系统,返回接收数据指针
 *
 * @param referee_usart_handle
 * @return referee_info_t*
 */
referee_info_t *RefereeInit(UART_HandleTypeDef *referee_usart_handle);

/**
 * @brief 发送函数
 * @todo
 * @param send 待发送数据
 */
void RefereeSend(uint8_t *send, uint16_t tx_len);

#endif // !REFEREE_H
