#ifndef _CHASSIS_BOARD_CMD_H_
#define _CHASSIS_BOARD_CMD_H_
#include <buzzer.h>
#include <can_recv.h>
#include <can_send.h>
#include <robot_def.h>
#include <stdint.h>
class chassis_board_cmd {
   private:
    Robot_mode robot_mode;                //机器人状态
    cmd_chassis chassis_control;          //发送给底盘的控制量
    upload_chassis* chassis_upload_data;  //底盘模块回传数据
    chassis_board_send board_send;        //发送给云台主控的数据
    can_send<chassis_board_send> sender;  //板间通信发送
    can_recv<gimbal_board_send> recver;   //板间通信接收
    gimbal_board_send* board_recv;        //板间通信接收数据指针
    void* referee;                        //裁判系统（尚未完成）
    buzzer board_buzzer;                  //蜂鸣器
    uint8_t robot_ready;                  //底盘板是否准备好
   public:
    chassis_board_cmd();
    void update();
};

#endif