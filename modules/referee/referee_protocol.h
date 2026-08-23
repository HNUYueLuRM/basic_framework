/**
 * @file referee_protocol.h
 * @author kidneygood (you@domain.com)
 * @version 0.1
 * @date 2022-12-02
 *
 * @copyright Copyright (c) HNU YueLu EC 2022 all rights reserved
 *
 */

#ifndef referee_protocol_H
#define referee_protocol_H

#include "stdint.h"

/****************************宏定义部分****************************/

#define REFEREE_SOF 0xA5 // 起始字节,协议固定为0xA5
#define Robot_Red 0
#define Robot_Blue 1
#define Communicate_Data_LEN 5 // 自定义交互数据长度，该长度决定了我方发送和他方接收，自定义交互数据协议更改时只需要更改此宏定义即可
#define SELF_CTRL_DATA_LEN 30 //自定义控制器数据长度
#define CLIENT_DATA_LEN 300 //自定义客户端数据长度
#define CLIENT_DATA_RX_LEN 30 //自定义客户端数据长度

#pragma pack(1)

/****************************通信协议格式****************************/

/* 通信协议格式偏移，枚举类型,代替#define声明 */
typedef enum
{
	FRAME_HEADER_Offset = 0,
	CMD_ID_Offset = 5,
	DATA_Offset = 7,
} JudgeFrameOffset_e;

/* 通信协议长度 */
typedef enum
{
	LEN_HEADER = 5, // 帧头长
	LEN_CMDID = 2,	// 命令码长度
	LEN_TAIL = 2,	// 帧尾CRC16

	LEN_CRC8 = 4, // 帧头CRC8校验长度=帧头+数据长+包序号
} JudgeFrameLength_e;

/****************************帧头****************************/
/****************************帧头****************************/

/* 帧头偏移 */
typedef enum
{
	SOF = 0,		 // 起始位
	DATA_LENGTH = 1, // 帧内数据长度,根据这个来获取数据长度
	SEQ = 3,		 // 包序号
	CRC8 = 4		 // CRC8
} FrameHeaderOffset_e;

/* 帧头定义 */
typedef struct
{
	uint8_t SOF;
	uint16_t DataLength;
	uint8_t Seq;
	uint8_t CRC8;
} xFrameHeader;

/****************************cmd_id命令码说明****************************/
/****************************cmd_id命令码说明****************************/

/* 命令码ID,用来判断接收的是什么数据 */
typedef enum
{
	ID_game_state = 0x0001,				   // 比赛状态数据
	ID_game_result = 0x0002,			   // 比赛结果数据
	ID_game_robot_survivors = 0x0003,	   // 比赛机器人血量数据
	ID_event_data = 0x0101,				   // 场地事件数据
	ID_supply_projectile_booking = 0x0103, // 场地补给站预约子弹数据
	ID_referee_warning_t = 0x0104,		   // 裁判警告信息
	ID_dart_info_t = 0x0105,			   // 场地飞镖发射数据
	ID_game_robot_state = 0x0201,		   // 机器人状态数据
	ID_power_heat_data = 0x0202,		   // 实时功率热量数据
	ID_game_robot_pos = 0x0203,			   // 机器人位置数据
	ID_buff_musk = 0x0204,				   // 机器人增益数据
	ID_robot_hurt = 0x0206,				   // 伤害状态数据
	ID_shoot_data = 0x0207,				   // 实时射击数据
	ID_projectile_allowance_t = 0x0208,	   // 弹仓数据
	ID_rfid_status_t = 0x0209,			   // RFID状态数据
	ID_student_interactive = 0x0301,	   // 机器人间交互数据
	ID_custom2robot = 0x0302,                 // 自定义控制器数据
	ID_map_interact_data = 0x0303,          //小地图交互数据
	ID_remote_control = 0x0304,            // 图传链路键鼠数据
	ID_radar2server = 0x0305,              // 雷达链路数据
	ID_customl2player = 0x0306,           // 自定义控制器向操作端发送数据
	ID_robot2custom = 0x0309,            // 机器人向自定义控制器发送数据
	ID_robot2selfclient = 0x0310,          // 机器人向自定义客户端发送数据
	ID_selfclient2robot = 0x0311          // 自定义客户端向机器人发送数据
} CmdID_e;

