/**
 * @file referee_custom.c
 * @author LHXY (cjy719@hnu.edu.cn)
 * @brief 用以实现图传链路机器人发送功能，主要是自定义控制器
 * @version 0.1
 * @date 2026-03-27
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include "referee_vt.h"
#include "string.h"
#include "crc_ref.h"
#include "stdio.h"
#include "referee_custom.h"

static Robot_Custom_ReFresh_t Robot_Custom_ReFresh_data; // 机器人自定义控制器数据
int VT_Seq = 0;
/************************************************机器人推送数据*********************************/
void CustomDataRefresh(robot2custom_data_t Robot_Data)
{
    uint8_t temp_datalength =  Robot_Custom_Data_LEN; // 自定义控制器数据长度

	Robot_Custom_ReFresh_data.FrameHeader.SOF = REFEREE_SOF;
	Robot_Custom_ReFresh_data.FrameHeader.DataLength = temp_datalength;
	Robot_Custom_ReFresh_data.FrameHeader.Seq = VT_Seq;
	Robot_Custom_ReFresh_data.FrameHeader.CRC8 = Get_CRC8_Check_Sum((uint8_t *)&Robot_Custom_ReFresh_data, LEN_CRC8, 0xFF);

	Robot_Custom_ReFresh_data.CmdID = ID_robot2custom;

	Robot_Custom_ReFresh_data.Controller_Data = Robot_Data;

	Robot_Custom_ReFresh_data.frametail = Get_CRC16_Check_Sum((uint8_t *)&Robot_Custom_ReFresh_data, LEN_HEADER + LEN_CMDID + temp_datalength, 0xFFFF);

	VT_Custom_RefereeSend((uint8_t *)&Robot_Custom_ReFresh_data, LEN_HEADER + LEN_CMDID + temp_datalength + LEN_TAIL); // 发送
	VT_Seq++;                                // 包序号+1
	if (VT_Seq > 255) {
		VT_Seq = 0;
	}
}