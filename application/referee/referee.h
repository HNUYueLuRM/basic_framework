#ifndef REFEREE_H
#define REFEREE_H

#include "rm_referee.h"
#include "robot_def.h"
#pragma pack(1)

typedef struct
{
	uint32_t chassis_flag  : 1;
	uint32_t gimbal_flag  : 1;
	uint32_t cover_flag  : 1;
	uint32_t friction_flag  : 1;
	uint32_t Power_flag  : 1;
	uint32_t end_angle_flag  : 1;

} Referee_Interactive_Flag_t; 


// 此结构体包含UI绘制与机器人车间通信的需要的其他非裁判系统数据
typedef struct
{	
    Referee_Interactive_Flag_t Referee_Interactive_Flag;
	//为UI绘制以及交互数据所用
	Robot_Status_e Robot_Status;// 机器人状态
	App_Status_e App_Status;// 应用状态
	chassis_mode_e chassis_mode;//底盘模式
	gimbal_mode_e gimbal_mode;//云台模式
	shoot_mode_e shoot_mode;//发射模式设置
	friction_mode_e friction_mode;//摩擦轮关闭
	lid_mode_e lid_mode;//弹舱盖打开
	loader_mode_e loader_mode;//单发...连发
	Chassis_Power_Data_s Chassis_Power_Data;// 功率控制

} Referee_Interactive_info_t;

#pragma pack()


void Referee_Interactive_init(void);
void Referee_Interactive_task(void);
#endif // REFEREE_H



