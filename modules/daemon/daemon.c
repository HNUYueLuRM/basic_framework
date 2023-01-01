#include "daemon.h"
#include "bsp_dwt.h" // 后续通过定时器来计时?
#include "stdlib.h"
#include "memory.h"

// 用于保存所有的daemon instance
static DaemonInstance *daemon_instances[DAEMON_MX_CNT] = {NULL};
static uint8_t idx; // 用于记录当前的daemon instance数量,配合回调使用

DaemonInstance *DaemonRegister(Daemon_Init_Config_s *config)
{
    DaemonInstance *instance = (DaemonInstance *)malloc(sizeof(DaemonInstance));
    memset(instance, 0, sizeof(DaemonInstance));

    instance->owner_id = config->owner_id;
    instance->reload_count = config->reload_count;
    instance->callback = config->callback;

    daemon_instances[idx++] = instance;
    return instance;
}

void DaemonReload(DaemonInstance *instance)
{
    instance->temp_count = instance->reload_count;
}

uint8_t DaemonIsOnline(DaemonInstance *instance)
{
    return instance->temp_count > 0;
}

void DaemonTask()
{
    static DaemonInstance *dins; // 提高可读性同时降低访存开销
    for (size_t i = 0; i < idx; ++i)
    {
        dins = daemon_instances[i];
        if (dins->temp_count > 0)
            dins->temp_count--;
        else if (dins->callback) // 如果有callback
        {
            dins->callback(dins->owner_id); // module内可以将owner_id强制类型转换成自身类型从而调用自身的offline callback
        }
    }
}
