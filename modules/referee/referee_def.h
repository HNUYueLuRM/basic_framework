/**
 * @file referee_def.h
 * @author kidneygood (you@domain.com)
 * @version 0.1
 * @date 2022-12-02
 *
 * @copyright Copyright (c) HNU YueLu EC 2022 all rights reserved
 *
 */

#ifndef REFEREE_DEF_H
#define REFEREE_DEF_H

#include "stdint.h"

#pragma pack(1)

/* 通信协议格式偏移，枚举类型,代替#define声明 */
typedef enum
{
	FRAME_HEADER = 0,
	CMD_ID = 5,
	DATA = 7,
} JudgeFrameOffset;

/* 帧头偏移 */
typedef enum
{
	SOF = 0,		 // 起始位
	DATA_LENGTH = 1, // 帧内数据长度,根据这个来获取数据长度
	SEQ = 3,		 // 包序号
	CRC8 = 4		 // CRC8
} FrameHeaderOffset;

/* 帧头定义 */
typedef struct
{
	uint8_t SOF;
	uint16_t DataLength;
	uint8_t Seq;
	uint8_t CRC8;
} xFrameHeader;

/* 命令码ID,用来判断接收的是什么数据 */
typedef enum
{
	ID_game_state = 0x0001,				   // 比赛状态数据
	ID_game_result = 0x0002,			   // 比赛结果数据
	ID_game_robot_survivors = 0x0003,	   // 比赛机器人血量数据
	ID_event_data = 0x0101,				   // 场地事件数据
	ID_supply_projectile_action = 0x0102,  // 场地补给站动作标识数据
	ID_supply_projectile_booking = 0x0103, // 场地补给站预约子弹数据
	ID_game_robot_state = 0x0201,		   // 机器人状态数据
	ID_power_heat_data = 0x0202,		   // 实时功率热量数据
	ID_game_robot_pos = 0x0203,			   // 机器人位置数据
	ID_buff_musk = 0x0204,				   // 机器人增益数据
	ID_aerial_robot_energy = 0x0205,	   // 空中机器人能量状态数据
	ID_robot_hurt = 0x0206,				   // 伤害状态数据
	ID_shoot_data = 0x0207,				   // 实时射击数据

} CmdID;

/* 命令码数据段长,根据官方协议来定义长度 */
typedef enum
{
	LEN_game_state = 3,				  // 0x0001
	LEN_game_result = 1,			  // 0x0002
	LEN_game_robot_HP = 2,			  // 0x0003
	LEN_event_data = 4,				  // 0x0101
	LEN_supply_projectile_action = 4, // 0x0102
	LEN_game_robot_state = 27,	 // 0x0201
	LEN_power_heat_data = 14,	 // 0x0202
	LEN_game_robot_pos = 16,	 // 0x0203
	LEN_buff_musk = 1,			 // 0x0204
	LEN_aerial_robot_energy = 1, // 0x0205
	LEN_robot_hurt = 1,			 // 0x0206
	LEN_shoot_data = 7,			 // 0x0207
} JudgeDataLength;

#pragma pack()

#endif 