/* 命令码数据段长,根据官方协议来定义长度，还有自定义数据长度 */
typedef enum
{
	LEN_game_state = 11,						 // 0x0001
	LEN_game_result = 1,						 // 0x0002
	LEN_game_robot_HP = 20,						 // 0x0003,2026更改为20字节
	LEN_event_data = 4,							 // 0x0101
	LEN_referee_warning_t = 3,					 // 0x0104
	LEN_dart_info_t = 3,							 // 0x0105
	LEN_game_robot_state = 17,					 // 0x0201
	LEN_power_heat_data = 14,					 // 0x0202,2026更改为14字节
	LEN_game_robot_pos = 16,					 // 0x0203
	LEN_buff_musk = 8,							 // 0x0204，2026更改为8字节
	LEN_robot_hurt = 1,							 // 0x0206
	LEN_shoot_data = 7,							 // 0x0207
	LEN_projectile_allowance_t = 8,				 // 0x0208
	LEN_rfid_status_t = 5,						 // 0x0209，2026更改为5字节
	LEN_receive_data = 6 + Communicate_Data_LEN, // 0x0301
    LEN_map_interact_data = 15,                  // 0x0303
	LEN_radar2server_data = 48,					// 0x0305
	LEN_selfctrl2player_data = 8,					// 0x0306
	/*以上为常规链路*/
	LEN_custom2robot_data = 30, 					// 0x0302
	LEN_remote_control_data = 12,   				// 0x0304
	LEN_robot2custom_data = 30,					// 0x0309
	LEN_robot2selfclient_data = 300,			//0x0310 
	LEN_selfclient2robot = 30

} JudgeDataLength_e;

/****************************接收数据的详细说明****************************/
/****************************接收数据的详细说明****************************/

/* ID: 0x0001  Byte:  11    比赛状态数据 */
typedef  struct 
{ 
 uint8_t game_type : 4; 
 uint8_t game_progress : 4; 
 uint16_t stage_remain_time; 
 uint64_t SyncTimeStamp; 
} ext_game_state_t;

/* ID: 0x0002  Byte:  1    比赛结果数据 */
typedef struct
{
	uint8_t winner;
} ext_game_result_t;

/* ID: 0x0003  Byte:  20    比赛机器人血量数据 */
typedef struct
{
	uint16_t self_1_robot_HP;
	uint16_t self_2_robot_HP;
	uint16_t self_3_robot_HP;
	uint16_t self_4_robot_HP;
	int16_t diff_damage;
	uint16_t self_7_robot_HP;
	uint16_t self_outpost_HP;
	uint16_t self_base_HP;
	uint16_t opponent_outpost_HP;
	uint16_t opponent_base_HP;
} ext_game_robot_HP_t;

/* ID: 0x0101  Byte:  4    场地事件数据 */
typedef struct
{
	uint32_t event_data;
} ext_event_data_t;

/* ID: 0x0104  Byte:  3    裁判警告数据 */
typedef struct 
{ 
  uint8_t level; 
  uint8_t offending_robot_id; 
  uint8_t count; 
}ext_referee_warning_t;

/* ID: 0x0105  Byte:  3    场地飞镖发射数据 */
typedef struct
{
 uint8_t dart_remaining_time;
 uint16_t dart_info;
}ext_dart_info_t; 

/* ID: 0X0201  Byte: 17    机器人状态数据 */
typedef struct
{
	uint8_t robot_id;
	uint8_t robot_level;
	uint16_t current_HP;
	uint16_t maximum_HP;
	uint16_t shooter_barrel_cooling_value;
	uint16_t shooter_barrel_heat_limit;
	uint16_t chassis_power_limit;
	float shooter_speed_limit;
	uint8_t power_management_gimbal_output : 1;
	uint8_t power_management_chassis_output : 1;
	uint8_t power_management_shooter_output : 1; 
} ext_game_robot_state_t;

/* ID: 0X0202  Byte: 14    实时功率热量数据 */
typedef struct
{
	uint16_t reserved1; 
	uint16_t reserved2;
	uint32_t reserved3;
	uint16_t buffer_energy;
	uint16_t shooter_17mm_barrel_heat;
	uint16_t shooter_42mm_barrel_heat; 
} ext_power_heat_data_t;

/* ID: 0x0203  Byte: 16    机器人位置数据 */
typedef struct
{
	float x;
	float y;
	float z;
	float yaw;
} ext_game_robot_pos_t;

/* ID: 0x0204  Byte:  8    机器人增益数据 */
typedef struct
{
    uint8_t recovery_buff;
    uint16_t cooling_buff;
    uint8_t defence_buff;
    uint8_t vulnerability_buff;
    uint16_t attack_buff;
    uint8_t remaining_energy; 
} ext_buff_musk_t;

