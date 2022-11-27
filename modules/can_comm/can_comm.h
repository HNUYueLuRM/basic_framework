/**
 * @file can_comm.h
 * @author Neo neozng1@hnu.edu.cn
 * @brief  用于多机CAN通信的收发模块
 * @version 0.1
 * @date 2022-11-27
 * 
 * @copyright Copyright (c) 2022 HNUYueLu EC all rights reserved
 * 
 */
#ifndef CAN_COMM_H
#define CAN_COMM_H


#include "bsp_can.h"

#define MX_CAN_COMM_COUNT 4 //注意均衡负载,一条总线上不要挂载过多的外设

#define CAN_COMM_MAX_BUFFSIZE 68 // 最大发送/接收字节数
#define CAN_COMM_HEADER 's'
#define CAN_COMM_TAIL   'e'
#define CAN_COMM_OFFSET_BYTES 4 // 's'+datalen+'e'+crc8

typedef struct 
{
    can_instance can_ins;
    /* 发送部分 */
    uint8_t send_data_len;
    uint8_t send_buf_len;
    uint8_t raw_sendbuf[CAN_COMM_MAX_BUFFSIZE+CAN_COMM_OFFSET_BYTES]; //额外4个bytes保存帧头帧尾和校验和
    /* 接收部分 */
    uint8_t recv_data_len;
    uint8_t recv_buf_len;
    uint8_t raw_recvbuf[CAN_COMM_MAX_BUFFSIZE+CAN_COMM_OFFSET_BYTES]; //额外4个bytes保存帧头帧尾和校验和
    uint8_t unpacked_recv_data[CAN_COMM_MAX_BUFFSIZE]; 
    /* 接收和更新标志位*/
    uint8_t recv_state;
    uint8_t cur_recv_len;
    uint8_t update_flag;
} CANCommInstance;

typedef struct
{
    uint8_t send_data_len;
    uint8_t recv_data_len;
    can_instance_config_s can_config;
}CANComm_Init_Config_s;

/**
 * @brief 
 * 
 * @param config 
 * @return CANCommInstance* 
 */
CANCommInstance* CANCommInit(CANComm_Init_Config_s config);

/**
 * @brief 发送数据
 * 
 * @param instance can comm实例
 * @param data 注意此地址的有效数据长度需要和初始化时传入的datalen相同
 */
void CANCommSend(CANCommInstance* instance,uint8_t* data);

/**
 * @brief 获取CAN COMM接收的数据,需要自己使用强制类型转换将返回的void指针转换成指定类型
 * 
 * @return void* 返回的数据指针
 * @attention 注意如果希望直接通过转换指针访问数据,如果数据是union或struct,要检查是否使用了pack(n)
 *            CAN COMM接收到的数据可以看作是pack(1)之后的,是连续存放的
 */
void* CANCommGet(CANCommInstance* instance);


#endif // !CAN_COMM_H