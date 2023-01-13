#include "referee_UI.h"
#include "string.h"
#include "crc.h"
#include "bsp_usart.h"
#include "dma.h"
#include "stdio.h"
#include "referee.h"
/* syhtodo 根据自身id判断客户端id 
涉及到的数字是否可以枚举定义？？？
*/

uint16_t Robot_ID = UI_Data_RobotID_BHero;
uint16_t Cilent_ID = UI_Data_CilentID_BHero;

uint8_t UI_Seq;                      //包序号

/********************************************删除操作*************************************
**参数：Del_Operate  对应头文件删除操作
        Del_Layer    要删除的层 取值0-9
*****************************************************************************************/
void UI_Delete(uint8_t Del_Operate,uint8_t Del_Layer)
{
   UI_delete_t UI_delete_data;

   UI_delete_data.FrameHeader.SOF = REFEREE_SOF;
   UI_delete_data.FrameHeader.DataLength = UI_Data_LEN_Del;
   UI_delete_data.FrameHeader.Seq = UI_Seq;
   UI_delete_data.FrameHeader.CRC8 = Get_CRC8_Check_Sum((uint8_t *)&UI_delete_data,4,0xFF);

   UI_delete_data.CmdID = ID_student_interactive;

   UI_delete_data.datahead.data_cmd_id = UI_Data_ID_Del;
   UI_delete_data.datahead.receiver_ID = Cilent_ID;
   UI_delete_data.datahead.sender_ID = Robot_ID;

   UI_delete_data.Delete_Operate = Del_Operate;         //删除操作
   UI_delete_data.Layer = Del_Layer; 

   UI_delete_data.frametail = Get_CRC16_Check_Sum((uint8_t *)&UI_delete_data,LEN_HEADER+LEN_CMDID+UI_Data_LEN_Del,0xFFFF);
   /* syhtodo为什么填入0xFFFF */

   RefereeSend((uint8_t *)&UI_delete_data,LEN_HEADER+LEN_CMDID+UI_Data_LEN_Del+LEN_TAIL); //发送 
 
   UI_Seq++;                                                         //包序号+1
}
/************************************************绘制直线*************************************************
**参数：*graph Graph_Data类型变量指针，用于存放图形数据
        graphname[3]   图片名称，用于标识更改
        Graph_Operate   图片操作，见头文件
        Graph_Layer    图层0-9
        Graph_Color    图形颜色
        Graph_Width    图形线宽
        Start_x、Start_y  起点xy坐标
        End_x、End_y   终点xy坐标
**********************************************************************************************************/
        
void Line_Draw(Graph_Data_t *graph,char graphname[3],uint32_t Graph_Operate,uint32_t Graph_Layer,uint32_t Graph_Color,
                  uint32_t Graph_Width,uint32_t Start_x,uint32_t Start_y,uint32_t End_x,uint32_t End_y)
{
   int i;
   //??????syhtodo
   for(i=0;i<3&&graphname[i]!='\0';i++)
   {
      graph->graphic_name[2-i]=graphname[i];
   }

   graph->operate_tpye = Graph_Operate;
   graph->graphic_tpye = UI_Graph_Line;
   graph->layer = Graph_Layer;
   graph->color = Graph_Color;

   graph->width = Graph_Width;
   graph->start_x = Start_x;
   graph->start_y = Start_y;
   graph->end_x = End_x;
   graph->end_y = End_y;
}

/************************************************绘制矩形*************************************************
**参数：*graph Graph_Data类型变量指针，用于存放图形数据
        graphname[3]   图片名称，用于标识更改
        Graph_Operate   图片操作，见头文件
        Graph_Layer    图层0-9
        Graph_Color    图形颜色
        Graph_Width    图形线宽
        Start_x、Start_y    起点xy坐标
        End_x、End_y        对角顶点xy坐标
**********************************************************************************************************/      
void Rectangle_Draw(Graph_Data_t *graph,char graphname[3],uint32_t Graph_Operate,uint32_t Graph_Layer,uint32_t Graph_Color,
                  uint32_t Graph_Width,uint32_t Start_x,uint32_t Start_y,uint32_t End_x,uint32_t End_y)
{
   int i;
   for(i=0;i<3&&graphname[i]!='\0';i++)
   {
      graph->graphic_name[2-i]=graphname[i];
   }

   graph->graphic_tpye = UI_Graph_Rectangle;
   graph->operate_tpye = Graph_Operate;
   graph->layer = Graph_Layer;
   graph->color = Graph_Color;

   graph->width = Graph_Width;
   graph->start_x = Start_x;
   graph->start_y = Start_y;
   graph->end_x = End_x;
   graph->end_y = End_y;
}

