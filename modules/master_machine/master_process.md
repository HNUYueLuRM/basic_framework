# master_process



<p align='right'>neozng1@hnu.edu.cn</p>

> TODO:
>
> 1. 补全标志位解析和发送设置的代码
> 2. 增加发送给视觉数据的时间戳用于数据对齐



## 总览和封装说明

模块包含了和视觉通信的初始化、向上位机发送信息的接口和模块的串口的回调处理。接口的定义统一，可以方便的替换成其他通信方式，如CAN。

## 代码结构

.h文件内包括了外部接口和与**视觉上位机通信的数据结构定义**，以及模块对应的宏。c文件内为私有函数和外部接口的定义。

本模块主要是对协议解析的处理和协议发送的封装，实际内容不多。协议相关内容都在`seasky_protocol.h`中。

## 类型定义

和视觉通信所必须的标志位和数据。包括开火模式，目标状态，目标类型，接收/发送数据结构体。

```c
typedef enum
{
	NO_FIRE = 0,
	AUTO_FIRE = 1,
	AUTO_AIM = 2
} Fire_Mode_e;

typedef enum
{
	NO_TARGET = 0,
	TARGET_CONVERGING = 1,
	READY_TO_FIRE = 2
} Target_State_e;

typedef enum
{
	NO_TARGET_NUM = 0,
	HERO1 = 1,
	ENGINEER2 = 2,
	INFANTRY3 = 3,
	INFANTRY4 = 4,
	INFANTRY5 = 5,
	OUTPOST = 6,
	SENTRY = 7,
	BASE = 8
} Target_Type_e;

typedef struct
{
	Fire_Mode_e fire_mode;
	Target_State_e target_state;
	Target_Type_e target_type;

	float pitch;
	float yaw;
} Vision_Recv_s;

typedef enum
{
	BLUE = 0,
	RED = 1
} Enemy_Color_e;

typedef enum
{
	MODE_AIM = 0,
	MODE_SMALL_BUFF = 1,
	MODE_BIG_BUFF = 2
} Work_Mode_e;

typedef enum
{
	BIG_AMU_10 = 10,
	SMALL_AMU_15 = 15,
	BIG_AMU_16 = 16,
	SMALL_AMU_18 = 18,
	SMALL_AMU_30 = 30,
} Bullet_Speed_e;

typedef struct
{
	Enemy_Color_e enemy_color;
	Work_Mode_e work_mode;
	Bullet_Speed_e bullet_speed;

	float yaw;
	float pitch;
	float roll;
} Vision_Send_s;
```

## 外部接口

```c
Vision_Recv_s *VisionInit(UART_HandleTypeDef *_handle);

void VisionSend(Vision_Send_s *send);
```

给`VisionInit()`传入串口handle，将初始化一个视觉通信模块，返回值是接收数据的结构体指针。拥有视觉模块的应用应该在初始化中调用此函数，并保存返回值的指针。

调用`VisionSend`并传入填好发送数据的结构体，会通过底层的通信模块向视觉发送一帧报文。

## 私有函数和变量

```c
static Vision_Recv_s recv_data;

static usart_instance *vision_usart_instance;

static void DecodeVision()
{
    static uint16_t flag_register;
    get_protocol_info(vision_usart_instance->recv_buff, &flag_register, (uint8_t*)&recv_data.pitch);
    // TODO: code to resolve flag_register;
}
```

第一个是保存接收数据的结构体，其指针将会在初始化的时候返回给拥有者。目前最多只能配置一个视觉模块。

第二个是该模块拥有的串口实例指针，用于调度其底层的发送和接收。如果要换成CAN/SPI等，替换成对应实例，并修改初始化和发送的实现即可。

`DecodeVision()`是解析视觉接收数据的回调函数，会在串口接收回调中被调用。如果修改通信协议，只需要更改

`get_protocol_info()`。