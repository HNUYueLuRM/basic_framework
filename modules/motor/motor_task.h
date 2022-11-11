/**
 * @file motor_task.h
 * @author neozng
 * @brief  对所有电机,舵机等控制任务的进一步封装,MotorControlTask()将在操作系统中按一定频率调用
 * @version beta
 * @date 2022-11-01
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef MOTOR_TASK_H
#define MOTOR_TASK_H

#include "LK9025.h"
#include "HT04.h"
#include "dji_motor.h"

// 舵机控制任务的频率设定为20Hz或更低
// 开关式的舵机控制不应该放在该函数中
/**
 * @brief 
 * 
 */
void MotorControlTask();

#endif // !MOTOR_TASK_H

