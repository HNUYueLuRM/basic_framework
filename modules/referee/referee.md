# referee

## referee运行流程

首先在chassis的初始化中调用裁判系统初始化函数，将要绘制的uidata的指针传递给接口，接口会返回裁判系统的反馈数据指针。然后，在refereeUItask里进行UI初始化，确定ui发送的目标并绘制初始化UI。完成后，uitask会以10hz的频率按顺序更新UI。

## 如何绘制你的自定义UI？以绘制超级电容能量条为例

UI的绘制包含初始化和TASK两个部分，初始化部分在`MyUIInit`函数中，TASK部分在`MyUIRefresh`函数中。

### 初始化部分

初始化部分的UI主要有两个目的：静态UI的绘制、为动态UI的绘制做准备。

分析超级电容能量条功能可知，此UI包含如下：
Power：xxx Power为静态不变的，冒号后的xxx为变化的量。
方框以及方框内的能量条：方框为静态不变的，能量条为变化的量。（参考游戏血条）

因而，静态UI的绘制包含如下：
绘制字符“Power:”、绘制矩形方框。
为动态UI的准备如下：
绘制矩形方框内的初始能量条、绘制Power的初始值。

### 绘制字符“Power:”

设置绘制用结构体，此处使用数组是因为需要绘制多个字符。本次绘制的字符为“Power:”，只是用到了第6个，即xxx[5]：

```c
static String_Data_t UI_State_sta[6];  // 静态
```

字符格式以及内容设置：

```c
UICharDraw(&UI_State_sta[5], "ss5", UI_Graph_ADD, 7, UI_Color_Green, 18, 2, 620, 230, "Power:");

//各参数意义如下，函数定义中有详细注释：
        string String_Data类型变量指针，用于存放字符串数据
        stringname[3]   字符串名称，用于标识更改
        String_Operate   字符串操作，初始化时一般使用"UI_Graph_ADD"
        String_Layer    图层0-9
        String_Color    字符串颜色
        String_Size     字号
        String_Width    字符串线宽
        Start_x、Start_y    开始坐标
        *stringdata    字符串数据

//设置完毕后，使用“Char_ReFresh”发送即可：
UICharRefresh(&referee_recv_info->referee_id, UI_State_sta[5]);
```

### 绘制能量框

定义一个图形类结构体，用于绘制能量框：

```c
static Graph_Data_t UI_energy_line[3]; // 电容能量条
```

能量框参数设置以及发送函数：

```c
UIRectangleDraw(&UI_energy_line[0],"ss6", UI_Graph_ADD, 7, UI_Color_Green,20, 720, 220, 820, 240)
UIRefresh(&referee_recv_info->referee_id, 1,UI_energy_line[0]);
```

### 绘制power的初始值

```c
UIFloatDraw(&UI_Energy[1], "sd5", UI_Graph_ADD, 8, UI_Color_Green, 18, 2, 2, 750, 230, 24000);
```

### 绘制能量条的初始值

```c
UILineDraw(&UI_Energy[2], "sd6", UI_Graph_ADD, 8, UI_Color_Pink, 30, 720, 160, 1020, 160);
```

将两个图形打包发送

```
UIRefresh(&referee_recv_info->referee_id, 2, UI_Energy[1], UI_Energy[2]);
```

## TASK部分

task中UI处于动态变化，此时需要检测所画的UI是否发生变化，若发生变化，则刷新对应UI。

### 添加变化检测

绘制功率部分UI，我们需要的是`Chassis_Power_Data_s`中的数据，我们定义`Chassis_Power_Data_s Chassis_Power_Data;`和`Chassis_Power_Data_s Chassis_last_Power_Data;`分别存储此次和上次的对应数据，本次和上次对应检测变化的需求。

```c
typedef struct
{
 Referee_Interactive_Flag_t Referee_Interactive_Flag;
 // 为UI绘制以及交互数据所用
 chassis_mode_e chassis_mode;    // 底盘模式
 gimbal_mode_e gimbal_mode;     // 云台模式
 shoot_mode_e shoot_mode;     // 发射模式设置
 friction_mode_e friction_mode;    // 摩擦轮关闭
 lid_mode_e lid_mode;      // 弹舱盖打开
 Chassis_Power_Data_s Chassis_Power_Data; // 功率控制

 // 上一次的模式，用于flag判断
 chassis_mode_e chassis_last_mode; 
 gimbal_mode_e gimbal_last_mode;  
 shoot_mode_e shoot_last_mode;  
 friction_mode_e friction_last_mode; 
 lid_mode_e lid_last_mode;   
 Chassis_Power_Data_s Chassis_last_Power_Data;

} Referee_Interactive_info_t;
```

添加功率变化标志位，`uint32_t Power_flag : 1;`，1为检测到变化，0为未检测到变换

```
typedef struct
{
 uint32_t chassis_flag : 1;
 uint32_t gimbal_flag : 1;
 uint32_t shoot_flag : 1;
 uint32_t lid_flag : 1;
 uint32_t friction_flag : 1;
 uint32_t Power_flag : 1;
} Referee_Interactive_Flag_t;
```

在变化检测函数中增加对应判断,由于voltage和能量条的变化对应同一个参数`Chassis_last_Power_Data.chassis_power_mx`的变化，所以只需要一个参数即可：

```
static void UIChangeCheck(Referee_Interactive_info_t *_Interactive_data)
{
    if (_Interactive_data->chassis_mode != _Interactive_data->chassis_last_mode)
    ......
    ......
    if (_Interactive_data->lid_mode != _Interactive_data->lid_last_mode)
    {
        _Interactive_data->Referee_Interactive_Flag.lid_flag = 1;
        _Interactive_data->lid_last_mode = _Interactive_data->lid_mode;
    }

	if (_Interactive_data->Chassis_Power_Data.chassis_power_mx != _Interactive_data->Chassis_last_Power_Data.chassis_power_mx)
    {
        _Interactive_data->Referee_Interactive_Flag.Power_flag = 1;
        _Interactive_data->Chassis_last_Power_Data.chassis_power_mx = _Interactive_data->Chassis_Power_Data.chassis_power_mx;
    }
}
```

### 根据功率的变化绘制UI

在绘制变化的UI时，由于初始化时已经使用`UI_Graph_ADD`操作添加了UI，所以在绘制时，需要使用`UI_Graph_Change`操作，以便于刷新UI。

同时，完成UI刷新后，需要将对应的flag置0，以便于下次检测变化

```
if (_Interactive_data->Referee_Interactive_Flag.Power_flag == 1)
{
 UIFloatDraw(&UI_Energy[1], "sd5", UI_Graph_Change, 8, UI_Color_Green, 18, 2, 2, 750, 230, _Interactive_data->Chassis_Power_Data.chassis_power_mx * 1000);
 UILineDraw(&UI_Energy[2], "sd6", UI_Graph_Change, 8, UI_Color_Pink, 30, 720, 160, (uint32_t)750 + _Interactive_data->Chassis_Power_Data.chassis_power_mx * 30, 160);
 UIRefresh(&referee_recv_info->referee_id, 2, UI_Energy[1], UI_Energy[2]);
 _Interactive_data->Referee_Interactive_Flag.Power_flag = 0;
}
```

---

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
 UI_Seq++;                    // 包序号+1
}
```
