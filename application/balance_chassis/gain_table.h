/* 平衡底盘lqr反馈增益和腿长的关系表,可以选择查找精度和插值 */

#pragma once
#include "stdint.h"
#include "stm32f407xx.h"
#include "arm_math.h"
#include "math.h"

#define GAIN_TABLE_SIZE 100 // 增益表大小

// K 2x6,6个状态变量2个输出(Tp关节电机和T驱动轮电机)
static float leglen2gain [GAIN_TABLE_SIZE][2][6] = {0};

static interpolation_flag = 0; // 插值方式:1 线性插值 0 关闭插值

void EnalbeInterpolation(void)
{
    interpolation_flag = 1;
}

/* 默认关闭插值,向下取整 */
float* LookUpKgain(float leg_length)
{
    
}