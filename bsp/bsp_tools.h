#include "cmsis_os.h"
#include "bsp_log.h"

/**
 * @brief 创建一个新的任务,该任务收到osSignalSet信号时会被唤醒,否则保持挂起状态
 * 
 * @note 注册完成后,只需要将module提供的回调函数替换成osSignalSet即可
 *       推荐将处理较为复杂的回调函数注册为任务,如视觉长包解析/多机通信/双板通信/裁判系统通信等
 *
 * @param name 任务名称,注意以'\0'结尾
 * @param cbk 回调函数指针
 * @param ins 回调函数的参数,bsp为实例指针
 * @param priority 任务优先级,决定了在中断结束后能否保证立刻执行
 * @return uint32_t 信号量,用于唤醒该任务
 */
uint32_t CreateCallbackTask(char *name, void *cbk, void *ins, osPriority priority);