/************************************************绘制整圆*************************************************
**参数：*graph Graph_Data类型变量指针，用于存放图形数据
        graphname[3]   图片名称，用于标识更改
        Graph_Operate   图片操作，见头文件
        Graph_Layer    图层0-9
        Graph_Color    图形颜色
        Graph_Width    图形线宽
        Start_x、Start_y    圆心xy坐标
        Graph_Radius  圆形半径
**********************************************************************************************************/
        
void Circle_Draw(Graph_Data_t *graph,char graphname[3],uint32_t Graph_Operate,uint32_t Graph_Layer,uint32_t Graph_Color,
                  uint32_t Graph_Width,uint32_t Start_x,uint32_t Start_y,uint32_t Graph_Radius)
{
   int i;
   for(i=0;i<3&&graphname[i]!='\0';i++)
   {
      graph->graphic_name[2-i]=graphname[i];
   }

   graph->graphic_tpye = UI_Graph_Circle;
   graph->operate_tpye = Graph_Operate;
   graph->layer = Graph_Layer;
   graph->color = Graph_Color;

   graph->width = Graph_Width;
   graph->start_x = Start_x;
   graph->start_y = Start_y;
   graph->radius = Graph_Radius;
}
/************************************************绘制椭圆*************************************************
**参数：*graph Graph_Data类型变量指针，用于存放图形数据
        graphname[3]   图片名称，用于标识更改
        Graph_Operate   图片操作，见头文件
        Graph_Layer    图层0-9
        Graph_Color    图形颜色
        Graph_Width    图形线宽
        Start_x、Start_y    圆心xy坐标
        End_x、End_y        xy半轴长度
**********************************************************************************************************/
void Elliptical_Draw(Graph_Data_t *graph,char graphname[3],uint32_t Graph_Operate,uint32_t Graph_Layer,uint32_t Graph_Color,
                  uint32_t Graph_Width,uint32_t Start_x,uint32_t Start_y,uint32_t end_x,uint32_t end_y)
{
   int i;
   for(i=0;i<3&&graphname[i]!='\0';i++)
   {
      graph->graphic_name[2-i]=graphname[i];
   }

   graph->graphic_tpye = UI_Graph_Ellipse;
   graph->operate_tpye = Graph_Operate;
   graph->layer = Graph_Layer;
   graph->color = Graph_Color;
   graph->width = Graph_Width;

   graph->start_x = Start_x;
   graph->start_y = Start_y;
   graph->end_x = end_x;
   graph->end_y = end_y;
}


/************************************************绘制圆弧*************************************************
**参数：*graph Graph_Data类型变量指针，用于存放图形数据
        graphname[3]   图片名称，用于标识更改
        Graph_Operate   图片操作，见头文件
        Graph_Layer    图层0-9
        Graph_Color    图形颜色
        Graph_StartAngle,Graph_EndAngle    起始终止角度
		  Graph_Width    图形线宽
        Start_y,Start_y    圆心xy坐标
        x_Length,y_Length   xy半轴长度
**********************************************************************************************************/
        
void Arc_Draw(Graph_Data_t *graph,char graphname[3],uint32_t Graph_Operate,uint32_t Graph_Layer,uint32_t Graph_Color,
                  uint32_t Graph_StartAngle,uint32_t Graph_EndAngle,uint32_t Graph_Width,uint32_t Start_x,uint32_t Start_y, 
                  uint32_t end_x,uint32_t end_y)
{
   int i;
   for(i=0;i<3&&graphname[i]!='\0';i++)
   {
      graph->graphic_name[2-i]=graphname[i];
   }

   graph->graphic_tpye = UI_Graph_Arc;
   graph->operate_tpye = Graph_Operate;
   graph->layer = Graph_Layer;
   graph->color = Graph_Color;

   graph->start_angle = Graph_StartAngle;
   graph->end_angle = Graph_EndAngle;
   graph->width = Graph_Width;
   graph->start_x = Start_x;
   graph->start_y = Start_y;
   graph->end_x = end_x;
   graph->end_y = end_y;
}

/************************************************绘制浮点型数据*************************************************
**参数：*graph Graph_Data类型变量指针，用于存放图形数据
        graphname[3]   图片名称，用于标识更改
        Graph_Operate   图片操作，见头文件
        Graph_Layer    图层0-9
        Graph_Color    图形颜色
        Graph_Size     字号
        Graph_Digit    小数位数
		  Graph_Width    图形线宽
        Start_x、Start_y    开始坐标
        radius=a&0x3FF;   a为浮点数乘以1000后的32位整型数
        end_x=(a>>10)&0x7FF;
        end_y=(a>>21)&0x7FF;
**********************************************************************************************************/
        
