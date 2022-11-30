#include "message_center.h"
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
            for (size_t j = 0; j < MAX_EVENT_COUNT; j++) //遍历publisher
            {
                if(p_ptr[j]!=NULL) //不为空
                    if(strcmp(&sname[i],pname[j])==0) //比较消息名是否一致
                    {volatile size_t ss=strlen(sname[i]);
                    volatile size_t sss=strlen(pname[j]);
                        *s_pptr[i]=p_ptr[j]; // 将sub的指针指向pub的数据
                        break;
                    }
                    
                    
                else //到结尾,退出
                    while(1); //如果你卡在这里,说明没有找到消息发布者!请确认消息名称是否键入错误
            }
        else //说明已经遍历完所有的subs
            break;
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