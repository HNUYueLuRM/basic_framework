#ifndef REFEREE_UI_H
#define REFEREE_UI_H

#include "stdarg.h"
#include "usart.h"
#include "stdint.h"
#include "referee_def.h"
#include "referee.h"

#pragma pack(1) //按1字节对齐

/* 此处的定义只与UI绘制有关 */
typedef struct
{
	xFrameHeader FrameHeader; 
	uint16_t CmdID;
   ext_student_interactive_header_data_t datahead;
   uint8_t Delete_Operate;         //删除操作
   uint8_t Layer; 
   uint16_t frametail;
} UI_delete_t;

typedef struct
{
	xFrameHeader FrameHeader; 
	uint16_t CmdID;
   ext_student_interactive_header_data_t datahead;
   uint16_t frametail;
} UI_GraphReFresh_t;

typedef struct
{
  	xFrameHeader FrameHeader; 
	uint16_t CmdID;
   ext_student_interactive_header_data_t datahead;
   String_Data_t String_Data;
   uint16_t frametail; 
} UI_CharReFresh_t;                  //打印字符串数据

#pragma pack()

void Interactive_init(referee_info_t *_referee_info);

#endif