void Float_Draw(Graph_Data_t *graph,char graphname[3],uint32_t Graph_Operate,uint32_t Graph_Layer,uint32_t Graph_Color,
                  uint32_t Graph_Size,uint32_t Graph_Digit,uint32_t Graph_Width,uint32_t Start_x,uint32_t Start_y,int32_t Graph_Float)
{
   int i;
   for(i=0;i<3&&graphname[i]!='\0';i++)
   {
      graph->graphic_name[2-i]=graphname[i];
   }
   graph->graphic_tpye = UI_Graph_Float;
   graph->operate_tpye = Graph_Operate;
   graph->layer = Graph_Layer;
   graph->color = Graph_Color;

   graph->width = Graph_Width;
   graph->start_x = Start_x;
   graph->start_y = Start_y;
   graph->start_angle = Graph_Size;
   graph->end_angle = Graph_Digit;

   graph->radius=Graph_Float&0x3FF;   
   graph->end_x=(Graph_Float>>10)&0x7FF;
   graph->end_y=(Graph_Float>>21)&0x7FF;
}

/************************************************绘制整型数据*************************************************
**参数：*graph Graph_Data类型变量指针，用于存放图形数据
        graphname[3]   图片名称，用于标识更改
        Graph_Operate   图片操作，见头文件
        Graph_Layer    图层0-9
        Graph_Color    图形颜色
        Graph_Size     字号
		  Graph_Width    图形线宽
        Start_x、Start_y    开始坐标
        radius=a&0x3FF;   a为32位整型数
        end_x=(a>>10)&0x7FF;
        end_y=(a>>21)&0x7FF;
**********************************************************************************************************/
void Integer_Draw(Graph_Data_t *graph,char graphname[3],uint32_t Graph_Operate,uint32_t Graph_Layer,uint32_t Graph_Color,
                  uint32_t Graph_Size,uint32_t Graph_Width,uint32_t Start_x,uint32_t Start_y,int32_t Graph_Integer)
{
   int i;
   for(i=0;i<3&&graphname[i]!='\0';i++)
   {
      graph->graphic_name[2-i]=graphname[i];
   }
   graph->graphic_tpye = UI_Graph_Int;
   graph->operate_tpye = Graph_Operate;
   graph->layer = Graph_Layer;
   graph->color = Graph_Color;

   graph->start_angle = Graph_Size;
   graph->width = Graph_Width;
   graph->start_x = Start_x;
   graph->start_y = Start_y;

   graph->radius=Graph_Integer&0x3FF;   
   graph->end_x=(Graph_Integer>>10)&0x7FF;
   graph->end_y=(Graph_Integer>>21)&0x7FF;
}


/************************************************绘制字符型数据*************************************************
**参数：*graph Graph_Data类型变量指针，用于存放图形数据
        graphname[3]   图片名称，用于标识更改
        Graph_Operate   图片操作，见头文件
        Graph_Layer    图层0-9
        Graph_Color    图形颜色
        Graph_Size     字号
        Graph_Width    图形线宽
        Start_x、Start_y    开始坐标
**********************************************************************************************************/        
void Char_Draw(String_Data_t *graph,char graphname[3],uint32_t Graph_Operate,uint32_t Graph_Layer,uint32_t Graph_Color,
                  uint32_t Graph_Size,uint32_t Graph_Width,uint32_t Start_x,uint32_t Start_y)
{  
   memset(graph->Graph_Control.graphic_name, 0, 3);//syhtodo 是否需要手动清零
   int i;
   for(i=0;i<3&&graphname[i]!='\0';i++)
   {
      graph->Graph_Control.graphic_name[2-i]=graphname[i];
   }
   
   graph->Graph_Control.graphic_tpye = UI_Graph_Char;
   graph->Graph_Control.operate_tpye = Graph_Operate;
   graph->Graph_Control.layer = Graph_Layer;
   graph->Graph_Control.color = Graph_Color;

   graph->Graph_Control.width = Graph_Width;
   graph->Graph_Control.start_x = Start_x;
   graph->Graph_Control.start_y = Start_y;
   graph->Graph_Control.start_angle = Graph_Size;

   //syhtodo无关的赋值为0
   graph->Graph_Control.radius=0;
   graph->Graph_Control.end_x=0;
   graph->Graph_Control.end_y=0;
}

