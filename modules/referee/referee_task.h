#ifndef REFEREE_H
#define REFEREE_H

#include "rm_referee.h"
#include "robot_def.h"

/**
 * @brief 初始化裁判系统交互任务(UI和多机通信)
 *
 */
void Referee_Interactive_init();

/**
 * @brief 裁判系统交互任务(UI和多机通信)
 *
 */
void Referee_Interactive_task();

#endif // REFEREE_H
