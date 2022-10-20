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

extern DWT_Time_t SysTime;

void DWT_Init(uint32_t CPU_Freq_mHz);
float DWT_GetDeltaT(uint32_t *cnt_last);
double DWT_GetDeltaT64(uint32_t *cnt_last);
float DWT_GetTimeline_s(void);
float DWT_GetTimeline_ms(void);
uint64_t DWT_GetTimeline_us(void);
void DWT_Delay(float Delay);
void DWT_SysTimeUpdate(void);



#endif /* BSP_DWT_H_ */
