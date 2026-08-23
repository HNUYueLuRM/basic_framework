/**
 * @file referee_vt_task.c
 * @author LHXY (cjy719@hnu.edu.cn)
 * @brief 
 * @version 0.1
 * @date 2026-03-27
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include "referee_vt_task.h"
#include "robot_def.h"
#include "referee_vt.h"
#include "referee_custom.h"
#include "string.h"
#include "message_center.h"
#include "cmsis_os.h"

static referee_vt_info_t *vt_recv_info; // 接收到的裁判系统数据

referee_vt_info_t *VTTaskInit(UART_HandleTypeDef *vt_usart_handle)
{
    vt_recv_info = RefereeVtInit(vt_usart_handle); // 初始化裁判系统的串口,并返回裁判系统反馈数据指针
    vt_recv_info->init_flag = 1;
    return vt_recv_info;
}

//和图传链路发送有关的东西都删掉了，原代码过于混乱，等有发送需求的时候再重新写吧