#pragma once

#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

#include "ins_task.h"
#include "motor_task.h"
#include "referee_task.h"
#include "master_process.h"
#include "daemon.h"
#include "HT04.h"

osThreadId insTaskHandle;
osThreadId robotTaskHandle;
osThreadId motorTaskHandle;
osThreadId daemonTaskHandle;
osThreadId uiTaskHandle;

void StartINSTASK(void const *argument);
void StartMOTORTASK(void const *argument);
void StartDAEMONTASK(void const *argument);
void StartROBOTTASK(void const *argument);
void StartUITASK(void const *argument);

/**
 * @brief 初始化机器人任务,所有持续运行的任务都在这里初始化
 *
 */
void OSTaskInit()
{
    osThreadDef(instask, StartINSTASK, osPriorityAboveNormal, 0, 1024);
    insTaskHandle = osThreadCreate(osThread(instask), NULL); // 由于是阻塞读取传感器,为姿态解算设置较高优先级,确保以1khz的频率执行
    // 后续修改为读取传感器数据准备好的中断处理,

    osThreadDef(motortask, StartMOTORTASK, osPriorityNormal, 0, 256);
    motorTaskHandle = osThreadCreate(osThread(motortask), NULL);

    osThreadDef(daemontask, StartDAEMONTASK, osPriorityNormal, 0, 128);
    daemonTaskHandle = osThreadCreate(osThread(daemontask), NULL);

    osThreadDef(robottask, StartROBOTTASK, osPriorityNormal, 0, 1024);
    robotTaskHandle = osThreadCreate(osThread(robottask), NULL);

    osThreadDef(uitask, StartUITASK, osPriorityNormal, 0, 512);
    uiTaskHandle = osThreadCreate(osThread(uitask), NULL);

    // 若使用HT电机则取消本行注释,该接口会为注册了的电机设备创建线程
    // HTMotorControlInit();
}

void StartINSTASK(void const *argument)
{
    static float ins_start, ins_dt;
    INS_Init(); // 确保BMI088被正确初始化.
    while (1)
    {
        // 1kHz
        ins_start = DWT_GetTimeline_ms();
        INS_Task();
        ins_dt = DWT_GetTimeline_ms() - ins_start;
        VisionSend(); // 解算完成后发送视觉数据,但是当前的实现不太优雅,后续若添加硬件触发需要重新考虑结构的组织
        osDelay(1);
    }
}

void StartMOTORTASK(void const *argument)
{
    static float motor_dt, motor_start;
    while (1)
    {
        motor_start = DWT_GetTimeline_ms();
        MotorControlTask();
        motor_dt = DWT_GetTimeline_ms() - motor_start;
        osDelay(1);
    }
}

void StartDAEMONTASK(void const *argument)
{
    while (1)
    {
        // 100Hz
        DaemonTask();
        osDelay(10);
    }
}

void StartROBOTTASK(void const *argument)
{
    static float robot_dt, robot_start;
    // 200Hz-500Hz,若有额外的控制任务如平衡步兵可能需要提升至1kHz
    while (1)
    {
        robot_start = DWT_GetTimeline_ms();
        RobotTask();
        robot_dt = DWT_GetTimeline_ms() - robot_start;
        osDelay(5);
    }
}

void StartUITASK(void const *argument)
{
    My_UI_init();
    while (1)
    {
        Referee_Interactive_task(); // 每次给裁判系统发送完一包数据后,挂起一次,防止卡在裁判系统发送中,详见Referee_Interactive_task函数的refereeSend();
        osDelay(1);                 // 即使没有任何UI需要刷新,也挂起一次,防止卡在UITask中无法切换
    }
}
