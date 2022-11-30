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

#include "stdint-gcc.h"


#define MAX_EVENT_NAME_LEN  32  //最大的事件名长度,每个事件都有字符串来命名
#define MAX_EVENT_COUNT 12      //最多支持的事件数量
#define QUEUE_SIZE 1


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

typedef struct mqt
{
    /* 用数组模拟FIFO队列 */
    void* queue[QUEUE_SIZE];
    uint8_t data_len;
    uint8_t front_idx;
    uint8_t back_idx;
    uint8_t temp_size; // 当前队列长度

    /* 指向下一个订阅了相同的事件的订阅者的指针 */
    struct mqt* next_subs_queue;
} Subscriber_t;

/**
 * @brief 发布者类型.每个发布者拥有发布者实例,这个实例
 * 
 */
typedef struct ent  
{   
    /* 事件名称 */
    char event_name[MAX_EVENT_NAME_LEN];
    uint8_t data_len;
    /* 指向第一个订阅了该事件的订阅者,通过链表访问所有订阅者 */
    Subscriber_t* first_subs;
    /* 指向下一个Publisher的指针 */
    struct ent* next_event_node;

} Publisher_t;

/**
 * @brief 订阅name的事件消息
 * 
 * @param name 事件名称
 * @param data_len 消息长度,通过sizeof()获取
 * @return Subscriber_t* 返回订阅者实例
 */
Subscriber_t* SubRegister(char* name,uint8_t data_len);

/**
 * @brief 注册成为消息发布者
 * 
 * @param name 发布者发布的话题名称(事件)
 * @return Publisher_t* 返回发布者实例
 */
Publisher_t* PubRegister(char* name,uint8_t data_len);

/**
 * @brief 获取消息
 * 
 * @param sub 订阅者实例指针
 * @param data_ptr 数据指针,接收的消息将会放到此处
 * @return uint8_t 返回值为0说明没有新的消息(消息队列为空),为1说明获取到了新的消息
 */
uint8_t SubGetMessage(Subscriber_t* sub,void* data_ptr);

/**
 * @brief 发布者给所有订阅了事件的订阅者推送消息
 * 
 * @param pub 发布者实例指针
 * @param data_ptr 指向要发布的数据的指针
 */
void PubPushMessage(Publisher_t* pub,void* data_ptr);


