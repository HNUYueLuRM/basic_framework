/**
 * @file referee.C
 * @author kidneygood (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2022-11-18
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "referee.h"
#include "robot_def.h"
#include "rm_referee.h"
#include "referee_UI.h"

static Referee_Interactive_info_t Interactive_data; // 非裁判系统数据
static referee_info_t *referee_data;                 // 裁判系统相关数据
static void determine_ID(referee_info_t *_referee_info);
static void My_UI_init(referee_info_t *_referee_info);
static void My_UI_Refresh(referee_info_t *_referee_info, Referee_Interactive_info_t *_Interactive_data);
static void robot_mode_change(Referee_Interactive_info_t *_Interactive_data); // 测试用函数，实现模式自动变化

void Referee_Interactive_init()
{
    referee_data = RefereeInit(&huart6); // 裁判系统初始化
    while (referee_data->GameRobotState.robot_id == 0);
    determine_ID(referee_data);
    My_UI_init(referee_data);
}

void Referee_Interactive_task()
{
    robot_mode_change(&Interactive_data); // 测试用函数，实现模式自动变化
    My_UI_Refresh(referee_data, &Interactive_data);
}

static Graph_Data_t UI_shoot_line[10]; // 射击准线
static String_Data_t UI_State_sta[5];  // 机器人状态,静态只需画一次
static String_Data_t UI_State_dyn[5];  // 机器人状态,动态先add才能change
static uint32_t shoot_line_location[10] = {540, 960, 490, 515, 565};

static void My_UI_init(referee_info_t *_referee_info)
{
    UI_Delete(&_referee_info->referee_id, UI_Data_Del_ALL, 0);

    // 绘制发射基准线
    Line_Draw(&UI_shoot_line[0], "sl0", UI_Graph_ADD, 7, UI_Color_White, 3, 710, shoot_line_location[0], 1210, shoot_line_location[0]);
    Line_Draw(&UI_shoot_line[1], "sl1", UI_Graph_ADD, 7, UI_Color_White, 3, shoot_line_location[1], 340, shoot_line_location[1], 740);
    Line_Draw(&UI_shoot_line[2], "sl2", UI_Graph_ADD, 7, UI_Color_Yellow, 2, 810, shoot_line_location[2], 1110, shoot_line_location[2]);
    Line_Draw(&UI_shoot_line[3], "sl3", UI_Graph_ADD, 7, UI_Color_Yellow, 2, 810, shoot_line_location[3], 1110, shoot_line_location[3]);
    Line_Draw(&UI_shoot_line[4], "sl4", UI_Graph_ADD, 7, UI_Color_Yellow, 2, 810, shoot_line_location[4], 1110, shoot_line_location[4]);

    UI_ReFresh(&_referee_info->referee_id, 5, UI_shoot_line[0], UI_shoot_line[1], UI_shoot_line[2], UI_shoot_line[3], UI_shoot_line[4]);

    // 绘制车辆状态标志，静态
    Char_Draw(&UI_State_sta[0], "ss0", UI_Graph_ADD, 8, UI_Color_Main, 15, 2, 150, 750);
    Char_Write(&UI_State_sta[0], "chassis:");
    Char_ReFresh(&_referee_info->referee_id, UI_State_sta[0]);

    Char_Draw(&UI_State_sta[1], "ss1", UI_Graph_ADD, 8, UI_Color_Yellow, 15, 2, 150, 700);
    Char_Write(&UI_State_sta[1], "gimbal:");
    Char_ReFresh(&_referee_info->referee_id, UI_State_sta[1]);

    Char_Draw(&UI_State_sta[2], "ss2", UI_Graph_ADD, 8, UI_Color_Orange, 15, 2, 150, 650);
    Char_Write(&UI_State_sta[2], "cover:");
    Char_ReFresh(&_referee_info->referee_id, UI_State_sta[2]);

    Char_Draw(&UI_State_sta[3], "ss3", UI_Graph_ADD, 8, UI_Color_Pink, 15, 2, 150, 600);
    Char_Write(&UI_State_sta[3], "frict:");
    Char_ReFresh(&_referee_info->referee_id, UI_State_sta[3]);

    // 底盘功率显示，静态
    Char_Draw(&UI_State_sta[4], "ss4", UI_Graph_ADD, 8, UI_Color_Green, 18, 2, 720, 210);
    Char_Write(&UI_State_sta[4], "Power:");
    Char_ReFresh(&_referee_info->referee_id, UI_State_sta[4]);

    // 绘制车辆状态标志，动态  
    //syhtodo 由于初始化时xxx_last_mode默认为0，所以此处对应UI也应该设为0时对应的UI，防止模式不变的情况下无法置位flag，导致UI无法刷新
    Char_Draw(&UI_State_dyn[0], "sd0", UI_Graph_ADD, 8, UI_Color_Main, 15, 2, 270, 750);
    Char_Write(&UI_State_dyn[0], "0000");
    Char_ReFresh(&_referee_info->referee_id, UI_State_dyn[0]);

    Char_Draw(&UI_State_dyn[1], "sd1", UI_Graph_ADD, 8, UI_Color_Yellow, 15, 2, 270, 700);
    Char_Write(&UI_State_dyn[1], "0000");
    Char_ReFresh(&_referee_info->referee_id, UI_State_dyn[1]);

    Char_Draw(&UI_State_dyn[2], "sd2", UI_Graph_ADD, 8, UI_Color_Orange, 15, 2, 270, 650);
    Char_Write(&UI_State_dyn[2], "0000");
    Char_ReFresh(&_referee_info->referee_id, UI_State_dyn[2]);

    Char_Draw(&UI_State_dyn[3], "sd3", UI_Graph_ADD, 8, UI_Color_Pink, 15, 2, 270, 600);
    Char_Write(&UI_State_dyn[3], "0000");
    Char_ReFresh(&_referee_info->referee_id, UI_State_dyn[3]);

    // 底盘功率显示，动态
    Char_Draw(&UI_State_dyn[4], "sd4", UI_Graph_ADD, 8, UI_Color_Green, 18, 2, 840, 210);
    Char_Write(&UI_State_dyn[4], "0000");
    Char_ReFresh(&_referee_info->referee_id, UI_State_dyn[4]);
}

static uint8_t count = 0;
static uint16_t count1 = 0;
static void robot_mode_change(Referee_Interactive_info_t *_Interactive_data) // 测试用函数，实现模式自动变化
{
    count++;
    if (count >= 50)
    {
        count = 0;
        count1++;
    }
    switch (count1%4)
    {
        case 0:
        {
            _Interactive_data->chassis_mode =CHASSIS_ZERO_FORCE;
            _Interactive_data->gimbal_mode =GIMBAL_ZERO_FORCE;
            _Interactive_data->shoot_mode = SHOOT_ON;
            _Interactive_data->friction_mode = FRICTION_ON;
            _Interactive_data->lid_mode = LID_OPEN;
            break;

        }
        case 1:
        {;
            _Interactive_data->chassis_mode =CHASSIS_ROTATE;
            _Interactive_data->gimbal_mode =GIMBAL_FREE_MODE;
            _Interactive_data->shoot_mode = SHOOT_OFF;
            _Interactive_data->friction_mode = FRICTION_OFF;
            _Interactive_data->lid_mode = LID_CLOSE;
            break;
        }
        case 2:
        {
            _Interactive_data->chassis_mode =CHASSIS_NO_FOLLOW;
            _Interactive_data->gimbal_mode =GIMBAL_GYRO_MODE;
            _Interactive_data->shoot_mode = SHOOT_ON;
            _Interactive_data->friction_mode = FRICTION_ON;
            _Interactive_data->lid_mode = LID_OPEN;
            break;
        }
        case 3:
        {
             _Interactive_data->chassis_mode =CHASSIS_FOLLOW_GIMBAL_YAW;
            _Interactive_data->gimbal_mode =GIMBAL_ZERO_FORCE;
            _Interactive_data->shoot_mode = SHOOT_OFF;
            _Interactive_data->friction_mode = FRICTION_OFF;
            _Interactive_data->lid_mode = LID_CLOSE;
            break;
        }
        default : break;
    }   
}

static void My_UI_Refresh(referee_info_t *_referee_info, Referee_Interactive_info_t *_Interactive_data)
{
    // syhtodo 按键刷新
    // syhtodo与上次不一样才要进入刷新，避免重复发送，使用新结构体flag
    if (_Interactive_data->chassis_mode !=_Interactive_data->chassis_last_mode)
    {
        _Interactive_data->Referee_Interactive_Flag.chassis_flag=1;
        _Interactive_data->chassis_last_mode = _Interactive_data->chassis_mode;
    }

    if (_Interactive_data->Referee_Interactive_Flag.chassis_flag==1)
    {
        switch(_Interactive_data->chassis_mode)
        {
            case CHASSIS_ZERO_FORCE:
            {
                Char_Draw(&UI_State_dyn[0], "sd0", UI_Graph_Change, 8, UI_Color_Main, 15, 2, 270, 750);
                Char_Write(&UI_State_dyn[0], "zeroforce");
                break;
            }
            case CHASSIS_ROTATE:
            {
                Char_Draw(&UI_State_dyn[0], "sd0", UI_Graph_Change, 8, UI_Color_Main, 15, 2, 270, 750);
                Char_Write(&UI_State_dyn[0], "rotate   "); //此处注意字数对齐问题，字数相同才能覆盖掉 
                break;
            }
            case CHASSIS_NO_FOLLOW:
            {
                Char_Draw(&UI_State_dyn[0], "sd0", UI_Graph_Change, 8, UI_Color_Main, 15, 2, 270, 750);
                Char_Write(&UI_State_dyn[0], "nofollow ");
                break;
            }
            case CHASSIS_FOLLOW_GIMBAL_YAW:
            {
                Char_Draw(&UI_State_dyn[0], "sd0", UI_Graph_Change, 8, UI_Color_Main, 15, 2, 270, 750);
                Char_Write(&UI_State_dyn[0], "follow   ");
                break;
            }
        }

        Char_ReFresh(&_referee_info->referee_id, UI_State_dyn[0]);
        _Interactive_data->Referee_Interactive_Flag.chassis_flag=0;
    }









    // 			switch(dbus_infomation.gimbal_mode)
    // 			{
    // 				case mechanical_mode:
    // 				{
    // 					Char_Draw(&UI_State[1],"s2",UI_Graph_Change,8,UI_Color_Yellow,15,2,150,700);
    // 					Char_Write(&UI_State[1],"gimbal:mech  ");
    // 					break;
    // 				}
    // 				case visual_mode:
    // 				{
    // 					Char_Draw(&UI_State[1],"s2",UI_Graph_Change,8,UI_Color_Yellow,15,2,150,700);
    // 					Char_Write(&UI_State[1],"gimbal:visual");
    // 					break;
    // 				}
    // 				case gyro_mode:
    // 				{
    // 					Char_Draw(&UI_State[1],"s2",UI_Graph_Change,8,UI_Color_Yellow,15,2,150,700);
    // 					Char_Write(&UI_State[1],"gimbal:gyro  ");
    // 					break;
    // 				}
    // 			}
    // 			Char_ReFresh(UI_State[1]);

    // 			switch(dbus_infomation.cover_state)
    // 			{
    // 				case cover_close_sign:
    // 				{
    // 					Char_Draw(&UI_State[2],"s3",UI_Graph_Change,8,UI_Color_Orange,15,2,150,650);
    // 					Char_Write(&UI_State[2],"cover:OFF");
    // 					break;
    // 				}
    // 				case cover_open_sign:
    // 				{
    // 					Char_Draw(&UI_State[2],"s3",UI_Graph_Change,8,UI_Color_Orange,15,2,150,650);
    // 					Char_Write(&UI_State[2],"cover:ON ");
    // 					break;
    // 				}
    // 			}
    // 			Char_ReFresh(UI_State[2]);

    // 			switch(dbus_infomation.friction_state)
    // 			{
    // 				case friction_stop_sign:
    // 				{
    // 					Char_Draw(&UI_State[3],"s4",UI_Graph_Change,8,UI_Color_Pink,15,2,150,600);
    // 					Char_Write(&UI_State[3],"friction:OFF");
    // 					break;
    // 				}
    // 				case friction_start_sign:
    // 				{
    // 					Char_Draw(&UI_State[3],"s4",UI_Graph_Change,8,UI_Color_Pink,15,2,150,600);
    // 					Char_Write(&UI_State[3],"friction:ON ");
    // 					break;
    // 				}
    // 			}
    // 			Char_ReFresh(UI_State[3]);

    // 		}
    // 		else if(timer_count == PEFEREE_PERIOD_TX_C/2)
    // 		{
    // 			//功率值变化
    // 			Char_Draw(&UI_State[4],"s5",UI_Graph_Change,8,UI_Color_Green,18,2,720,240);
    // 			Char_Write(&UI_State[4],"Voltage:%dV",super_cap_info.cap_voltage_cap);
    // 			Char_ReFresh(UI_State[4]);
    // 		}

    // 	}
}

/**
 * @brief  判断各种ID，选择客户端ID
 * @param  void
 * @retval referee_info
 * @attention
 */
static void determine_ID(referee_info_t *_referee_info)
{
    // id小于7是红色,大于7是蓝色,0为红色，1为蓝色   #define Robot_Red 0    #define Robot_Blue 1
    _referee_info->referee_id.Robot_Color = _referee_info->GameRobotState.robot_id > 7 ? Robot_Blue : Robot_Red;
    _referee_info->referee_id.Robot_ID = _referee_info->GameRobotState.robot_id;
    _referee_info->referee_id.Cilent_ID = 0x0100 + _referee_info->referee_id.Robot_ID; // 计算客户端ID
    _referee_info->referee_id.Receiver_Robot_ID = 0x00;                                // 机器人车间通信时接收者的ID暂时为0
}