/* ID: 0x0206  Byte:  1    伤害状态数据 */
typedef struct
{
	uint8_t armor_id : 4;
	uint8_t hurt_type : 4;
} ext_robot_hurt_t;

/* ID: 0x0207  Byte:  7    实时射击数据 */
typedef struct
{
	uint8_t bullet_type;
	uint8_t shooter_id;
	uint8_t bullet_freq;
	float bullet_speed;
} ext_shoot_data_t;

/* ID：0x0208  Byte:  8    机器人允许发弹量数据 */
typedef struct
{
	uint16_t projectile_allowance_17mm;
	uint16_t projectile_allowance_42mm;
	uint16_t remaining_gold_coin;
	uint16_t projectile_allowance_fortress;	//储备17mm弹丸允许发弹量
} ext_projectile_allowance_t;

/* ID: 0x0209  Byte:  5    RFID状态数据 */
typedef struct
{
	uint32_t rfid_status;
	uint8_t rfid_status2;
} ext_rfid_status_t;

// /* ID：0x020A  Byte:  6    飞镖发射站状态 */
// typedef struct
// {
// 	uint8_t dart_launch_opening_status;
// 	uint8_t reserved;
// 	uint16_t target_change_time;
// 	uint16_t latest_launch_cmd_time;
// } ext_dart_client_cmd_t;

// /* ID：0x020B  Byte:  40   地面机器人位置状态*/
// typedef struct
// {
// 	float hero_x;
// 	float hero_y;
// 	float engineer_x;
// 	float engineer_y;
// 	float standard_3_x;
// 	float standard_3_y;
// 	float standard_4_x;
// 	float standard_4_y;
// 	float reserved;
// } ext_ground_robot_position_t;

// /* ID：0x020C  Byte:  2    雷达标识 */
// typedef struct
// {
// 	uint16_t mark_progress;
// } ext_radar_mark_data_t;

// /* ID：0x020D  Byte:  6    哨兵状态 */
// typedef struct
// {
// 	uint32_t sentry_info;
// 	uint16_t sentry_info_2;
// } ext_sentry_info_t;

// /* ID：0x020E  Byte:  1    雷达状态*/
// typedef struct
// {
// 	uint8_t radar_info;
// } ext_radar_info_t;

 /* ID: 0x0303  Byte:  15   选手端小地图交互数据 */
 typedef struct 
{ 
    float target_position_x; 
    float target_position_y; 
    uint8_t cmd_keyboard; 
    uint8_t target_robot_id; 
    uint16_t cmd_source; 
	uint8_t reserved[3];// 官方文档有矛盾，先这么写保险
}ext_map_command_t; 

/****************************机器人交互数据****************************/
/****************************机器人交互数据****************************/
/* 发送的内容数据段最大为 113 检测是否超出大小限制?实际上图形段不会超，数据段最多30个，也不会超*/
/* 交互数据头结构 */
typedef struct
{
	uint16_t data_cmd_id; // 由于存在多个内容 ID，但整个cmd_id 上行频率最大为 30Hz，请合理安排带宽。注意交互部分的上行频率
	uint16_t sender_ID;
	uint16_t receiver_ID;
} ext_student_interactive_header_data_t;

/* 机器人id */
typedef enum
{
	// 红方机器人ID
	RobotID_RHero = 1,
	RobotID_REngineer = 2,
	RobotID_RStandard1 = 3,
	RobotID_RStandard2 = 4,
	RobotID_RStandard3 = 5,
	RobotID_RAerial = 6,
	RobotID_RSentry = 7,
	RobotID_RRadar = 9,
	// 蓝方机器人ID
	RobotID_BHero = 101,
	RobotID_BEngineer = 102,
	RobotID_BStandard1 = 103,
	RobotID_BStandard2 = 104,
	RobotID_BStandard3 = 105,
	RobotID_BAerial = 106,
	RobotID_BSentry = 107,
	RobotID_BRadar = 109,
} Robot_ID_e;

/* 交互数据ID */
typedef enum
{
	UI_Data_ID_Del = 0x100,
	UI_Data_ID_Draw1 = 0x101,
	UI_Data_ID_Draw2 = 0x102,
	UI_Data_ID_Draw5 = 0x103,
	UI_Data_ID_Draw7 = 0x104,
	UI_Data_ID_DrawChar = 0x110,

	/* 自定义交互数据部分 */
	Communicate_Data_ID = 0x0200,

} Interactive_Data_ID_e;
/* 交互数据长度 */
typedef enum
{
	Interactive_Data_LEN_Head = 6,
	UI_Operate_LEN_Del = 2,
	UI_Operate_LEN_PerDraw = 15,
	UI_Operate_LEN_DrawChar = 15 + 30,

	/* 自定义交互数据部分 */
	Robot_Custom_Data_LEN = 30, //机器人发给自定义控制器数据，长度30

} Interactive_Data_Length_e;

