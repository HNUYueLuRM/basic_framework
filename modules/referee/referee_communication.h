/**
 * @file referee_communication.h
 * @author kidneygood (you@domain.com)
 * @version 0.1
 * @date 2022-12-02
 *
 * @copyright Copyright (c) HNU YueLu EC 2022 all rights reserved
 *
 */

#ifndef REFEREE_COMMUNICATION_H
#define REFEREE_COMMUNICATION_H

#include "stdint.h"
#include "referee_def.h"
#pragma pack(1)




/*
	客户端 客户端自定义数据：cmd_id:0x0301。内容 ID:0xD180
	发送频率：上限 10Hz
	1.	客户端 客户端自定义数据：cmd_id:0x0301。内容 ID:0xD180。发送频率：上限 10Hz
	字节偏移量 	大小 	说明 				备注
	0 			2 		数据的内容 ID 		0xD180
	2 			2 		送者的 ID 			需要校验发送者机器人的 ID 正确性
	4 			2 		客户端的 ID 		只能为发送者机器人对应的客户端
	6 			4 		自定义浮点数据 1
	10 			4 		自定义浮点数据 2
	14 			4 		自定义浮点数据 3
	18 			1 		自定义 8 位数据 4

*/
typedef struct
{
	float data1;
	float data2;
	float data3;
	uint8_t masks;
} client_custom_data_t;

/*
	学生机器人间通信 cmd_id 0x0301，内容 ID:0x0200~0x02FF
	交互数据 机器人间通信：0x0301。
	发送频率：上限 10Hz
*/
typedef struct
{
	uint8_t data[10]; // 数据段,n需要小于113
} robot_interactive_data_t;

// 发送给客户端的信息
//交互数据包括一个统一的数据段头结构。数据段包含了内容 ID，发送者以及接收者的 ID 和内容数据段，
// 整个交互数据的包总共长最大为 128 个字节，减去 frame_header,cmd_id 和 frame_tail 共 9 个字节以及
// 数据段头结构的 6 个字节，故而发送的内容数据段最大为 113。
// 帧头  命令码   数据段头结构  数据段   帧尾
typedef struct
{
	xFrameHeader txFrameHeader;							   // 帧头
	uint16_t CmdID;										   // 命令码
	ext_student_interactive_header_data_t dataFrameHeader; // 数据段头结构
	client_custom_data_t clientData;					   // 数据段
	uint16_t FrameTail;									   // 帧尾
} ext_SendClientData_t;

// 机器人交互信息
typedef struct
{
	xFrameHeader txFrameHeader;							   // 帧头
	uint16_t CmdID;										   // 命令码
	ext_student_interactive_header_data_t dataFrameHeader; // 数据段头结构
	robot_interactive_data_t interactData;				   // 数据段
	uint16_t FrameTail;									   // 帧尾
} ext_CommunatianData_t;

#pragma pack()

#endif 