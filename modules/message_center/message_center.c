#include "message_center.h"
#include "stdlib.h"
#include "string.h"

/* 消息初始化用 */
static char pname[MAX_EVENT_COUNT][MAX_EVENT_NAME_LEN+1];
static char sname[MAX_EVENT_COUNT][MAX_EVENT_NAME_LEN+1];
static void* p_ptr[MAX_EVENT_COUNT]={NULL};
static void** s_pptr[MAX_EVENT_COUNT]={NULL}; // 因为要修改指针,所以需要二重指针

void MessageInit()
{
    // pub必须唯一,即消息名称不能重复,不得有多个pub发布相同消息名称
    // 对每一个subscriber,寻找相同消息名称的publisher,可能有多个sub从相同pub获取消息
    for (size_t i = 0; i < MAX_EVENT_COUNT; i++)
    {
        if(s_pptr[i]!=NULL)
        {
            for (size_t j = 0; j < MAX_EVENT_COUNT; j++) //遍历publisher
            {
                if(p_ptr[j]!=NULL) //不为空
                {
                    if(strcmp(sname[i],pname[j])==0) //比较消息名是否一致
                    {
                        *s_pptr[i]=p_ptr[j]; // 将sub的指针指向pub的数据
                        break;
                    }
                }
                else //到结尾,退出
                {
                    while(1); //如果你卡在这里,说明没有找到消息发布者!请确认消息名称是否键入错误
                }
            }
        }
        else //说明已经遍历完所有的subs
        {
            break;
        }
    }
}

/* 传入数据地址 */
void PublisherRegister(char* name,void* data)
{
    static uint8_t idx;
    for (size_t i = 0; i < idx; i++)
    {
        if(strcmp(pname[i],name)==0)
            while(1); //运行至此说明pub的消息发布名称冲突
    }
    strcpy(pname[idx],name);
    p_ptr[idx++]=data;
}

/* 注意传入的是指针的地址,传参时使用&对数据指针取地址 */
void SubscribeEvent(char* name,void** data_ptr)
{
    static uint8_t idx;
    strcpy(sname[idx],name);
    s_pptr[idx++]=data_ptr;
}






/* 以下是队列版的pubsub,TODO */

/* message_center是fake node,方便链表编写的技巧,这样不需要处理链表头的特殊情况 */
static Publisher_t message_center={
    .event_name="Message_Manager",
    .first_subs=NULL,
    .next_event_node=NULL
};

Subscriber_t* SubRegister(char* name,uint8_t data_len)
{
    Publisher_t* node=&message_center;
    while(node->next_event_node) // 遍历链表
    {
        node=node->next_event_node;
        if(strcmp(name,node->event_name)==0) //如果事件名相同就订阅这个事件
        {
            Subscriber_t* sub=node->first_subs; //指向第一个事件
            while (sub->next_subs_queue) //遍历订阅了该事件的链表
            {
                sub=sub->next_subs_queue;
            } //遇到空指针停下,创建新的订阅节点
            Subscriber_t* ret=(Subscriber_t*)malloc(sizeof(Subscriber_t));
            memset(ret,0,sizeof(Subscriber_t));
            ret->data_len=data_len;
            for (size_t i = 0; i < QUEUE_SIZE; i++)
            {   //给消息队列分配空间,queue里保存的实际上是数据执指针,这样可以兼容不同的数据长度
                ret->queue[i]=malloc(sizeof(data_len)); 
            }
            sub->next_subs_queue=ret; //接到尾部,延长该订阅该事件的subs链表
            return ret;
        }
        // 时间名不同,在下一轮循环访问下一个结点
    } 
    //遍历完,发现尚未注册事件,那么创建一个事件.此时node是publisher链表的最后一个结点
    node->next_event_node=(Publisher_t*)malloc(sizeof(Publisher_t)); 
    strcpy(node->next_event_node->event_name,name);
    node->next_event_node->next_event_node=NULL; //新publisher的下一个节点置为NULL
    //创建subscriber作为新事件的第一个订阅者
    Subscriber_t* ret=(Subscriber_t*)malloc(sizeof(Subscriber_t));
    memset(ret,0,sizeof(Subscriber_t));
    ret->data_len=data_len;
    for (size_t i = 0; i < QUEUE_SIZE; i++)
    {   //给消息队列分配空间
        ret->queue[i]=malloc(sizeof(data_len)); 
    }
    node->next_event_node->first_subs=ret;
    return ret;
    
}

Publisher_t* PubRegister(char* name,uint8_t data_len)
{
    Publisher_t* node=&message_center;
    while(node->next_event_node) //message_center会直接跳过,不需要特殊处理
    {
        node=node->next_event_node;
        if(strcmp(node->event_name,name)==0)
        {
            return node;
        }
    } //遍历完发现尚未创建name对应的事件
    node->next_event_node=(Publisher_t*)malloc(sizeof(Publisher_t));
    memset(node->next_event_node,0,sizeof(Publisher_t)); 
    node->next_event_node->data_len=data_len;
    strcpy(node->next_event_node->event_name,name);
    return node->next_event_node;
}

/* 如果队列为空,会返回0;成功获取数据,返回1 */
uint8_t SubGetMessage(Subscriber_t* sub,void* data_ptr)
{
    if(sub->temp_size==0)
    {
        return 0;
    }
    memcpy(sub->queue[sub->front_idx],data_ptr,sub->data_len);
    sub->front_idx=(sub->front_idx++)%QUEUE_SIZE;
    sub->temp_size--;
    return 1;
}

void PubPushMessage(Publisher_t* pub,void* data_ptr)
{
    static Subscriber_t* node;
    node->next_subs_queue=pub->first_subs;
    while (node->next_subs_queue) //遍历订阅了当前事件的所有订阅者,依次填入最新消息
    {
        node=node->next_subs_queue;
        if(node->temp_size==QUEUE_SIZE) //如果队列已满,则需要删除最老的数据(头部),再填入
        {   //头索引增加,相当于移动到新一个位置上
            node->front_idx=(node->front_idx+1)%QUEUE_SIZE;
            node->temp_size--; //相当于出队,size-1
        }
        // 将Pub的数据复制到队列的尾部(最新)
        memcpy(node->queue[node->back_idx],data_ptr,pub->data_len);
        node->back_idx=(node->back_idx+1)%QUEUE_SIZE; //队列尾部前移
        node->temp_size++; //入队,size+1
    }
}