#include "daemon.h"
#include "bsp_dwt.h" // 后续通过定时器来计时?
#include "stdlib.h"

static DaemonInstance *daemon_instances[DAEMON_MX_CNT] = {NULL};
static uint8_t idx;

DaemonInstance *DaemonRegister(Daemon_Init_Config_s *config)
{
    
    daemon_instances[idx] = (DaemonInstance *)malloc(sizeof(DaemonInstance));

    daemon_instances[idx]->reload_count = config->reload_count;
    daemon_instances[idx]->callback = config->callback;
    daemon_instances[idx]->owner_id = config->id;
    daemon_instances[idx]->temp_count = config->reload_count;

    return daemon_instances[idx++];
}

void DaemonReload(DaemonInstance *instance)
{
    instance->temp_count = instance->reload_count;
}

uint8_t DaemonIsOnline(DaemonInstance *instance)
{
    return instance->temp_count>0;
}

void DaemonTask()
{
    static DaemonInstance* pins; //提高可读性同时降低访存开销
    for (size_t i = 0; i < idx; ++i)
    {
        pins=daemon_instances[i];
        if(pins->temp_count>0)
            pins->temp_count--;
        else if(pins->callback)
        {   // 每个module根据自身的offline callback进行调用
            pins->callback(pins->owner_id); // module将owner_id强制类型转换成自身类型
        }
    }
}
