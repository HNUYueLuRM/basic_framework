/**
 * @file referee.h
 * @author kidneygood (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2022-11-18
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef REFEREE_H
#define REFEREE_H

#include "bsp_usart.h"
#include "usart.h"
#include "referee_def.h"

#define FALSE 0
#define TRUE 1

#define JUDGE_DATA_ERROR 0
#define JUDGE_DATA_CORRECT 1

#define LEN_HEADER 5 // 帧头长
#define LEN_CMDID 2	 // 命令码长度
#define LEN_TAIL 2	 // 帧尾CRC16

// 起始字节,协议固定为0xA5
#define JUDGE_FRAME_HEADER (0xA5)

#pragma pack(1)

/* ID: 0x0001  Byte:  3    比赛状态数据 */
typedef struct
{
	uint8_t game_type : 4;
	uint8_t game_progress : 4;
	uint16_t stage_remain_time;
} ext_game_state_t;

/* ID: 0x0002  Byte:  1    比赛结果数据 */
typedef struct
{
	uint8_t winner;
} ext_game_result_t;

/* ID: 0x0003  Byte:  32    比赛机器人血量数据 */
typedef struct
{
	uint16_t red_1_robot_HP;
	uint16_t red_2_robot_HP;
	uint16_t red_3_robot_HP;
	uint16_t red_4_robot_HP;
	uint16_t red_5_robot_HP;
	uint16_t red_7_robot_HP;
	uint16_t red_outpost_HP;
	uint16_t red_base_HP;
	uint16_t blue_1_robot_HP;
	uint16_t blue_2_robot_HP;
	uint16_t blue_3_robot_HP;
	uint16_t blue_4_robot_HP;
	uint16_t blue_5_robot_HP;
	uint16_t blue_7_robot_HP;
	uint16_t blue_outpost_HP;
	uint16_t blue_base_HP;
} ext_game_robot_HP_t;

/* ID: 0x0101  Byte:  4    场地事件数据 */
typedef struct
{
	uint32_t event_type;
} ext_event_data_t;

/* ID: 0x0102  Byte:  3    场地补给站动作标识数据 */
typedef struct
{
	uint8_t supply_projectile_id;
	uint8_t supply_robot_id;
	uint8_t supply_projectile_step;
	uint8_t supply_projectile_num;
} ext_supply_projectile_action_t;

/* ID: 0X0201  Byte: 27    机器人状态数据 */
typedef struct
{
	uint8_t robot_id;
	uint8_t robot_level;
	uint16_t remain_HP;
	uint16_t max_HP;
	uint16_t shooter_id1_17mm_cooling_rate;
	uint16_t shooter_id1_17mm_cooling_limit;
	uint16_t shooter_id1_17mm_speed_limit;
	uint16_t shooter_id2_17mm_cooling_rate;
	uint16_t shooter_id2_17mm_cooling_limit;
	uint16_t shooter_id2_17mm_speed_limit;
	uint16_t shooter_id1_42mm_cooling_rate;
	uint16_t shooter_id1_42mm_cooling_limit;
	uint16_t shooter_id1_42mm_speed_limit;
	uint16_t chassis_power_limit;
	uint8_t mains_power_gimbal_output : 1;
	uint8_t mains_power_chassis_output : 1;
	uint8_t mains_power_shooter_output : 1;
} ext_game_robot_state_t;

/* ID: 0X0202  Byte: 14    实时功率热量数据 */
typedef struct
{
	uint16_t chassis_volt;
	uint16_t chassis_current;
	float chassis_power;		   // 瞬时功率
	uint16_t chassis_power_buffer; // 60焦耳缓冲能量
	uint16_t shooter_heat0;		   // 17mm
	uint16_t shooter_heat1;
} ext_power_heat_data_t;

/* ID: 0x0203  Byte: 16    机器人位置数据 */
typedef struct
{
	float x;
	float y;
	float z;
	float yaw;
} ext_game_robot_pos_t;

/* ID: 0x0204  Byte:  1    机器人增益数据 */
typedef struct
{
	uint8_t power_rune_buff;
} ext_buff_musk_t;

/* ID: 0x0205  Byte:  1    空中机器人能量状态数据 */
typedef struct
{
	uint8_t attack_time;
} aerial_robot_energy_t;

/* ID: 0x0206  Byte:  1    伤害状态数据 */
typedef struct
{
	uint8_t armor_id : 4;
	uint8_t hurt_type : 4;
} ext_robot_hurt_t;

/* ID: 0x0207  Byte:  7    实时射击数据 */
typedef struct
{
	uint8_t bullet_type;
	uint8_t shooter_id;
	uint8_t bullet_freq;
	float bullet_speed;
} ext_shoot_data_t;

// 裁判系统接收数据整合进一个结构体
typedef struct
{
	xFrameHeader FrameHeader; // 接收到的帧头信息
	uint16_t CmdID;
	ext_game_state_t GameState;							   // 0x0001
	ext_game_result_t GameResult;						   // 0x0002
	ext_game_robot_HP_t GameRobotHP;					   // 0x0003
	ext_event_data_t EventData;							   // 0x0101
	ext_supply_projectile_action_t SupplyProjectileAction; // 0x0102
	ext_game_robot_state_t GameRobotStat;				   // 0x0201
	ext_power_heat_data_t PowerHeatData;				   // 0x0202
	ext_game_robot_pos_t GameRobotPos;					   // 0x0203
	ext_buff_musk_t BuffMusk;							   // 0x0204
	aerial_robot_energy_t AerialRobotEnergy;			   // 0x0205
	ext_robot_hurt_t RobotHurt;							   // 0x0206
	ext_shoot_data_t ShootData;							   // 0x0207

	// ext_SendClientData_t ShowData;	 // 客户端信息
	// ext_CommunatianData_t CommuData; // 队友通信信息

} referee_info_t;

#pragma pack()

/**
 * @brief 初始化裁判系统,返回接收数据指针
 * 
 * @param referee_usart_handle 
 * @return referee_info_t* 
 */
referee_info_t* RefereeInit(UART_HandleTypeDef *referee_usart_handle);


#endif // !REFEREE_H
