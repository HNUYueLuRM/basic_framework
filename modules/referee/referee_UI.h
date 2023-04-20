#ifndef REFEREE_UI_H
#define REFEREE_UI_H

#include "stdarg.h"
#include "stdint.h"
#include "referee_protocol.h"
#include "rm_referee.h"

#pragma pack(1) // 按1字节对齐

/* 此处的定义只与UI绘制有关 */
typedef struct
{
   xFrameHeader FrameHeader;
   uint16_t CmdID;
   ext_student_interactive_header_data_t datahead;
   uint8_t Delete_Operate; // 删除操作
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
} UI_CharReFresh_t; // 打印字符串数据

#pragma pack()

void UIDelete(referee_id_t *_id, uint8_t Del_Operate, uint8_t Del_Layer);

void Line_Draw(Graph_Data_t *graph, char graphname[3], uint32_t Graph_Operate, uint32_t Graph_Layer, uint32_t Graph_Color,
               uint32_t Graph_Width, uint32_t Start_x, uint32_t Start_y, uint32_t End_x, uint32_t End_y);

void Rectangle_Draw(Graph_Data_t *graph, char graphname[3], uint32_t Graph_Operate, uint32_t Graph_Layer, uint32_t Graph_Color,
                    uint32_t Graph_Width, uint32_t Start_x, uint32_t Start_y, uint32_t End_x, uint32_t End_y);

void Circle_Draw(Graph_Data_t *graph, char graphname[3], uint32_t Graph_Operate, uint32_t Graph_Layer, uint32_t Graph_Color,
                 uint32_t Graph_Width, uint32_t Start_x, uint32_t Start_y, uint32_t Graph_Radius);

void Elliptical_Draw(Graph_Data_t *graph, char graphname[3], uint32_t Graph_Operate, uint32_t Graph_Layer, uint32_t Graph_Color,
                     uint32_t Graph_Width, uint32_t Start_x, uint32_t Start_y, uint32_t end_x, uint32_t end_y);

void Arc_Draw(Graph_Data_t *graph, char graphname[3], uint32_t Graph_Operate, uint32_t Graph_Layer, uint32_t Graph_Color,
              uint32_t Graph_StartAngle, uint32_t Graph_EndAngle, uint32_t Graph_Width, uint32_t Start_x, uint32_t Start_y,
              uint32_t end_x, uint32_t end_y);

void Float_Draw(Graph_Data_t *graph, char graphname[3], uint32_t Graph_Operate, uint32_t Graph_Layer, uint32_t Graph_Color,
                uint32_t Graph_Size, uint32_t Graph_Digit, uint32_t Graph_Width, uint32_t Start_x, uint32_t Start_y, int32_t Graph_Float);

void Integer_Draw(Graph_Data_t *graph, char graphname[3], uint32_t Graph_Operate, uint32_t Graph_Layer, uint32_t Graph_Color,
                  uint32_t Graph_Size, uint32_t Graph_Width, uint32_t Start_x, uint32_t Start_y, int32_t Graph_Integer);

void Char_Draw(String_Data_t *graph, char graphname[3], uint32_t Graph_Operate, uint32_t Graph_Layer, uint32_t Graph_Color,
               uint32_t Graph_Size, uint32_t Graph_Width, uint32_t Start_x, uint32_t Start_y, char *fmt, ...);

void UI_ReFresh(referee_id_t *_id, int cnt, ...);

void Char_ReFresh(referee_id_t *_id, String_Data_t string_Data);

#endif
