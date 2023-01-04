#ifndef BSP_INIT_h
#define BSP_INIT_h


/**
 * @brief bsp层初始化统一入口,这里仅初始化必须的bsp组件,其他组件的初始化在各自的模块中进行
 *        需在实时系统启动前调用,目前由RobotoInit()调用
 * 
 */
void BSPInit();

#endif // !BSP_INIT_h

