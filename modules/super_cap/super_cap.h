/*
 * @Descripttion:
 * @version:
 * @Author: Chenfu
 * @Date: 2022-12-02 21:32:47
 * @LastEditTime: 2022-12-05 15:25:46
 */
#ifndef SUPER_CAP_H
#define SUPER_CAP_H

#include "bsp_can.h"

typedef struct
{
    uint16_t vol;
    uint16_t current;
    uint16_t power;
} SuperCap_Msg_s;

#pragma pack(1)
typedef struct
{
    CANInstance *can_ins;
    SuperCap_Msg_s cap_msg;
} SuperCapInstance;
#pragma pack()

typedef struct
{
    CAN_Init_Config_s can_config;
    uint8_t send_data_len;
    uint8_t recv_data_len;
} SuperCap_Init_Config_s;

SuperCapInstance *SuperCapInit(SuperCap_Init_Config_s *supercap_config);

void SuperCapSend(SuperCapInstance *instance, uint8_t *data);

#endif // !SUPER_CAP_Hd
