/**
 * @file message_center.h
 * @author NeoZeng neozng1@hnu.edu.cn
 * @brief 这是一个伪pubsub机制,仅对应用之间的通信进行了隔离
 * @todo 实现基于链表-队列的pubsub机制
 * @version 0.1
 * @date 2022-11-30
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef PUBSUB_H
#define PUBSUB_H

#include "stdlib.h"
#include "stdint-gcc.h"


#define MAX_EVENT_NAME_LEN  32  //最大的事件名长度,每个事件都有字符串来命名
#define MAX_EVENT_COUNT 12      //最多支持的事件数量


/**
 * @brief 在所有应用初始化结束之后调用,当作app的"回调函数"
 * 
 */
void MessageInit();

/**
 * @brief 注册成为消息发布者
 * 
 * @param name 消息标识名,注意不要超过MAX_EVENT_NAME_LEN
 * @param data 发布者的数据地址
 */
void PublisherRegister(char* name,void* data);

/**
 * @brief 订阅消息,成为消息订阅者
 * 
 * @param name 消息标识名
 * @param data 保存数据的指针的地址,注意这是一个二级指针,传入的时候对数据指针取地址(&)
 */
void SubscribeEvent(char* name,void** data);

#endif // !PUBSUB_H




/* 以下是队列版的pubsub,TODO */
