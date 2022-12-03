#include "message_center.h"
#include "stdlib.h"
#include "string.h"

/* 消息初始化用 */
static char pname[MAX_EVENT_COUNT][MAX_EVENT_NAME_LEN + 1];
static char sname[MAX_EVENT_COUNT][MAX_EVENT_NAME_LEN + 1];
static void *p_ptr[MAX_EVENT_COUNT] = {NULL};
static void **s_pptr[MAX_EVENT_COUNT] = {NULL}; // 因为要修改指针,所以需要二重指针

/* ----------------------------------第三方指针传递版的实现----------------------------------- */

void MessageInit()
{
    // pub必须唯一,即消息名称不能重复,不得有多个pub发布相同消息名称
    // 对每一个subscriber,寻找相同消息名称的publisher,可能有多个sub从相同pub获取消息
    for (size_t i = 0; i < MAX_EVENT_COUNT; i++)
    {
        if (s_pptr[i] != NULL)
        {
            for (size_t j = 0; j < MAX_EVENT_COUNT; j++) // 遍历publisher
            {
                if (p_ptr[j] != NULL) // 不为空
                {
                    if (strcmp(sname[i], pname[j]) == 0) // 比较消息名是否一致
                    {
                        *s_pptr[i] = p_ptr[j]; // 将sub的指针指向pub的数据
                        break;
                    }
                }
                else // 到结尾,退出
                {
                    while (1)
                        ; // 如果你卡在这里,说明没有找到消息发布者!请确认消息名称是否键入错误
                }
            }
        }
        else // 说明已经遍历完所有的subs
        {
            break;
        }
    }
}

/* 传入数据地址 */
void PublisherRegister(char *name, void *data)
{
    static uint8_t idx;
    for (size_t i = 0; i < idx; i++)
    {
        if (strcmp(pname[i], name) == 0)
            while (1)
                ; // 运行至此说明pub的消息发布名称冲突
    }
    strcpy(pname[idx], name);
    p_ptr[idx++] = data;
}

/* 注意传入的是指针的地址,传参时使用&对数据指针取地址 */
void SubscribeEvent(char *name, void **data_ptr)
{
    static uint8_t idx;
    strcpy(sname[idx], name);
    s_pptr[idx++] = data_ptr;
}

/* ----------------------------------链表-队列版的实现----------------------------------- */

/* message_center是fake node,是方便链表编写的技巧,这样不需要处理链表头的特殊情况 */
static Publisher_t message_center = {
    .event_name = "Message_Manager",
    .first_subs = NULL,
    .next_event_node = NULL};

static void CheckName(char *name)
{
    if (strnlen(name, MAX_EVENT_NAME_LEN + 1) >= MAX_EVENT_NAME_LEN)
    {
        while (1)
            ; // 进入这里说明事件名超出长度限制
    }
}

static void CheckLen(uint8_t len1, uint8_t len2)
{
    if (len1 != len2)
    {
        while (1)
            ; // 相同事件的消息长度不同
    }
}

