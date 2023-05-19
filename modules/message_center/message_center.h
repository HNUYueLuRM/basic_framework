/**
 * @file message_center.h
 * @author NeoZeng neozng1@hnu.edu.cn
 * @brief 这是一个伪pubsub机制,仅对应用之间的通信进行了隔离
 * @version 0.1
 * @date 2022-11-30
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef PUBSUB_H
#define PUBSUB_H

#include "stdint.h"

#define MAX_TOPIC_NAME_LEN 32 // 最大的话题名长度,每个话题都有字符串来命名
#define MAX_TOPIC_COUNT 12    // 最多支持的话题数量
#define QUEUE_SIZE 1

typedef struct mqt
{
    /* 用数组模拟FIFO队列 */
    void *queue[QUEUE_SIZE];
    uint8_t data_len;
    uint8_t front_idx;
    uint8_t back_idx;
    uint8_t temp_size; // 当前队列长度

    /* 指向下一个订阅了相同的话题的订阅者的指针 */
    struct mqt *next_subs_queue; // 使得发布者可以通过链表访问所有订阅了相同话题的订阅者
} Subscriber_t;

/**
 * @brief 发布者类型.每个发布者拥有发布者实例,并且可以通过链表访问所有订阅了自己发布的话题的订阅者
 *
 */
typedef struct ent
{
    /* 话题名称 */
    char topic_name[MAX_TOPIC_NAME_LEN + 1]; // 1个字节用于存放字符串结束符 '\0'
    uint8_t data_len;                        // 该话题的数据长度
    /* 指向第一个订阅了该话题的订阅者,通过链表访问所有订阅者 */
    Subscriber_t *first_subs;
    /* 指向下一个Publisher的指针 */
    struct ent *next_topic_node;
    uint8_t pub_registered_flag; // 用于标记该发布者是否已经注册
} Publisher_t;

/**
 * @brief 订阅name的话题消息
 *
 * @param name 话题名称
 * @param data_len 消息长度,通过sizeof()获取
 * @return Subscriber_t* 返回订阅者实例
 */
Subscriber_t *SubRegister(char *name, uint8_t data_len);

/**
 * @brief 注册成为消息发布者
 *
 * @param name 发布者发布的话题名称(话题)
 * @return Publisher_t* 返回发布者实例
 */
Publisher_t *PubRegister(char *name, uint8_t data_len);

/**
 * @brief 获取消息
 *
 * @param sub 订阅者实例指针
 * @param data_ptr 数据指针,接收的消息将会放到此处
 * @return uint8_t 返回值为0说明没有新的消息(消息队列为空),为1说明获取到了新的消息
 */
uint8_t SubGetMessage(Subscriber_t *sub, void *data_ptr);

/**
 * @brief 发布者给所有订阅了话题的订阅者推送消息
 *
 * @param pub 发布者实例指针
 * @param data_ptr 指向要发布的数据的指针
 * @return uint8_t 新消息成功推送给几个订阅者
 */
uint8_t PubPushMessage(Publisher_t *pub, void *data_ptr);

#endif // !PUBSUB_H