/************************************************绘制字符型数据*************************************************
**参数：*graph Graph_Data类型变量指针，用于存放图形数据
        fmt需要显示的字符串
syhtodo 尚未理解该函数的写法
**********************************************************************************************************/
void Char_Write(String_Data_t *graph,char* fmt, ...)
{
	uint16_t i = 0;
	va_list ap;
	va_start(ap,fmt);
	vsprintf((char*)graph->show_Data, fmt, ap);
	va_end(ap);
	i = strlen((const char*)graph->show_Data);
	graph->Graph_Control.end_angle = i;
}


/* UI推送函数（使更改生效）
   参数： cnt   图形个数
            ...   图形变量参数
   Tips：：该函数只能推送1，2，5，7个图形，其他数目协议未涉及
 */
int UI_ReFresh(int cnt,...)
{
   int i;
   UI_GraphReFresh_t UI_GraphReFresh_data;
   Graph_Data_t graphData;

   va_list ap;//创建一个 va_list 类型变量
   va_start(ap,cnt);//初始化 va_list 变量为一个参数列表

   UI_GraphReFresh_data.FrameHeader.SOF = REFEREE_SOF;
   UI_GraphReFresh_data.FrameHeader.DataLength = 6+cnt*15;//syhtodo 换成枚举
   UI_GraphReFresh_data.FrameHeader.Seq = UI_Seq;
   UI_GraphReFresh_data.FrameHeader.CRC8 = Get_CRC8_Check_Sum((uint8_t *)&UI_GraphReFresh_data,4,0xFF);

   UI_GraphReFresh_data.CmdID = ID_student_interactive;

   switch(cnt)  //syhtodo可以直接计算式解决
   {
      case 1:
         UI_GraphReFresh_data.datahead.data_cmd_id=UI_Data_ID_Draw1;
         break;
      case 2:
         UI_GraphReFresh_data.datahead.data_cmd_id=UI_Data_ID_Draw2;
         break;
      case 5:
         UI_GraphReFresh_data.datahead.data_cmd_id=UI_Data_ID_Draw5;
         break;
      case 7:
         UI_GraphReFresh_data.datahead.data_cmd_id=UI_Data_ID_Draw7;
         break;
      default:
         return (-1);
   }
   UI_GraphReFresh_data.datahead.receiver_ID = Cilent_ID;
   UI_GraphReFresh_data.datahead.sender_ID = Robot_ID;

   UI_GraphReFresh_data.frametail=Get_CRC16_Check_Sum((uint8_t *)&UI_GraphReFresh_data,LEN_HEADER+LEN_CMDID+6,0xFFFF);
   RefereeSend((uint8_t *)&UI_GraphReFresh_data,LEN_HEADER+LEN_CMDID+6);

   unsigned char *framepoint;                      //读写指针

   for(i=0;i<cnt;i++) //发送图片帧
   {
      graphData=va_arg(ap,Graph_Data_t);//访问参数列表中的每个项,第二个参数是你要返回的参数的类型,在取值时需要将其强制转化为指定类型的变量
      RefereeSend((uint8_t *)&graphData,15);//syhtod 15可否计算
      framepoint=(uint8_t *)&graphData;
      UI_GraphReFresh_data.frametail=Get_CRC16_Check_Sum(framepoint,15,UI_GraphReFresh_data.frametail);                                          
   }

   RefereeSend((uint8_t *)&UI_GraphReFresh_data.frametail,LEN_TAIL); //发送CRC16校验值

   va_end(ap);//结束可变参数的获取
   UI_Seq++;     //包序号+1
   return 0;
}

/************************************************UI推送字符（使更改生效）*********************************/
int Char_ReFresh(String_Data_t string_Data)
{
   UI_CharReFresh_t UI_CharReFresh_data;

   UI_CharReFresh_data.FrameHeader.SOF = REFEREE_SOF;
   UI_CharReFresh_data.FrameHeader.DataLength = UI_Data_LEN_DrawChar;
   UI_CharReFresh_data.FrameHeader.Seq = UI_Seq;
   UI_CharReFresh_data.FrameHeader.CRC8 = Get_CRC8_Check_Sum((uint8_t *)&UI_CharReFresh_data,4,0xFF);

   UI_CharReFresh_data.CmdID = ID_student_interactive;

   UI_CharReFresh_data.datahead.data_cmd_id = UI_Data_ID_DrawChar;
   UI_CharReFresh_data.datahead.receiver_ID = Cilent_ID;
   UI_CharReFresh_data.datahead.sender_ID = Robot_ID;

   UI_CharReFresh_data.String_Data = string_Data;

   UI_CharReFresh_data.frametail = Get_CRC16_Check_Sum((uint8_t *)&UI_CharReFresh_data,LEN_HEADER+LEN_CMDID+UI_Data_LEN_DrawChar,0xFFFF);

   RefereeSend((uint8_t *)&UI_CharReFresh_data,LEN_HEADER+LEN_CMDID+UI_Data_LEN_DrawChar+LEN_TAIL); //发送 

   UI_Seq++;                                                         //包序号+1
   return 0;
}




