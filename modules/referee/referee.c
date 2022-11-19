#include "referee.h"
#include "string.h"
#include "crc.h"
#include "bsp_usart.h"
#include "dma.h"

// 参考深圳大学  Infantry_X-master
#define RE_RX_BUFFER_SIZE 200

// static usart_instance referee_usart_instance;
usart_instance referee_usart_instance;

/**************裁判系统数据******************/
referee_info_t referee_info;
uint8_t Judge_Self_ID;		  // 当前机器人的ID
uint16_t Judge_SelfClient_ID; // 发送者机器人对应的客户端ID

static void ReceiveCallback()
{
	JudgeReadData(referee_usart_instance.recv_buff);
}

void RefereeInit(UART_HandleTypeDef *referee_usart_handle)
{
	referee_usart_instance.module_callback = ReceiveCallback;
	referee_usart_instance.usart_handle = referee_usart_handle;
	referee_usart_instance.recv_buff_size = RE_RX_BUFFER_SIZE;
	USARTRegister(&referee_usart_instance);
}

/**
 * @brief  读取裁判数据,中断中读取保证速度
 * @param  缓存数据
 * @retval 是否对正误判断做处理
 * @attention  在此判断帧头和CRC校验,无误再写入数据，不重复判断帧头
 */
void JudgeReadData(uint8_t *ReadFromUsart)
{
	uint16_t judge_length; // 统计一帧数据长度
	//	referee_info.CmdID = 0; //数据命令码解析
	// 空数据包，则不作任何处理
	if (ReadFromUsart == NULL)
		return;

	// 写入帧头数据(5-byte),用于判断是否开始存储裁判数据
	memcpy(&referee_info.FrameHeader, ReadFromUsart, LEN_HEADER);

	// 判断帧头数据(0)是否为0xA5
	if (ReadFromUsart[SOF] == JUDGE_FRAME_HEADER)
	{
		// 帧头CRC8校验
		if (Verify_CRC8_Check_Sum(ReadFromUsart, LEN_HEADER) == TRUE)
		{
			// 统计一帧数据长度(byte),用于CR16校验
			judge_length = ReadFromUsart[DATA_LENGTH] + LEN_HEADER + LEN_CMDID + LEN_TAIL;
			// 帧尾CRC16校验
			if (Verify_CRC16_Check_Sum(ReadFromUsart, judge_length) == TRUE)
			{
				// 2个8位拼成16位int
				referee_info.CmdID = (ReadFromUsart[6] << 8 | ReadFromUsart[5]);
				// 解析数据命令码,将数据拷贝到相应结构体中(注意拷贝数据的长度)
				// 第8个字节开始才是数据 data=7
				switch (referee_info.CmdID)
				{
				case ID_game_state: // 0x0001
					memcpy(&referee_info.GameState, (ReadFromUsart + DATA), LEN_game_state);
					break;

				case ID_game_result: // 0x0002
					memcpy(&referee_info.GameResult, (ReadFromUsart + DATA), LEN_game_result);
					break;

				case ID_game_robot_survivors: // 0x0003
					memcpy(&referee_info.GameRobotHP, (ReadFromUsart + DATA), LEN_game_robot_HP);
					break;

				case ID_event_data: // 0x0101
					memcpy(&referee_info.EventData, (ReadFromUsart + DATA), LEN_event_data);
					break;

				case ID_supply_projectile_action: // 0x0102
					memcpy(&referee_info.SupplyProjectileAction, (ReadFromUsart + DATA), LEN_supply_projectile_action);
					break;

				case ID_game_robot_state: // 0x0201
					memcpy(&referee_info.GameRobotStat, (ReadFromUsart + DATA), LEN_game_robot_state);
					break;
				case ID_power_heat_data: // 0x0202
					memcpy(&referee_info.PowerHeatData, (ReadFromUsart + DATA), LEN_power_heat_data);
					break;

				case ID_game_robot_pos: // 0x0203
					memcpy(&referee_info.GameRobotPos, (ReadFromUsart + DATA), LEN_game_robot_pos);
					break;

				case ID_buff_musk: // 0x0204
					memcpy(&referee_info.BuffMusk, (ReadFromUsart + DATA), LEN_buff_musk);
					break;

				case ID_aerial_robot_energy: // 0x0205
					memcpy(&referee_info.AerialRobotEnergy, (ReadFromUsart + DATA), LEN_aerial_robot_energy);
					break;

				case ID_robot_hurt: // 0x0206
					memcpy(&referee_info.RobotHurt, (ReadFromUsart + DATA), LEN_robot_hurt);
					break;

				case ID_shoot_data: // 0x0207
					memcpy(&referee_info.ShootData, (ReadFromUsart + DATA), LEN_shoot_data);
					// JUDGE_ShootNumCount();//发弹量统计
					break;
				}
			}
		}
		// 首地址加帧长度,指向CRC16下一字节,用来判断是否为0xA5,用来判断一个数据包是否有多帧数据
		if (*(ReadFromUsart + sizeof(xFrameHeader) + LEN_CMDID + referee_info.FrameHeader.DataLength + LEN_TAIL) == 0xA5)
		{
			// 如果一个数据包出现了多帧数据,则再次读取
			JudgeReadData(ReadFromUsart + sizeof(xFrameHeader) + LEN_CMDID + referee_info.FrameHeader.DataLength + LEN_TAIL);
		}
	}
}

/**
 * @brief  读取瞬时功率
 * @param  void
 * @retval 实时功率值
 * @attention
 */
float JudgeGetChassisPower(void)
{
	return (referee_info.PowerHeatData.chassis_power);
}
