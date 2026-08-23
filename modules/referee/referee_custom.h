#ifndef REFEREE_CUSTOM_H
#define REFEREE_CUSTOM_H
#include "usart.h"
#include "referee_protocol.h"
#include "bsp_usart.h"
#include "FreeRTOS.h"

#include "robot_def.h"
#pragma pack(1)

typedef struct
{
   xFrameHeader FrameHeader;
   uint16_t CmdID;
   robot2custom_data_t  Controller_Data;
   uint16_t frametail;
} Robot_Custom_ReFresh_t; // 自定义控制器数据

#pragma pack()

void CustomDataRefresh(robot2custom_data_t Robot_Data);

#endif
