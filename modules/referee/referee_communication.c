/**
 * @file referee_communication.h
 * @author kidneygood (you@domain.com)
 * @version 0.1
 * @date 2022-12-02
 *
 * @copyright Copyright (c) HNU YueLu EC 2022 all rights reserved
 *
 */

#include "referee_communication.h"
#include "crc.h"
#include "stdio.h"
#include "rm_referee.h"

/**
 * @brief  发送机器人间的交互数据
 * @param  referee_id_t *_id  sender为本机器人，receiver为接收方机器人，发送前设置Receiver_Robot_ID再调用该函数
 *         robot_interactive_data_t *data    数据段
 * @retval none
 * @attention
 */
void Communicate_SendData(referee_id_t *_id,robot_interactive_data_t *_data)
{
    Communicate_SendData_t SendData;
    uint8_t temp_datalength = Interactive_Data_LEN_Head + Communicate_Data_LEN; // 计算交互数据长度  6+n,n为交互数据长度

    SendData.FrameHeader.SOF = REFEREE_SOF;
    SendData.FrameHeader.DataLength = temp_datalength;
    SendData.FrameHeader.Seq = UI_Seq;
    SendData.FrameHeader.CRC8 = Get_CRC8_Check_Sum((uint8_t *)&SendData, LEN_CRC8, 0xFF);

    SendData.CmdID = ID_student_interactive;

    SendData.datahead.data_cmd_id = Communicate_Data_ID;
    SendData.datahead.sender_ID = _id->Robot_ID;
    SendData.datahead.receiver_ID = _id->Receiver_Robot_ID;

    SendData.Data = *_data;

    SendData.frametail = Get_CRC16_Check_Sum((uint8_t *)&SendData,LEN_HEADER+LEN_CMDID+temp_datalength,0xFFFF);

    RefereeSend((uint8_t *)&SendData,LEN_HEADER+LEN_CMDID+temp_datalength+LEN_TAIL); //发送
    UI_Seq++; // 包序号+1
}
