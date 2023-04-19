# referee
## 如何绘制你的自定义UI？以绘制超级电容能量条为例：
UI的绘制包含初始化和TASK两个部分，初始化部分在`My_UI_init`函数中，TASK部分在`My_UI_Refresh`函数中。

## 初始化部分：
初始化部分的UI主要有两个目的：静态UI的绘制、为动态UI的绘制做准备。

分析超级电容能量条功能可知，此UI包含如下：
voltage：xxx voltage为静态不变的，冒号后的xxx为变化的量。
方框以及方框内的能量条：方框为静态不变的，能量条为变化的量。（参考游戏血条）

因而，静态UI的绘制包含如下：
绘制字符“voltage:”、绘制矩形方框。
为动态UI的准备如下：
绘制矩形方框内的初始能量条、绘制voltage的初始值。

### 绘制字符“voltage:”：
设置绘制用结构体，此处使用数组是因为需要绘制多个字符。本次绘制的字符为“voltage:”，只是用到了第6个，即xxx[5]：
```c
static String_Data_t UI_State_sta[6];  // 静态
static String_Data_t UI_State_dyn[6];  // 动态
```
字符格式以及内容设置：
```c
// 底盘功率显示，静态
Char_Draw(&UI_State_sta[5], "ss5", UI_Graph_ADD, 8, UI_Color_Green, 18, 2, 720, 210, "Voltage:");
```
各参数意义如下，函数定义中有详细注释：

        string String_Data类型变量指针，用于存放字符串数据
        stringname[3]   字符串名称，用于标识更改
        String_Operate   字符串操作，初始化时一般使用"UI_Graph_ADD"
        String_Layer    图层0-9
        String_Color    字符串颜色
        String_Size     字号
        String_Width    字符串线宽
        Start_x、Start_y    开始坐标
        *stringdata    字符串数据
设置完毕后，使用“Char_ReFresh”发送即可：
```c
// 底盘功率显示，静态
Char_ReFresh(&_referee_info->referee_id, UI_State_sta[5]);
```
### 绘制能量框：
定义一个图形类结构体，用于绘制能量框：
```c
static Graph_Data_t UI_energy_line[1]; // 电容能量条
```




若需要进行多机交互，可增加此函数：
```c
void CommBetweenRobotSend(referee_id_t *_id, robot_interactive_data_t *_data)
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

	SendData.frametail = Get_CRC16_Check_Sum((uint8_t *)&SendData, LEN_HEADER + LEN_CMDID + temp_datalength, 0xFFFF);

	RefereeSend((uint8_t *)&SendData, LEN_HEADER + LEN_CMDID + temp_datalength + LEN_TAIL); // 发送
	UI_Seq++;																				// 包序号+1
}
```