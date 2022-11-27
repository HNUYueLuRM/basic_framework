#ifndef _GIMBAL_ROBOT_CMD_H
#define _GIMBAL_ROBOT_CMD_H
#include <DT7_DR16.h>
#include <buzzer.h>
#include <can_pc.h>
#include <pub_sub.h>
#include <robot_def.h>
class gimbal_board_cmd {
   private:
    gimbal_board_send board_send;  //需要发送给下板的板间通信数据
    cmd_gimbal gimbal_control;     //发送给云台的控制量
    cmd_shoot shoot_control;       //发送给发射机构的控制量

    can_send<gimbal_board_send> sender;   //板间通信发送
    can_recv<chassis_board_send> recver;  //板间通信接收
    chassis_board_send* board_recv;       //板间通信接收数据指针
    canpc pc;                             //小电脑视觉数据
    dt7Remote remote;                     //遥控器
    Robot_mode robot_mode;                //机器人模式
    uint8_t robot_ready;                  //机器人准备好标志位
    AutoAim_mode autoaim_mode;            //机器人自瞄模式
    buzzer board_buzzer;                  //蜂鸣器
    upload_gimbal* gimbal_upload_data;    //云台模块回传的数据

    void stop_mode_update();       //机器人停止模式更新函数
    void remote_mode_update();     //机器人遥控器模式更新函数
    void mouse_key_mode_update();  //机器人键鼠模式更新函数
    void send_cmd_and_data();      //发布指令和板间通信

   public:
    gimbal_board_cmd();
    void update();
};

#endif