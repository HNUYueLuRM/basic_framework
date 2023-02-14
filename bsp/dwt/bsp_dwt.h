/**
 ******************************************************************************
 * @file	bsp_dwt.h
 * @author  Wang Hongxi
 * @version V1.1.0
 * @date    2022/3/8
 * @brief
 ******************************************************************************
 * @attention
 *
 ******************************************************************************
 */
#ifndef _BSP_DWT_H
#define _BSP_DWT_H

#include "main.h"
#include "stdint.h"

typedef struct
{
    uint32_t s;
    uint16_t ms;
    uint16_t us;
} DWT_Time_t;


/**
 * @brief 初始化DWT,传入参数为CPU频率,单位MHz
 * 
 * @param CPU_Freq_mHz c板为168MHz,A板为180MHz
 */
void DWT_Init(uint32_t CPU_Freq_mHz);

/**
 * @brief 获取两次调用之间的时间间隔,单位为秒/s
 * 
 * @param cnt_last 上一次调用的时间戳
 * @return float 时间间隔,单位为秒/s
 */
float DWT_GetDeltaT(uint32_t *cnt_last);

/**
 * @brief 获取两次调用之间的时间间隔,单位为秒/s,高精度
 * 
 * @param cnt_last 上一次调用的时间戳
 * @return double 时间间隔,单位为秒/s
 */
double DWT_GetDeltaT64(uint32_t *cnt_last);

/**
 * @brief 获取当前时间,单位为秒/s,即初始化后的时间
 * 
 * @return float 时间轴
 */
float DWT_GetTimeline_s(void);

/**
 * @brief 获取当前时间,单位为毫秒/ms,即初始化后的时间
 * 
 * @return float 
 */
float DWT_GetTimeline_ms(void);

/**
 * @brief 获取当前时间,单位为微秒/us,即初始化后的时间
 * 
 * @return uint64_t 
 */
uint64_t DWT_GetTimeline_us(void);

/**
 * @brief DWT延时函数,单位为秒/s
 * @attention 该函数不受中断是否开启的影响,可以在临界区和关闭中断时使用
 * @note 禁止在__disable_irq()和__enable_irq()之间使用HAL_Delay()函数,应使用本函数
 * 
 * @param Delay 延时时间,单位为秒/s
 */
void DWT_Delay(float Delay);

/**
 * @brief DWT更新时间轴函数,会被三个timeline函数调用
 * @attention 如果长时间不调用timeline函数,则需要手动调用该函数更新时间轴,否则CYCCNT溢出后定时和时间轴不准确
 */
void DWT_SysTimeUpdate(void);



#endif /* BSP_DWT_H_ */
