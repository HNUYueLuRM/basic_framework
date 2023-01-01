#ifndef _BSP_LOG_H
#define _BSP_LOG_H

/**
 * @brief 初始化日志功能,在操作系统启动之前调用
 * 
 */
void BSPLogInit();

/**
 * @brief 通过segger RTT打印日志,支持格式化输出,格式化输出的实现参考printf
 * 
 * @param fmt 
 * @param ... 
 * @return int 
 */
int PrintLog(const char *fmt, ...);

/**
 * @brief 利用sprintf(),将float转换为字符串进行打印
 * 
 * @param str 转换后的字符串
 * @param va 待转换的float
 */
void Float2Str(char *str, float va);

#endif