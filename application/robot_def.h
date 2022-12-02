/**
 * @file robot_def.h
 * @author NeoZeng neozng1@hnu.edu.cn
 * @author Even
 * @version 0.1
 * @date 2022-12-02
 * 
 * @copyright Copyright (c) HNU YueLu EC 2022 all rights reserved
 * 
 */

#ifndef ROBOT_DEF_H
#define ROBOT_DEF_H

#include "ins_task.h"
#include "master_process.h"
#include "stdint-gcc.h"

/* 开发板类型定义,烧录时注意不要弄错对应功能;修改定义后需要重新编译 */
/* 只能存在一个宏定义! */
#define ONE_BOARD // 单板控制整车
// #define CHASSIS_BOARD //底盘板
// #define GIMBAL_BOARD  //云台板

/* 重要参数定义,注意根据不同机器人进行修改 */
#define YAW_MID_ECD

#if (defined(ONE_BOARD) && defined(CHASSIS_BOARD)) || \
    (defined(ONE_BOARD) && defined(GIMBAL_BOARD)) ||  \
    (defined(CHASSIS_BOARD) && defined(GIMBAL_BOARD))
#error Conflict board definition! You can only define one type.
#endif // 检查是否出现定义冲突

#pragma pack(1) // 压缩结构体,取消字节对齐

/* -------------------------基本控制模式和数据类型定义-------------------------*/
/**
 * @brief 这些枚举类型和结构体会作为CMD控制数据和各应用的反馈数据的一部分
 *
 */

// 应用状态
typedef enum
{
    APP_OFFLINE,
    APP_ONLINE,
    APP_ERROR,
} App_Status_e;

// 底盘模式设置
typedef enum
{
    CHASSIS_ZERO_FORCE,        // 电流零输入
    CHASSIS_ROTATE,            // 小陀螺模式
    CHASSIS_NO_FOLLOW,         // 不跟随，允许全向平移
    CHASSIS_FOLLOW_GIMBAL_YAW, // 跟随模式，底盘叠加角度环控制
} chassis_mode_e;

// 云台模式设置
typedef enum
{
    GIMBAL_ZERO_FORCE,  // 电流零输入
    GIMBAL_FREE_MODE,   // 云台自由运动模式，反馈值为电机total_angle
    GIMBAL_GYRO_MODE,   // 云台陀螺仪反馈模式，反馈值为陀螺仪pitch,total_yaw_angle
    GIMBAL_VISUAL_MODE, // 视觉模式，反馈值为陀螺仪，输入为视觉数据
} gimbal_mode_e;

// 发射模式设置
typedef enum
{
    FRICTION_OFF, // 摩擦轮关闭
    FRICTION_ON,  // 摩擦轮开启
} shoot_mode_e;

typedef enum
{
    LID_CLOSE, // 弹舱盖打开
    LID_ON,    // 弹舱盖关闭
} lid_mode_e;

typedef enum
{
    LOAD_STOP,      // 停止发射
    LOAD_REVERSE,   // 反转
    LOAD_1_BULLET,  // 单发
    LOAD_3_BULLET,  // 三发
    LOAD_BURSTFIRE, // 连发
} loader_mode_e;

// 功率限制,从裁判系统获取
typedef struct
{ // 功率控制
    uint8_t power_limit;
    uint8_t buffer_power_rest;
} Chassis_Power_Data_s;




/* ------------CMD模块之间的控制数据传递,应当由gimbal_cmd和chassis_cmd订阅------------ */
/**
 * @brief 对于双板通信的情况,两个CMD模块各有一个can comm.
 *
 */

// gimbal_cmd发布的底盘控制数据,由chassis_cmd订阅
//  注意这里没有功率限制,为了兼容双板模式,先让chassis_cmd订阅,后者会添加功率信息再发布
typedef struct
{
    // 速度控制
    float vx;           // 前进方向速度
    float vy;           // 横移方向速度
    float wz;           // 旋转速度
    float offset_angle; // 底盘和归中位置的夹角

    chassis_mode_e chassis_mode;
} Gimbal2Chassis_Data_s;

// chassis_cmd发布的裁判系统和底盘信息相关的数据,由gimbal_cmd订阅
typedef struct
{
    float chassis_real_rotate_wz;
    uint8_t rest_heat;
    Bullet_Speed_e bullet_speed;
    Enemy_Color_e enemy_color; // 0 for blue, 1 for red
} Chassis2Gimbal_Data_s;




/* ----------------CMD应用发布的控制数据,应当由gimbal/chassis/shoot订阅---------------- */
/**
 * @brief 对于双板情况,遥控器和pc在云台,裁判系统在底盘
 *
 */
// chassis_cmd根据raw ctrl data添加功率限制后发布的底盘控制数据,由chassis订阅
typedef struct
{
    Chassis_Power_Data_s chassis_power;
    Gimbal2Chassis_Data_s chassis_cmd;
} Chassis_Ctrl_Cmd_s;

// gimbal_cmd发布的云台控制数据,由gimbal订阅
typedef struct
{ // 云台角度控制
    float yaw;
    float pitch;
    float chassis_rotate_wz;

    gimbal_mode_e gimbal_mode;
} Gimbal_Ctrl_Cmd_s;

// gimba_cmd发布的发射控制数据,由shoot订阅
typedef struct
{ // 发射弹速控制
    loader_mode_e load_mode;
    lid_mode_e lid_mode;
    shoot_mode_e shoot_mode;
    Bullet_Speed_e bullet_speed;
    uint8_t rest_heat;
} Shoot_Ctrl_Cmd_s;




/* ----------------gimbal/shoot/chassis发布的反馈数据----------------*/
/**
 * @brief 由cmd订阅,其他应用也可以根据需要获取.
 *
 */

typedef struct
{
#ifdef CHASSIS_BOARD
    attitude_t chassis_imu_data;
#endif // CHASSIS_BOARD
} Chassis_Upload_Data_s;

typedef struct
{
    attitude_t gimbal_imu_data;
} Gimbal_Upload_Data_s;

typedef struct
{
    // code to go here
    // ...
} Shoot_Upload_Data_s;

#pragma pack() // 开启字节对齐

#endif // !ROBOT_DEF_H