Subscriber_t *SubRegister(char *name, uint8_t data_len)
{
    CheckName(name);
    Publisher_t *node = &message_center; // 可以将message_center看作对消息管理器的抽象,它用于管理所有pub和sub
    while (node->next_event_node)        // 遍历链表,如果当前有发布者已经注册
    {
        node = node->next_event_node;            // 指向下一个发布者(发布者发布的事件)
        if (strcmp(name, node->event_name) == 0) // 如果事件名相同就订阅这个事件
        {
            CheckLen(data_len, node->data_len);
            // 创建新的订阅者结点,申请内存,注意要memset因为新空间不一定是空的,可能有之前留存的垃圾值
            Subscriber_t *ret = (Subscriber_t *)malloc(sizeof(Subscriber_t));
            memset(ret, 0, sizeof(Subscriber_t));
            // 对新建的Subscriber进行初始化
            ret->data_len = data_len; // 设定数据长度
            for (size_t i = 0; i < QUEUE_SIZE; i++)
            { // 给消息队列的每一个元素分配空间,queue里保存的实际上是数据执指针,这样可以兼容不同的数据长度
                ret->queue[i] = malloc(sizeof(data_len));
            }
            //如果之前没有订阅者,特殊处理一下
            if(node->first_subs==NULL) 
            {
                node->first_subs=ret;
                return ret;
            }
            // 遍历订阅者链表,直到到达尾部
            Subscriber_t *sub = node->first_subs; // iterator
            while (sub->next_subs_queue) // 遍历订阅了该事件的订阅者链表
            {
                sub = sub->next_subs_queue; // 移动到下一个订阅者,遇到空指针停下,说明到了链表尾部
            }                               
            sub->next_subs_queue = ret; // 把刚刚创建的订阅者接上
            return ret;
        }
        // 事件名不同,在下一轮循环访问下一个结点
    }
    // 遍历完,发现尚未注册事件(还没有发布者);那么创建一个事件,此时node是publisher链表的最后一个结点
    node->next_event_node = (Publisher_t *)malloc(sizeof(Publisher_t));
    memset(node->next_event_node, 0, sizeof(Publisher_t));
    strcpy(node->next_event_node->event_name, name);
    node->next_event_node->data_len = data_len;
    // 同之前,创建subscriber作为新事件的第一个订阅者
    Subscriber_t *ret = (Subscriber_t *)malloc(sizeof(Subscriber_t));
    memset(ret, 0, sizeof(Subscriber_t));
    ret->data_len = data_len;
    for (size_t i = 0; i < QUEUE_SIZE; i++)
    { // 给消息队列分配空间
        ret->queue[i] = malloc(sizeof(data_len));
    }
    // 新建的订阅者是该发布者的第一个订阅者,发布者会通过这个指针顺序访问所有订阅者
    node->next_event_node->first_subs = ret;
    return ret;
}

Publisher_t *PubRegister(char *name, uint8_t data_len)
{
    CheckName(name);
    Publisher_t *node = &message_center;
    while (node->next_event_node) // message_center会直接跳过,不需要特殊处理,这被称作dumb_head(编程技巧)
    {
        node = node->next_event_node;            // 切换到下一个发布者(事件)结点
        if (strcmp(node->event_name, name) == 0) // 如果已经注册了相同的事件,直接返回结点指针
        {
            CheckLen(data_len, node->data_len);
            return node;
        }
    } // 遍历完发现尚未创建name对应的事件
    // 在链表尾部创建新的事件并初始化
    node->next_event_node = (Publisher_t *)malloc(sizeof(Publisher_t));
    memset(node->next_event_node, 0, sizeof(Publisher_t));
    node->next_event_node->data_len = data_len;
    strcpy(node->next_event_node->event_name, name);
    return node->next_event_node;
}

/* 如果队列为空,会返回0;成功获取数据,返回1;后续可以做更多的修改,比如剩余消息数目等 */
uint8_t SubGetMessage(Subscriber_t *sub, void *data_ptr)
{
    if (sub->temp_size == 0)
    {
        return 0;
    }
    memcpy(data_ptr, sub->queue[sub->front_idx], sub->data_len);
    sub->front_idx = (sub->front_idx++) % QUEUE_SIZE; // 队列头索引增加
    sub->temp_size--;                                 // pop一个数据,长度减1
    return 1;
}

void PubPushMessage(Publisher_t *pub, void *data_ptr)
{
    Subscriber_t *iter = pub->first_subs; // iter作为订阅者指针,遍历订阅该事件的所有订阅者;如果为空说明遍历结束
    while (iter)                          // 遍历订阅了当前事件的所有订阅者,依次填入最新消息
    {
        if (iter->temp_size == QUEUE_SIZE) // 如果队列已满,则需要删除最老的数据(头部),再填入
        {                                  // 队列头索引前移动,相当于抛弃前一个位置,被抛弃的位置稍后会被写入新的数据
            iter->front_idx = (iter->front_idx + 1) % QUEUE_SIZE;
            iter->temp_size--; // 相当于出队,size-1
        }
        // 将Pub的数据复制到队列的尾部(最新)
        memcpy(iter->queue[iter->back_idx], data_ptr, pub->data_len);
        iter->back_idx = (iter->back_idx + 1) % QUEUE_SIZE; // 队列尾部前移
        iter->temp_size++;                                  // 入队,size+1

        iter = iter->next_subs_queue; // 访问下一个订阅者
    }
}