/****************************自定义交互数据****************************/
/*
	学生机器人间通信 cmd_id 0x0301，内容 ID:0x0200~0x02FF
	自定义交互数据 机器人间通信：0x0301。
	发送频率：上限 10Hz
*/
// 自定义交互数据协议，可更改，更改后需要修改最上方宏定义数据长度的值
typedef struct
{
	uint8_t data[Communicate_Data_LEN]; // 数据段,n需要小于113
} robot_interactive_data_t;

// 机器人交互信息_发送
typedef struct
{
	xFrameHeader FrameHeader;
	uint16_t CmdID;
	ext_student_interactive_header_data_t datahead;
	robot_interactive_data_t Data; // 数据段
	uint16_t frametail;
} Communicate_SendData_t;
// 机器人交互信息_接收
typedef struct
{
	ext_student_interactive_header_data_t datahead;
	robot_interactive_data_t Data; // 数据段
} Communicate_ReceiveData_t;
/****************************UI交互数据****************************/

/* 图形数据 */
typedef struct
{
	uint8_t graphic_name[3];
	uint32_t operate_tpye : 3;
	uint32_t graphic_tpye : 3;
	uint32_t layer : 4;
	uint32_t color : 4;
	uint32_t start_angle : 9;
	uint32_t end_angle : 9;
	uint32_t width : 10;
	uint32_t start_x : 11;
	uint32_t start_y : 11;
	uint32_t radius : 10;
	uint32_t end_x : 11;
	uint32_t end_y : 11;
} Graph_Data_t;

typedef struct
{
	Graph_Data_t Graph_Control;
	uint8_t show_Data[30];
} String_Data_t; // 打印字符串数据

/* 删除操作 */
typedef enum
{
	UI_Data_Del_NoOperate = 0,
	UI_Data_Del_Layer = 1,
	UI_Data_Del_ALL = 2, // 删除全部图层，后面的参数已经不重要了。
} UI_Delete_Operate_e;

/* 图形配置参数__图形操作 */
typedef enum
{
	UI_Graph_ADD = 1,
	UI_Graph_Change = 2,
	UI_Graph_Del = 3,
} UI_Graph_Operate_e;

/* 图形配置参数__图形类型 */
typedef enum
{
	UI_Graph_Line = 0,		// 直线
	UI_Graph_Rectangle = 1, // 矩形
	UI_Graph_Circle = 2,	// 整圆
	UI_Graph_Ellipse = 3,	// 椭圆
	UI_Graph_Arc = 4,		// 圆弧
	UI_Graph_Float = 5,		// 浮点型
	UI_Graph_Int = 6,		// 整形
	UI_Graph_Char = 7,		// 字符型

} UI_Graph_Type_e;

/* 图形配置参数__图形颜色 */
typedef enum
{
	UI_Color_Main = 0, // 红蓝主色
	UI_Color_Yellow = 1,
	UI_Color_Green = 2,
	UI_Color_Orange = 3,
	UI_Color_Purplish_red = 4, // 紫红色
	UI_Color_Pink = 5,
	UI_Color_Cyan = 6, // 青色
	UI_Color_Black = 7,
	UI_Color_White = 8,

} UI_Graph_Color_e;

/****************************图传链路数据****************************/
/*cmd_id:0x0302 */
typedef struct
{
	uint8_t data[SELF_CTRL_DATA_LEN]; //长度固定，26赛季为30，视情况更改
} custom2robot_data_t; //自定义控制器发往机器人的数据

/*cmd_id:0x0309 */
typedef struct
{
	uint8_t data[SELF_CTRL_DATA_LEN]; //长度固定，26赛季为30，视情况更改
} robot2custom_data_t; //机器人发往自定义控制器的数据

/*cmd_id:0x0310 */
typedef struct
{
	uint8_t data[CLIENT_DATA_LEN]; //长度固定，26赛季为300，视情况更改
} robot2client_data_t; //机器人发往自定义客户端的数据

/*cmd_id:0x0311 */
typedef struct
{
	uint8_t data[CLIENT_DATA_RX_LEN]; //长度固定，26赛季为300，视情况更改
} client2robot_data_t; //自定义客户端发往机器人的数据

#pragma pack()

#endif // referee_protocol_H