// #define send_max_len     200
// unsigned char CliendTxBuffer[send_max_len];
// void JUDGE_Show_Data(void)
// {
// 	static uint8_t datalength,i;
// 	uint8_t judge_led = 0xff;//初始化led为全绿
// 	static uint8_t auto_led_time = 0;
// 	static uint8_t buff_led_time = 0;
	
// 	determine_ID();//判断发送者ID和其对应的客户端ID
	
// 	ShowData.txFrameHeader.SOF = 0xA5;
// 	ShowData.txFrameHeader.DataLength = sizeof(ext_student_interactive_header_data_t) + sizeof(client_custom_data_t);
// 	ShowData.txFrameHeader.Seq = 0;
// 	memcpy(CliendTxBuffer, &ShowData.txFrameHeader, sizeof(xFrameHeader));//写入帧头数据
//    Append_CRC8_Check_Sum(CliendTxBuffer, sizeof(xFrameHeader));//写入帧头CRC8校验码
	
// 	ShowData.CmdID = 0x0301;
	
// 	ShowData.dataFrameHeader.data_cmd_id = 0xD180;//发给客户端的cmd,官方固定
// 	//ID已经是自动读取的了
// 	ShowData.dataFrameHeader.send_ID 	 = Judge_Self_ID;//发送者的ID
// 	ShowData.dataFrameHeader.receiver_ID = Judge_SelfClient_ID;//客户端的ID，只能为发送者机器人对应的客户端
	
// 	/*- 自定义内容 -*/
// 	ShowData.clientData.data1 = (float)Capvoltage_Percent();//电容剩余电量
// 	ShowData.clientData.data2 = (float)Base_Angle_Measure();//吊射角度测
// 	ShowData.clientData.data3 = GIMBAL_PITCH_Judge_Angle();//云台抬头角度
// 	ShowData.clientData.masks = judge_led;//0~5位0红灯,1绿灯
	
// 	//打包写入数据段
// 	memcpy(	
// 			CliendTxBuffer + 5, 
// 			(uint8_t*)&ShowData.CmdID, 
// 			(sizeof(ShowData.CmdID)+ sizeof(ShowData.dataFrameHeader)+ sizeof(ShowData.clientData))
// 		  );			
			
// 	Append_CRC16_Check_Sum(CliendTxBuffer,sizeof(ShowData));//写入数据段CRC16校验码	

// 	datalength = sizeof(ShowData); 
// 	for(i = 0;i < datalength;i++)
// 	{
// 		USART_SendData(UART5,(uint16_t)CliendTxBuffer[i]);
// 		while(USART_GetFlagStatus(UART5,USART_FLAG_TC)==RESET);
// 	}	 
// }




// /**
//   * @brief  判断自己红蓝方
//   * @param  void
//   * @retval RED   BLUE
//   * @attention  数据打包,打包完成后通过串口发送到裁判系统
//   */
// bool Color;
// bool is_red_or_blue(void)
// {
// 	Judge_Self_ID = GameRobotStat.robot_id;//读取当前机器人ID
	
// 	if(GameRobotStat.robot_id > 10)
// 	{
// 		return BLUE;
// 	}
// 	else 
// 	{
// 		return RED;
// 	}
// }

// /**
//   * @brief  判断自身ID，选择客户端ID
//   * @param  void
//   * @retval RED   BLUE
//   * @attention  数据打包,打包完成后通过串口发送到裁判系统
//   */
// void determine_ID(void)
// {
// 	Color = is_red_or_blue();
// 	if(Color == BLUE)
// 	{
// 		Judge_SelfClient_ID = 0x0110 + (Judge_Self_ID-10);//计算客户端ID
// 	}
// 	else if(Color == RED)
// 	{
// 		Judge_SelfClient_ID = 0x0100 + Judge_Self_ID;//计算客户端ID
// 	}
// }          