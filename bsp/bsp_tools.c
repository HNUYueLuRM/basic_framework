#include <stdarg.h>

#include "cmsis_os.h"
#include "bsp_log.h"
#include "bsp_tools.h"

#define MX_SIG_LIST_SIZE 32 // 不可修改,最大信号量数量

typedef struct
{
    void *callback;
    uint32_t sig;
    void *ins;
} CallbackTask_t;

// 递增记录全局信号量的数组,最大32个信号量
static uint8_t sig_idx = 0;
static uint32_t tmp_sig = 1; // tmp_sing << sig_idx从而生成对应信号量

static osThreadId cbkid_list[MX_SIG_LIST_SIZE];
static CallbackTask_t cbkinfo_list[MX_SIG_LIST_SIZE];

// 死循环任务,执行cbk函数指针,每次执行完毕后等待sig信号
static void CallbackTaskBase(const void *cbk)
{
    void (*cbk_func)(void *) = (void (*)(void *))((CallbackTask_t *)cbk)->callback;
    void *ins = ((CallbackTask_t *)cbk)->ins;
    uint32_t sig = ((CallbackTask_t *)cbk)->sig;

    for (;;)
    {
        cbk_func(ins);
        osSignalWait(sig, osWaitForever);
    }
}

uint32_t CreateCallbackTask(char *name, void *cbk, void *ins, osPriority priority)
{
    if (sig_idx >= MX_SIG_LIST_SIZE)
        while (1)
            LOGERROR("[rtos:cbk_register] CreateCallbackTask: sig_idx >= MX_SIG_LIST_SIZE");

    cbkinfo_list[sig_idx].callback = cbk;
    cbkinfo_list[sig_idx].sig = tmp_sig << sig_idx;
    cbkinfo_list[sig_idx].ins = ins;

    osThreadDef_t threadDef;
    threadDef.name = name;
    threadDef.pthread = CallbackTaskBase;
    threadDef.tpriority = priority;
    threadDef.instances = 0;
    threadDef.stacksize = 128;
    cbkid_list[sig_idx] = osThreadCreate(&threadDef, (void *)&cbkinfo_list[sig_idx]);

    return cbkinfo_list[sig_idx++].sig; // 返回信号量,同时增加索引
}